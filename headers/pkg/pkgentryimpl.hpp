#pragma once

#include "pkgentry.hpp"

#include <span>
#include <string>
#include <string_view>

namespace uc2
{
class PkgEntryImpl : public PkgEntry
{
public:
    PkgEntryImpl(std::string_view filePath, std::uint64_t pkgFileOffset,
                 std::uint64_t encryptedSize, std::uint64_t decryptedSize,
                 bool isEncrypted, std::span<std::uint8_t> fileData,
                 std::string_view szvPkgKey = {});
    virtual ~PkgEntryImpl() override;

public:
    virtual std::pair<std::uint8_t*, std::uint64_t> DecryptFile(
        const std::uint64_t iBytesToDecrypt = 0) override;

    virtual const std::string_view GetFilePath() override;
    virtual std::uint64_t GetPkgFileOffset() override;
    virtual std::uint64_t GetEncryptedSize() override;
    virtual std::uint64_t GetDecryptedSize() override;
    virtual bool IsEncrypted() override;

    void SetDataBufferView(std::span<std::uint8_t> newDataView);
    void ReleaseDataBufferView();

private:
    std::pair<std::uint8_t*, std::uint64_t> HandleEncryptedFile(
        const std::uint64_t iBytesToDecrypt) const;
    std::pair<std::uint8_t*, std::uint64_t> HandlePlainFile(
        const std::uint64_t iBytesToDecrypt) const noexcept;

private:
    std::span<std::uint8_t> m_FileDataView;

    std::string m_szHashedKey;

    std::string m_szFilePath;
    std::uint64_t m_iPkgFileOffset;
    std::uint64_t m_iEncryptedSize;
    std::uint64_t m_iDecryptedSize;
    bool m_bIsEncrypted;
};
}  // namespace uc2
