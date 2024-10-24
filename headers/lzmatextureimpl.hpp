#pragma once

#include "lzmatexture.hpp"

#include <span>

namespace uc2
{
constexpr const std::uint8_t CSO2_LZMA_VTF_HWORD_SIGNATURE = 'C';      // C
constexpr const std::uint16_t CSO2_LZMA_VTF_LWORD_SIGNATURE = 0x324F;  // 02

#pragma pack(push, 1)
struct LzmaVtfHeader_t
{
    struct
    {
        std::uint8_t iHighByte;
        std::uint16_t iLowWord;
    } Signature;
    std::uint8_t iProbs;
    std::uint32_t iOriginalSize;
    std::uint32_t iChunkSizes[];  // TODO: can this flexible array be
                                  // changed to something else?
};
#pragma pack(pop)

class LzmaTextureImpl : public LzmaTexture
{
public:
    LzmaTextureImpl(std::vector<std::uint8_t>& texData);
    LzmaTextureImpl(std::span<std::uint8_t> texDataView);
    virtual ~LzmaTextureImpl();

    virtual std::uint64_t GetOriginalSize() override;

    virtual bool Decompress(std::uint8_t* outBuffer,
                            std::uint64_t outBufferSize) override;

    static bool IsLzmaTextureSpan(std::span<std::uint8_t> texData);

private:
    std::span<std::uint8_t> m_TexDataView;
};
}  // namespace uc2
