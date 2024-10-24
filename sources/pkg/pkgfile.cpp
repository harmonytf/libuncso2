#include "pkg/pkgfileimpl.hpp"

#include <vector>

#include "ciphers/aescipher.hpp"
#include "decryptor.hpp"
#include "keyhashes.hpp"
#include "pkg/pkgentryimpl.hpp"
#include "pkg/pkgfileoptionsimpl.hpp"

namespace uc2
{
constexpr const std::size_t PKG_HASHED_ENTRY_KEY_LEN = 16;
constexpr const std::uint64_t PKG_HEADER_SKIP_HASH_OFFSET = 33;

PkgFile::ptr_t PkgFile::Create(std::string szFilename,
                               std::vector<std::uint8_t>& fileData,
                               std::string szEntryKey /*= {}*/,
                               std::string szDataKey /*= {}*/,
                               PkgFileOptions* options /*= nullptr*/)
{
    return std::make_unique<PkgFileImpl>(szFilename, fileData, szEntryKey,
                                         szDataKey, options);
}

std::uint64_t PkgFile::GetHeaderSize(bool bTfoPkg)
{
    if (bTfoPkg == true)
    {
        return PKG_HEADER_SKIP_HASH_OFFSET + sizeof(PkgHeaderTfo_t);
    }
    else
    {
        return PKG_HEADER_SKIP_HASH_OFFSET + sizeof(PkgHeader_t);
    }
}

PkgFile::ptr_t PkgFileImpl::CreateSpan(std::string szFilename,
                                       std::span<std::uint8_t> fileData,
                                       std::string szEntryKey /*= {}*/,
                                       std::string szDataKey /*= {}*/,
                                       PkgFileOptions* pOptions /* = nullptr*/)
{
    return std::make_unique<PkgFileImpl>(szFilename, fileData, szEntryKey,
                                         szDataKey, pOptions);
}

PkgFileImpl::PkgFileImpl(std::string szFilename,
                         std::vector<std::uint8_t>& fileData,
                         std::string szEntryKey /*= {}*/,
                         std::string szDataKey /*= {}*/,
                         PkgFileOptions* options /* = nullptr*/)
    : m_szFilename(szFilename),
      m_szHashedEntryKey(GeneratePkgFileKey(szFilename, szEntryKey)),
      m_szDataKey(szDataKey), m_FileDataView(fileData), m_bParsed(false)
{
    this->Initialize(szEntryKey, options);
}

PkgFileImpl::PkgFileImpl(std::string szFilename,
                         std::span<std::uint8_t> fileData,
                         std::string szEntryKey /*= {}*/,
                         std::string szDataKey /*= {}*/,
                         PkgFileOptions* pOptions /* = nullptr*/)
    : m_szFilename(szFilename), m_szHashedEntryKey(), m_szDataKey(szDataKey),
      m_FileDataView(fileData), m_bParsed(false)
{
    this->Initialize(szEntryKey, pOptions);
}

PkgFileImpl::~PkgFileImpl() {}

void PkgFileImpl::Initialize(std::string szEntryKey, PkgFileOptions* pOptions)
{
    this->m_bIsTfoPkg = pOptions != nullptr ? pOptions->IsTfoPkg() : false;

    if (this->m_bIsTfoPkg == true)
    {
        this->ValidateInit<PkgHeaderTfo_t>();
    }
    else
    {
        this->ValidateInit<PkgHeader_t>();
    }

    this->SetEntryKey(szEntryKey);
}

template <typename PkgHeaderType>
void PkgFileImpl::ValidateInit() const
{
    if (this->m_szFilename.empty() == true)
    {
        throw std::invalid_argument(
            "libuncso2: The file name argument cannot be empty");
    }

    const std::uint64_t iFileDataSize = this->m_FileDataView.size_bytes();

    if (iFileDataSize < sizeof(PkgHeaderType))
    {
        throw std::range_error(
            "libuncso2: The PKG file does not have a header");
    }
}

bool PkgFileImpl::IsHeaderDecrypted() const
{
    if (this->m_bIsTfoPkg == true)
    {
        return this->IsHeaderDecryptedInternal<PkgHeaderTfo_t>();
    }
    else
    {
        return this->IsHeaderDecryptedInternal<PkgHeader_t>();
    }
}

template <typename PkgHeaderType>
bool PkgFileImpl::IsHeaderDecryptedInternal() const
{
    auto pPkgHeader = this->GetPkgHeader<PkgHeaderType>();
    return pPkgHeader->UnknownVal == 0;
}

std::string_view PkgFileImpl::GetFilename()
{
    return this->m_szFilename;
}

void PkgFileImpl::SetEntryKey(std::string szNewEntryKey)
{
    this->m_szHashedEntryKey =
        GeneratePkgFileKey(this->m_szFilename, szNewEntryKey);
    this->m_szHashedEntryKey.resize(PKG_HASHED_ENTRY_KEY_LEN);
}

void PkgFileImpl::SetDataKey(std::string szNewDataKey)
{
    this->m_szDataKey = szNewDataKey;
}

void PkgFileImpl::SetTfoPkg(bool bNewState)
{
    this->m_bIsTfoPkg = bNewState;
}

void PkgFileImpl::SetDataBuffer(std::vector<std::uint8_t>& newFileData)
{
    this->m_FileDataView = newFileData;
    this->UpdateEntriesDataView();
}

void PkgFileImpl::SetDataBufferSpan(std::span<std::uint8_t> newDataBuffer)
{
    this->m_FileDataView = newDataBuffer;
    this->UpdateEntriesDataView();
}

void PkgFileImpl::ReleaseDataBuffer()
{
    this->m_FileDataView = {};
}

std::uint64_t PkgFileImpl::GetFullHeaderSize()
{
    if (this->m_bIsTfoPkg == true)
    {
        return PkgFileImpl::GetFullHeaderSizeInternal<PkgHeaderTfo_t,
                                                      PkgEntryHeaderTfo_t>();
    }
    else
    {
        return PkgFileImpl::GetFullHeaderSizeInternal<PkgHeader_t,
                                                      PkgEntryHeader_t>();
    }
}

std::string_view PkgFileImpl::GetMd5Hash()
{
    return this->m_szMd5Hash;
}

template <typename PkgHeaderType, typename PkgEntryHeaderType>
std::uint64_t PkgFileImpl::GetFullHeaderSizeInternal() const
{
    if (this->IsHeaderDecryptedInternal<PkgHeaderType>() == false)
    {
        throw std::runtime_error("libuncso2: The header is encrypted, could "
                                 "not fetch full header size.");
    }

    if (this->m_FileDataView.size_bytes() < sizeof(PkgHeaderType))
    {
        throw std::invalid_argument(
            "The file data is smaller than the PKG header structure.");
    }

    auto pPkgHeader = this->GetPkgHeader<PkgHeaderType>();

    return PKG_HEADER_SKIP_HASH_OFFSET + sizeof(PkgHeaderType) +
           pPkgHeader->iEntries * sizeof(PkgEntryHeaderType);
}

bool PkgFileImpl::DecryptHeader()
{
    if (this->m_FileDataView.empty() == true)
    {
        throw std::runtime_error("The file data provided is empty.");
    }

    if (this->m_szDataKey.empty() == true)
    {
        throw std::runtime_error("The data key provided is empty.");
    }

    if (this->m_szHashedEntryKey.empty() == true)
    {
        throw std::runtime_error("The entry key provided is empty.");
    }

    if (this->m_bIsTfoPkg == true)
    {
        return this->DecryptHeaderInternal<PkgHeaderTfo_t>();
    }
    else
    {
        return this->DecryptHeaderInternal<PkgHeader_t>();
    }
}

void PkgFileImpl::Parse()
{
    if (this->m_bParsed == true)
    {
        return;
    }

    if (this->m_FileDataView.empty() == true)
    {
        throw std::runtime_error("The file data provided is empty.");
    }

    if (this->m_bIsTfoPkg == true)
    {
        this->ParseEntries<PkgHeaderTfo_t, PkgEntryHeaderTfo_t>();
    }
    else
    {
        this->ParseEntries<PkgHeader_t, PkgEntryHeader_t>();
    }
}

std::vector<PkgFileImpl::entryptr_t>& PkgFileImpl::GetEntries()
{
    return this->m_Entries;
}

template <typename PkgHeaderType>
bool PkgFileImpl::DecryptHeaderInternal()
{
    // Don't decrypt the header again...
    if (this->IsHeaderDecryptedInternal<PkgHeaderType>() == true)
    {
        return true;
    }

    CAesCipher cipher;
    CDecryptor decryptor(&cipher, this->m_szHashedEntryKey, false);

    auto pPkgHeader = this->GetPkgHeader<PkgHeaderType>();
    decryptor.DecryptInBuffer(pPkgHeader, sizeof(PkgHeaderType));

    if (this->IsHeaderDecryptedInternal<PkgHeaderType>() == false)
    {
        return false;
    }

    return true;
}

template <typename PkgHeaderType, typename PkgEntryHeaderType>
void PkgFileImpl::ParseEntries()
{
    if (this->IsHeaderDecryptedInternal<PkgHeaderType>() == false)
    {
        throw std::runtime_error("libuncso2: The header is encrypted, could "
                                 "not parse the PKG file.");
    }

    this->m_szMd5Hash = reinterpret_cast<const char*>(
        this->m_FileDataView.subspan(0, PKG_HEADER_SKIP_HASH_OFFSET).data());

    auto pPkgHeader = this->GetPkgHeader<PkgHeaderType>();
    auto pEntries = this->GetEntriesHeader<PkgHeaderType, PkgEntryHeaderType>();

    const std::uint64_t iDataStartOffset =
        PKG_HEADER_SKIP_HASH_OFFSET + sizeof(PkgHeaderType) +
        pPkgHeader->iEntries * sizeof(PkgEntryHeaderType);

    CAesCipher cipher;
    CDecryptor decryptor(&cipher, this->m_szHashedEntryKey, false);

    for (std::uint32_t i = 0; i < pPkgHeader->iEntries; i++)
    {
        PkgEntryHeaderType* entry = &pEntries[i];
        decryptor.DecryptInBuffer(entry, sizeof(PkgEntryHeaderType));

        auto pNewEntry = std::make_unique<PkgEntryImpl>(
            entry->szFilePath, iDataStartOffset + entry->iOffset,
            entry->iEncryptedSize, entry->iDecryptedSize, entry->bIsEncrypted,
            this->m_FileDataView, this->m_szDataKey);

        this->m_Entries.push_back(std::move(pNewEntry));
    }

    this->m_bParsed = true;
}

void PkgFileImpl::UpdateEntriesDataView()
{
    for (auto&& entry : this->m_Entries)
    {
        auto pEntryImpl = static_cast<PkgEntryImpl*>(entry.get());
        pEntryImpl->SetDataBufferView(this->m_FileDataView);
    }
}

template <typename PkgHeaderType>
PkgHeaderType* PkgFileImpl::GetPkgHeader() const
{
    std::uint64_t data =
        reinterpret_cast<std::uint64_t>(this->m_FileDataView.data());
    data += PKG_HEADER_SKIP_HASH_OFFSET;
    return reinterpret_cast<PkgHeaderType*>(data);
}

template <typename PkgHeaderType, typename PkgEntryHeaderType>
PkgEntryHeaderType* PkgFileImpl::GetEntriesHeader() const
{
    std::uint64_t data =
        reinterpret_cast<std::uint64_t>(this->m_FileDataView.data());
    data += PKG_HEADER_SKIP_HASH_OFFSET + sizeof(PkgHeaderType);
    return reinterpret_cast<PkgEntryHeaderType*>(data);
}
}  // namespace uc2