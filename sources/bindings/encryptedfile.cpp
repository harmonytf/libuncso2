#include "encryptedfile.h"
#include "encryptedfileimpl.hpp"

#ifdef __cplusplus
extern "C"
{
    EncryptedFile_t UNCSO2_CALLMETHOD uncso2_EncryptedFile_Create(
        const char* filename, void* dataBuffer, uint64_t dataSize,
        const uint8_t (*keyCollection)[4][16])
    {
        gsl::span<uint8_t> dataView(reinterpret_cast<uint8_t*>(dataBuffer),
                                    dataSize);

        gsl::span<const uint8_t[4][16]> keyCollectionView(keyCollection,
                                                          16 * 4);

        try
        {
            auto newPkg = uc2::EncryptedFileImpl::CreateSpan(filename, dataView,
                                                             keyCollectionView);
            return reinterpret_cast<EncryptedFile_t>(newPkg.release());
        }
        catch (const std::exception&)
        {
            return NULL;
        }
    }

    void UNCSO2_CALLMETHOD uncso2_EncryptedFile_Free(EncryptedFile_t fileHandle)
    {
        auto pFile = reinterpret_cast<uc2::EncryptedFile*>(fileHandle);
        delete pFile;
    }

    bool UNCSO2_CALLMETHOD
    uncso2_EncryptedFile_IsEncryptedFile(void* pData, const uint64_t iDataSize)
    {
        return uc2::EncryptedFile::IsEncryptedFile(static_cast<uint8_t*>(pData),
                                                   iDataSize);
    }

    uint64_t UNCSO2_CALLMETHOD uncso2_EncryptedFile_GetHeaderSize()
    {
        return uc2::EncryptedFile::GetHeaderSize();
    }

    bool UNCSO2_CALLMETHOD uncso2_EncryptedFile_Decrypt(
        EncryptedFile_t fileHandle, void** outBuffer, uint64_t* outSize)
    {
        if (fileHandle == NULL)
        {
            return false;
        }

        auto pFile = reinterpret_cast<uc2::EncryptedFile*>(fileHandle);

        try
        {
            auto result = pFile->Decrypt();

            *outBuffer = result.first;
            *outSize = result.second;

            return true;
        }
        catch (const std::exception&)
        {
            return false;
        }
    }
#endif

#ifdef __cplusplus
}
#endif