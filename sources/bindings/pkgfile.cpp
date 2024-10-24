#include "pkgfile.h"
#include "pkg/pkgfileimpl.hpp"

#ifdef __cplusplus
extern "C"
{
    PkgFile_t UNCSO2_CALLMETHOD uncso2_PkgFile_Create(
        const char* filename, void* dataBuffer, uint64_t dataSize,
        const char* szEntryKey, const char* szDataKey,
        PkgFileOptions_t options /*= NULL*/)
    {
        gsl::span<uint8_t> dataView(reinterpret_cast<uint8_t*>(dataBuffer),
                                    dataSize);

        auto pOptions = reinterpret_cast<uc2::PkgFileOptions*>(options);

        try
        {
            auto newPkg = uc2::PkgFileImpl::CreateSpan(
                filename, dataView, szEntryKey, szDataKey, pOptions);
            return reinterpret_cast<PkgFile_t>(newPkg.release());
        }
        catch (const std::exception&)
        {
            return NULL;
        }
    }

    void UNCSO2_CALLMETHOD uncso2_PkgFile_Free(PkgFile_t pkgHandle)
    {
        auto pPkg = reinterpret_cast<uc2::PkgFile*>(pkgHandle);
        delete pPkg;
    }

    const char* UNCSO2_CALLMETHOD
    uncso2_PkgFile_GetFilename(PkgFile_t pkgHandle)
    {
        if (pkgHandle == NULL)
        {
            return nullptr;
        }

        auto pPkg = reinterpret_cast<uc2::PkgFile*>(pkgHandle);

        return pPkg->GetFilename().data();
    }

    void UNCSO2_CALLMETHOD uncso2_PkgFile_SetEntryKey(PkgFile_t pkgHandle,
                                                      const char* szNewEntryKey)
    {
        if (pkgHandle == NULL)
        {
            return;
        }

        auto pPkg = reinterpret_cast<uc2::PkgFile*>(pkgHandle);

        pPkg->SetEntryKey(szNewEntryKey);
    }

    void UNCSO2_CALLMETHOD uncso2_PkgFile_SetDataKey(PkgFile_t pkgHandle,
                                                     const char* szNewDataKey)
    {
        if (pkgHandle == NULL)
        {
            return;
        }

        auto pPkg = reinterpret_cast<uc2::PkgFile*>(pkgHandle);

        pPkg->SetDataKey(szNewDataKey);
    }

    void UNCSO2_CALLMETHOD uncso2_PkgFile_SetTfoPkg(PkgFile_t pkgHandle,
                                                    bool bNewState)
    {
        if (pkgHandle == NULL)
        {
            return;
        }

        auto pPkg = reinterpret_cast<uc2::PkgFile*>(pkgHandle);

        pPkg->SetTfoPkg(bNewState);
    }

    void UNCSO2_CALLMETHOD uncso2_PkgFile_SetDataBuffer(PkgFile_t pkgHandle,
                                                        void* dataBuffer,
                                                        uint64_t dataSize)
    {
        if (pkgHandle == NULL)
        {
            return;
        }

        auto pPkg = reinterpret_cast<uc2::PkgFileImpl*>(pkgHandle);

        gsl::span<uint8_t> dataView(reinterpret_cast<uint8_t*>(dataBuffer),
                                    dataSize);

        pPkg->SetDataBufferSpan(dataView);
    }

    void UNCSO2_CALLMETHOD uncso2_PkgFile_ReleaseDataBuffer(PkgFile_t pkgHandle)
    {
        if (pkgHandle == NULL)
        {
            return;
        }

        auto pPkg = reinterpret_cast<uc2::PkgFile*>(pkgHandle);

        pPkg->ReleaseDataBuffer();
    }

    uint64_t UNCSO2_CALLMETHOD
    uncso2_PkgFile_GetFullHeaderSize(PkgFile_t pkgHandle)
    {
        if (pkgHandle == NULL)
        {
            return 0;
        }

        auto pPkg = reinterpret_cast<uc2::PkgFile*>(pkgHandle);

        try
        {
            return pPkg->GetFullHeaderSize();
        }
        catch (const std::exception&)
        {
            return 0;
        }
    }

    bool UNCSO2_CALLMETHOD uncso2_PkgFile_DecryptHeader(PkgFile_t pkgHandle)
    {
        if (pkgHandle == NULL)
        {
            return false;
        }

        auto pPkg = reinterpret_cast<uc2::PkgFile*>(pkgHandle);

        try
        {
            pPkg->DecryptHeader();
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    bool UNCSO2_CALLMETHOD uncso2_PkgFile_Parse(PkgFile_t pkgHandle)
    {
        if (pkgHandle == NULL)
        {
            return false;
        }

        auto pPkg = reinterpret_cast<uc2::PkgFile*>(pkgHandle);

        try
        {
            pPkg->Parse();
            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    uint64_t UNCSO2_CALLMETHOD uncso2_PkgFile_GetEntriesNum(PkgFile_t pkgHandle)
    {
        if (pkgHandle == NULL)
        {
            return 0;
        }

        auto pPkg = reinterpret_cast<uc2::PkgFile*>(pkgHandle);

        try
        {
            return pPkg->GetEntries().size();
        }
        catch (const std::exception&)
        {
            return 0;
        }
    }

    PkgEntry_t* UNCSO2_CALLMETHOD uncso2_PkgFile_GetEntries(PkgFile_t pkgHandle)
    {
        if (pkgHandle == NULL)
        {
            return 0;
        }

        auto pPkg = reinterpret_cast<uc2::PkgFile*>(pkgHandle);

        try
        {
            return reinterpret_cast<PkgEntry_t*>(pPkg->GetEntries().data());
        }
        catch (const std::exception&)
        {
            return 0;
        }
    }

    uint64_t UNCSO2_CALLMETHOD uncso2_PkgFile_GetHeaderSize(bool bTfoPkg)
    {
        return uc2::PkgFile::GetHeaderSize(bTfoPkg);
    }
#endif

#ifdef __cplusplus
}
#endif