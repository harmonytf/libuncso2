#include "pkg/pkgentryimpl.hpp"

#include <assert.h>
#include <algorithm>
#include <vector>

#include <ciphers/aescipher.hpp>
#include "decryptor.hpp"
#include "keyhashes.hpp"

static std::string MakeUnixSeparated(std::string_view inPath)
{
    constexpr const char rootCharacter = '/';

    std::string szNewPath;
    szNewPath.reserve(sizeof(rootCharacter) + inPath.length());
    szNewPath = rootCharacter;
    szNewPath += inPath.data();
    std::replace(szNewPath.begin(), szNewPath.end(), '\\', '/');

    return szNewPath;
}

template <typename NumType,
          typename NoConstNumType = typename std::remove_const<NumType>::type>
constexpr inline NoConstNumType RoundNumberToBlock(const NumType num)
{
    return (num % 16 == 0) ? num : (16 - num % 16) + num;
}

namespace uc2
{
constexpr const std::size_t PKG_ENTRY_KEY_LEN = 16;
constexpr const std::uint64_t PKG_DATA_BLOCK_SIZE = 0x10000;

PkgEntryImpl::PkgEntryImpl(std::string_view szFilePath,
                           std::uint64_t pkgFileOffset,
                           std::uint64_t encryptedSize,
                           std::uint64_t decryptedSize, bool isEncrypted,
                           std::span<std::uint8_t> fileDataView,
                           std::string_view szvPkgKey /*= {}*/)
    : m_FileDataView(fileDataView), m_szFilePath(MakeUnixSeparated(szFilePath)),
      m_iPkgFileOffset(pkgFileOffset), m_iEncryptedSize(encryptedSize),
      m_iDecryptedSize(decryptedSize), m_bIsEncrypted(isEncrypted)
{
    if (isEncrypted == true)
    {
        // Packed size should be the same or bigger than unpacked size
        if (decryptedSize > encryptedSize)
        {
            throw std::invalid_argument("libuncso2: The decrypted size is "
                                        "bigger than the encrypted size");
        }

        fs::path filePath = this->m_szFilePath;
        this->m_szHashedKey =
            GeneratePkgFileKey(filePath.filename().string(), szvPkgKey);
        this->m_szHashedKey.resize(PKG_ENTRY_KEY_LEN);
    }
}

PkgEntryImpl::~PkgEntryImpl() {}

std::pair<std::uint8_t*, std::uint64_t> PkgEntryImpl::DecryptFile(
    const std::uint64_t iBytesToDecrypt /*= 0 */)
{
    if (this->m_FileDataView.empty() == true)
    {
        throw std::invalid_argument(
            "libuncso2: The entry's file data is empty.");
    }

    if (iBytesToDecrypt > this->m_iDecryptedSize)
    {
        throw std::invalid_argument(
            "libuncso2: The amount of bytes requested to decrypt is bigger "
            "than decrypted file size.");
    }

    const bool bDecryptAll = iBytesToDecrypt == 0;
    const std::uint64_t iAlignedBytes =
        bDecryptAll == true ? 0 : RoundNumberToBlock(iBytesToDecrypt);
    const std::uint64_t iRequiredDataSize =
        bDecryptAll == true ? this->m_iEncryptedSize : iAlignedBytes;

    const std::uint64_t iRequiredFileSize =
        this->m_iPkgFileOffset + iRequiredDataSize;
    const std::uint64_t iFileDataSize = this->m_FileDataView.size_bytes();

    if (iRequiredFileSize > iFileDataSize)
    {
        throw std::runtime_error("libuncso2: The file in this entry cannot be "
                                 "larger than the pkg file");
    }

    if (this->IsEncrypted() == true)
    {
        return this->HandleEncryptedFile(iAlignedBytes);
    }
    else
    {
        return this->HandlePlainFile(iAlignedBytes);
    }
}

const std::string_view PkgEntryImpl::GetFilePath()
{
    return this->m_szFilePath;
}

std::uint64_t PkgEntryImpl::GetPkgFileOffset()
{
    return this->m_iPkgFileOffset;
}

std::uint64_t PkgEntryImpl::GetEncryptedSize()
{
    return this->m_iEncryptedSize;
}

std::uint64_t PkgEntryImpl::GetDecryptedSize()
{
    return this->m_iDecryptedSize;
}

bool PkgEntryImpl::IsEncrypted()
{
    return this->m_bIsEncrypted;
}

std::pair<std::uint8_t*, std::uint64_t> PkgEntryImpl::HandleEncryptedFile(
    const std::uint64_t iBytesToDecrypt) const
{
    if (this->m_iEncryptedSize == 0)
    {
        return { nullptr, 0 };
    }

    std::uint64_t dwBlockStart =
        reinterpret_cast<std::uint64_t>(this->m_FileDataView.data()) +
        this->m_iPkgFileOffset;
    std::uint8_t* pFileStart = reinterpret_cast<std::uint8_t*>(dwBlockStart);

    CAesCipher cipher;
    CDecryptor decryptor(&cipher, this->m_szHashedKey, false);

    bool bDecryptAll = iBytesToDecrypt == 0;

    const std::uint64_t iTargetEncDataSize =
        bDecryptAll == true ? this->m_iEncryptedSize : iBytesToDecrypt;
    const std::uint64_t iTargetDecDataSize =
        bDecryptAll == true ? this->m_iDecryptedSize :
                              std::min(this->m_iDecryptedSize, iBytesToDecrypt);

    // The data must be decrypted each PKG_DATA_BLOCK_SIZE (which at the
    // time of writing this is 65536), or else only the first 65536
    // bytes will be correct
    for (std::uint64_t curOff = 0; curOff < iTargetEncDataSize;
         curOff += PKG_DATA_BLOCK_SIZE)
    {
        std::uint8_t* pBlock =
            reinterpret_cast<std::uint8_t*>(dwBlockStart + curOff);
        const std::uint64_t iCurBlockSize =
            std::min(iTargetEncDataSize - curOff, PKG_DATA_BLOCK_SIZE);

        decryptor.DecryptInBuffer(pBlock, iCurBlockSize);
    }

    return { pFileStart, iTargetDecDataSize };
}

std::pair<std::uint8_t*, std::uint64_t> PkgEntryImpl::HandlePlainFile(
    const std::uint64_t iBytesToDecrypt) const noexcept
{
    bool bDecryptAll = iBytesToDecrypt == 0;

    const std::uint64_t iTargetDecDataSize =
        bDecryptAll == true ? this->m_iDecryptedSize : iBytesToDecrypt;

    std::uint64_t dwBlockStart =
        reinterpret_cast<std::uint64_t>(this->m_FileDataView.data()) +
        this->m_iPkgFileOffset;
    std::uint8_t* pFileStart = reinterpret_cast<std::uint8_t*>(dwBlockStart);
    return { pFileStart, iTargetDecDataSize };
}

void PkgEntryImpl::SetDataBufferView(std::span<std::uint8_t> newDataView)
{
    this->m_FileDataView = newDataView;
}

void PkgEntryImpl::ReleaseDataBufferView()
{
    this->m_FileDataView = std::span<std::uint8_t>();
}
}  // namespace uc2