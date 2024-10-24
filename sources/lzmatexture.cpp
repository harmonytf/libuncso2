#include "lzmatextureimpl.hpp"

#include <assert.h>
#include <stdexcept>

#include "lzmaDecoder.h"

namespace uc2
{
LzmaTexture::ptr_t LzmaTexture::Create(std::vector<std::uint8_t>& texData)
{
    return std::make_unique<LzmaTextureImpl>(texData);
}

LzmaTexture::ptr_t LzmaTexture::Create(std::uint8_t* pData,
                                       const std::uint64_t iDataSize)
{
    return std::make_unique<LzmaTextureImpl>(
        std::span<std::uint8_t>(pData, iDataSize));
}

LzmaTextureImpl::LzmaTextureImpl(std::vector<std::uint8_t>& texData)
    : m_TexDataView(texData)
{
    if (LzmaTexture::IsLzmaTexture(this->m_TexDataView.data(),
                                   this->m_TexDataView.size_bytes()) == false)
    {
        throw std::invalid_argument("libuncso2: The file's header is invalid");
    }
}

LzmaTextureImpl::LzmaTextureImpl(std::span<std::uint8_t> texDataView)
    : m_TexDataView(texDataView)
{
    if (LzmaTexture::IsLzmaTexture(this->m_TexDataView.data(),
                                   this->m_TexDataView.size_bytes()) == false)
    {
        throw std::invalid_argument("libuncso2: The file's header is invalid");
    }
}

LzmaTextureImpl::~LzmaTextureImpl() {}

std::uint64_t LzmaTextureImpl::GetOriginalSize()
{
    auto pHeader =
        reinterpret_cast<LzmaVtfHeader_t*>(this->m_TexDataView.data());

    return pHeader->iOriginalSize;
}

bool LzmaTextureImpl::Decompress(std::uint8_t* outBuffer,
                                 std::uint64_t outBufferSize)
{
    auto pHeader =
        reinterpret_cast<LzmaVtfHeader_t*>(this->m_TexDataView.data());

    if (pHeader->iOriginalSize != outBufferSize)
    {
        return false;
    }

    std::uint8_t* pBuffer =
        reinterpret_cast<std::uint8_t*>(this->m_TexDataView.data());
    std::uint32_t iOutOffset = 0;

    CLZMA lzma;

    for (std::uint8_t i = 0; i < pHeader->iProbs; i++)
    {
        std::uint32_t iChunkSize = pHeader->iChunkSizes[i * 2];
        std::uint8_t* pChunk = pBuffer + (iChunkSize >> 1);
        bool bIsCompressed = iChunkSize & 1;

        unsigned int outSize;

        if (bIsCompressed)
        {
            outSize = lzma.Uncompress(pChunk, outBuffer + iOutOffset);
        }
        else
        {
            outSize = pHeader->iChunkSizes[i * 2 + 1];
            std::copy(pChunk, pChunk + outSize, outBuffer + iOutOffset);
        }

        iOutOffset += outSize;
    }

    if (pHeader->iOriginalSize != iOutOffset)
    {
        return false;
    }

    return true;
}

bool LzmaTexture::IsLzmaTexture(std::uint8_t* pData,
                                const std::uint64_t iDataSize)
{
    std::span<std::uint8_t> dataView(pData, iDataSize);
    return LzmaTextureImpl::IsLzmaTextureSpan(dataView);
}

std::uint64_t LzmaTexture::GetHeaderSize()
{
    return sizeof(LzmaVtfHeader_t);
}

bool LzmaTextureImpl::IsLzmaTextureSpan(std::span<std::uint8_t> texData)
{
    const std::uint64_t iFileDataSize = texData.size_bytes();

    if (iFileDataSize < sizeof(LzmaVtfHeader_t))
    {
        return false;
    }

    auto pHeader = reinterpret_cast<LzmaVtfHeader_t*>(texData.data());

    return pHeader->Signature.iHighByte == CSO2_LZMA_VTF_HWORD_SIGNATURE &&
           pHeader->Signature.iLowWord == CSO2_LZMA_VTF_LWORD_SIGNATURE;
}
}  // namespace uc2
