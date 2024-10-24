#pragma once

#include "pkgfile.hpp"

#include <span>
#include <string>

#include "pkg/pkgstructures.hpp"

namespace uc2
{
class PkgFileImpl : public PkgFile
{
public:
    PkgFileImpl(std::string szFilename, std::vector<std::uint8_t>& fileData,
                std::string szEntryKey = {}, std::string szDataKey = {},
                PkgFileOptions* options = nullptr);
    PkgFileImpl(std::string szFilename, std::span<std::uint8_t> fileDataView,
                std::string szEntryKey = {}, std::string szDataKey = {},
                PkgFileOptions* pOptions = nullptr);
    virtual ~PkgFileImpl() override;

    virtual std::string_view GetFilename() override;

    virtual void SetEntryKey(std::string szNewEntryKey) override;
    virtual void SetDataKey(std::string szNewDataKey) override;
    virtual void SetTfoPkg(bool bNewState) override;

    virtual void SetDataBuffer(std::vector<std::uint8_t>& newFileData) override;
    void SetDataBufferSpan(std::span<std::uint8_t> newDataBuffer);
    virtual void ReleaseDataBuffer() override;

    virtual std::uint64_t GetFullHeaderSize() override;

    virtual std::string_view GetMd5Hash() override;

    virtual bool DecryptHeader() override;
    virtual void Parse() override;

    virtual std::vector<entryptr_t>& GetEntries() override;

    static ptr_t CreateSpan(std::string szFilename,
                            std::span<std::uint8_t> fileDataView = {},
                            std::string szEntryKey = {},
                            std::string szDataKey = {},
                            PkgFileOptions* pOptions = nullptr);

    template <typename PkgHeaderType, typename PkgEntryHeaderType>
    std::uint64_t GetFullHeaderSizeInternal() const;

private:
    void Initialize(std::string szEntryKey, PkgFileOptions* pOptions);

    template <typename PkgHeaderType>
    void ValidateInit() const;

    bool IsHeaderDecrypted() const;
    template <typename PkgHeaderType>
    bool IsHeaderDecryptedInternal() const;

    template <typename PkgHeaderType>
    bool DecryptHeaderInternal();
    template <typename PkgHeaderType, typename PkgEntryHeaderType>
    void ParseEntries();

    void UpdateEntriesDataView();

    template <typename PkgHeaderType>
    PkgHeaderType* GetPkgHeader() const;

    template <typename PkgHeaderType, typename PkgEntryHeaderType>
    PkgEntryHeaderType* GetEntriesHeader() const;

private:
    std::string m_szFilename;
    std::string m_szHashedEntryKey;
    std::string m_szDataKey;

    std::string m_szMd5Hash;

    std::span<std::uint8_t> m_FileDataView;

    std::vector<std::unique_ptr<PkgEntry>> m_Entries;

    bool m_bIsTfoPkg;
    bool m_bParsed;
};
}  // namespace uc2
