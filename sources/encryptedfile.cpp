#include "encryptedfileimpl.hpp"

#include <assert.h>
#include <stdexcept>

#include "decryptor.hpp"
#include "keyhashes.hpp"
#include "util.hpp"

namespace uc2
{
constexpr const std::uint16_t ENCRYPTED_FILE_VERSION = 2;

EncryptedFile::ptr_t EncryptedFile::Create(
    std::string_view fileName, std::vector<std::uint8_t>& fileData,
    const std::uint8_t (&keyCollection)[4][16])
{
    return std::make_unique<EncryptedFileImpl>(fileName, fileData,
                                               keyCollection);
}

EncryptedFile::ptr_t EncryptedFile::Create(
    std::string_view fileName, std::uint8_t* pData,
    const std::uint64_t iDataSize, const std::uint8_t (&keyCollection)[4][16])
{
    return EncryptedFileImpl::CreateSpan(
        fileName, std::span<std::uint8_t>(pData, iDataSize),
        { &keyCollection, 1 });
}

EncryptedFile::ptr_t EncryptedFileImpl::CreateSpan(
    std::string_view fileName, std::span<std::uint8_t> fileDataView,
    std::span<const std::uint8_t[4][16]> keyCollectionView)
{
    return std::make_unique<EncryptedFileImpl>(fileName, fileDataView,
                                               keyCollectionView);
}

bool EncryptedFile::IsEncryptedFile(std::uint8_t* pData,
                                    const std::uint64_t iDataSize)
{
    std::span<std::uint8_t> dataView(pData, iDataSize);

    if (iDataSize < sizeof(EncryptedFileHeader_t))
    {
        return false;
    }

    auto pHeader =
        AddOffsetToBase<const EncryptedFileHeader_t>(dataView.data());

    if (pHeader->version != ENCRYPTED_FILE_VERSION)
    {
        return false;
    }

    // Don't validate the data's size, since the user may only provide the
    // header's data only
    /*const std::uint32_t iPredictedSize =
        sizeof(EncryptedFileHeader_t) + pHeader->fileSize;

    if (iPredictedSize > iDataSize)
    {
        return false;
    } */

    return true;
}

std::uint64_t EncryptedFile::GetHeaderSize()
{
    return sizeof(EncryptedFileHeader_t);
}

EncryptedFileImpl::EncryptedFileImpl(std::string_view fileName,
                                     std::vector<std::uint8_t>& fileData,
                                     const std::uint8_t (&keyCollection)[4][16])
    : m_szvFileName(fileName), m_FileDataView(fileData),
      m_KeyCollectionView(&keyCollection, 16 * 4)
{
    if (this->IsHeaderValid() == false)
    {
        throw std::invalid_argument("libuncso2: The file's header is invalid");
    }
}

EncryptedFileImpl::EncryptedFileImpl(
    std::string_view fileName, std::span<std::uint8_t> fileDataView,
    std::span<const std::uint8_t[4][16]> keyCollectionView)
    : m_szvFileName(fileName), m_FileDataView(fileDataView),
      m_KeyCollectionView(keyCollectionView)
{
    if (this->IsHeaderValid() == false)
    {
        throw std::invalid_argument("libuncso2: The file's header is invalid");
    }
}

EncryptedFileImpl::~EncryptedFileImpl() {}

bool EncryptedFileImpl::IsHeaderValid() const
{
    const std::uint64_t iFileDataSize = this->m_FileDataView.size_bytes();

    if (iFileDataSize < sizeof(EncryptedFileHeader_t))
    {
        return false;
    }

    auto pHeader = AddOffsetToBase<const EncryptedFileHeader_t>(
        this->m_FileDataView.data());

    const std::uint32_t iPredictedSize =
        sizeof(EncryptedFileHeader_t) + pHeader->fileSize;

    if (iPredictedSize > iFileDataSize)
    {
        return false;
    }

    return true;
}

std::pair<std::uint8_t*, std::size_t> EncryptedFileImpl::Decrypt()
{
    auto pHeader = AddOffsetToBase<const EncryptedFileHeader_t>(
        this->m_FileDataView.data());

    std::vector<std::uint8_t> digestedKey = GeneratePkgIndexKey(
        pHeader->flag, this->m_szvFileName, this->m_KeyCollectionView);

    auto pDataStart = AddOffsetToBase<std::uint8_t>(
        this->m_FileDataView.data(), sizeof(EncryptedFileHeader_t));

    auto pCipher = CreateIndexCipher(pHeader->cipher);
    CDecryptor decryptor(pCipher.get(), digestedKey);

    std::size_t iNewDataSize =
        decryptor.DecryptInBuffer(pDataStart, pHeader->fileSize);

    assert(iNewDataSize <= pHeader->fileSize);

    return { pDataStart, iNewDataSize };
}
}  // namespace uc2
