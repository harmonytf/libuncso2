#include "lzmatexture.h"
#include "lzmatextureimpl.hpp"

#ifdef __cplusplus
extern "C"
{
    LzmaTexture_t UNCSO2_CALLMETHOD uncso2_LzmaTexture_Create(void* texBuffer,
                                                              uint64_t texSize)
    {
        try
        {
            auto newTex = uc2::LzmaTextureImpl::Create(
                reinterpret_cast<uint8_t*>(texBuffer), texSize);
            return reinterpret_cast<LzmaTexture_t>(newTex.release());
        }
        catch (const std::exception&)
        {
            return NULL;
        }
    }

    void UNCSO2_CALLMETHOD uncso2_LzmaTexture_Free(LzmaTexture_t texHandle)
    {
        auto pTex = reinterpret_cast<uc2::LzmaTexture*>(texHandle);
        delete pTex;
    }

    uint64_t UNCSO2_CALLMETHOD
    uncso2_LzmaTexture_GetOriginalSize(LzmaTexture_t texHandle)
    {
        if (texHandle == NULL)
        {
            return 0;
        }

        auto pTex = reinterpret_cast<uc2::LzmaTexture*>(texHandle);

        try
        {
            return pTex->GetOriginalSize();
        }
        catch (const std::exception&)
        {
            return 0;
        }
    }

    bool uncso2_LzmaTexture_IsLzmaTexture(void* pData, const uint64_t iDataSize)
    {
        return uc2::LzmaTexture::IsLzmaTexture(static_cast<uint8_t*>(pData),
                                               iDataSize);
    }

    uint64_t uncso2_LzmaTexture_GetHeaderSize()
    {
        return uc2::LzmaTexture::GetHeaderSize();
    }

    bool UNCSO2_CALLMETHOD uncso2_LzmaTexture_Decompress(
        LzmaTexture_t texHandle, void* outBuffer, uint64_t outBufferSize)
    {
        if (texHandle == NULL)
        {
            return 0;
        }

        auto pTex = reinterpret_cast<uc2::LzmaTexture*>(texHandle);

        try
        {
            return pTex->Decompress(reinterpret_cast<uint8_t*>(outBuffer),
                                    outBufferSize);
        }
        catch (const std::exception&)
        {
            return 0;
        }
    }
#endif

#ifdef __cplusplus
}
#endif