#include "pkgentry.hpp"
#include "pkgentry.h"

#ifdef __cplusplus
extern "C"
{
#endif
    bool UNCSO2_CALLMETHOD
    uncso2_PkgEntry_Decrypt(PkgEntry_t entryHandle, void** outBuffer,
                            uint64_t* outSize, uint64_t bytesToDecrypt /*= 0 */)
    {
        if (entryHandle == NULL || outBuffer == NULL || outSize == NULL)
        {
            return false;
        }

        auto pEntry = reinterpret_cast<uc2::PkgEntry*>(entryHandle);

        try
        {
            auto result = pEntry->DecryptFile(bytesToDecrypt);

            *outBuffer = result.first;
            *outSize = result.second;

            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

    const char* UNCSO2_CALLMETHOD
    uncso2_PkgEntry_GetPath(PkgEntry_t entryHandle)
    {
        if (entryHandle == nullptr)
        {
            return NULL;
        }

        auto pEntry = reinterpret_cast<uc2::PkgEntry*>(entryHandle);

        try
        {
            return pEntry->GetFilePath().data();
        }
        catch (const std::exception&)
        {
            return NULL;
        }
    }

    uint64_t UNCSO2_CALLMETHOD
    uncso2_PkgEntry_GetFileOffset(PkgEntry_t entryHandle)
    {
        if (entryHandle == nullptr)
        {
            return 0;
        }

        auto pEntry = reinterpret_cast<uc2::PkgEntry*>(entryHandle);

        try
        {
            return pEntry->GetPkgFileOffset();
        }
        catch (const std::exception&)
        {
            return 0;
        }
    }

    uint64_t UNCSO2_CALLMETHOD
    uncso2_PkgEntry_GetEncryptedSize(PkgEntry_t entryHandle)
    {
        if (entryHandle == nullptr)
        {
            return 0;
        }

        auto pEntry = reinterpret_cast<uc2::PkgEntry*>(entryHandle);

        try
        {
            return pEntry->GetEncryptedSize();
        }
        catch (const std::exception&)
        {
            return 0;
        }
    }

    uint64_t UNCSO2_CALLMETHOD
    uncso2_PkgEntry_GetDecryptedSize(PkgEntry_t entryHandle)
    {
        if (entryHandle == nullptr)
        {
            return 0;
        }

        auto pEntry = reinterpret_cast<uc2::PkgEntry*>(entryHandle);

        try
        {
            return pEntry->GetDecryptedSize();
        }
        catch (const std::exception&)
        {
            return 0;
        }
    }

    bool UNCSO2_CALLMETHOD uncso2_PkgEntry_IsEncrypted(PkgEntry_t entryHandle)
    {
        if (entryHandle == nullptr)
        {
            return false;
        }

        auto pEntry = reinterpret_cast<uc2::PkgEntry*>(entryHandle);

        try
        {
            return pEntry->IsEncrypted();
        }
        catch (const std::exception&)
        {
            return false;
        }
    }

#ifdef __cplusplus
}
#endif
