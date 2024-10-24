#include <catch2/catch_all.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <string_view>

#include <uc2/uc2.h>
#include <uc2/uc2.hpp>

#include "cso2/nexon/settings.hpp"
#include "utils.hpp"

using namespace std::literals::string_view_literals;

TEST_CASE("Can decompress LZMA'd VTFs", "[lzmavtf]")
{
    SECTION("Decompressing texture file")
    {
        auto [bWasRead, vTexBuffer] = ReadFileToBuffer(cso2::TextureFilename);

        REQUIRE(bWasRead == true);
        REQUIRE(vTexBuffer.empty() == false);

        bool bIsLzmaTex = uc2::LzmaTexture::IsLzmaTexture(vTexBuffer.data(),
                                                          vTexBuffer.size());

        REQUIRE(bIsLzmaTex == true);

        try
        {
            auto pTex = uc2::LzmaTexture::Create(vTexBuffer);

            std::uint64_t iOrigSize = pTex->GetOriginalSize();
            REQUIRE(iOrigSize != 0);

            std::vector<std::uint8_t> buff(iOrigSize);
            bool bDecompressed = pTex->Decompress(buff.data(), buff.size());

            REQUIRE(bDecompressed == true);
            REQUIRE(GetDataHash(buff.data(), buff.size()) ==
                    cso2::TextureFileHash);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            throw e;
        }
    }
}

TEST_CASE("Can decompress LZMA'd VTFs with C bindings", "[lzmavtf]")
{
    SECTION("Decompressing texture file")
    {
        auto [bWasRead, vTexBuffer] = ReadFileToBuffer(cso2::TextureFilename);

        REQUIRE(bWasRead == true);
        REQUIRE(vTexBuffer.empty() == false);

        bool bIsLzmaTex = uncso2_LzmaTexture_IsLzmaTexture(vTexBuffer.data(),
                                                           vTexBuffer.size());

        REQUIRE(bIsLzmaTex == true);

        LzmaTexture_t pTexture =
            uncso2_LzmaTexture_Create(vTexBuffer.data(), vTexBuffer.size());
        REQUIRE(pTexture != nullptr);

        std::uint64_t iOrigSize = uncso2_LzmaTexture_GetOriginalSize(pTexture);
        REQUIRE(iOrigSize != 0);

        std::uint8_t* pOutBuf = new std::uint8_t[iOrigSize];
        REQUIRE(pOutBuf != nullptr);

        bool bWasDecompressed =
            uncso2_LzmaTexture_Decompress(pTexture, pOutBuf, iOrigSize);

        REQUIRE(bWasDecompressed == true);
        REQUIRE(GetDataHash(reinterpret_cast<std::uint8_t*>(pOutBuf),
                            iOrigSize) == cso2::TextureFileHash);

        free(pOutBuf);
        uncso2_LzmaTexture_Free(pTexture);
    }
}
