#include <catch2/catch_all.hpp>

#include <fstream>
#include <iostream>
#include <string_view>

#include <uc2/uc2.h>
#include <uc2/uc2.hpp>

#include "tfo/nexon/settings.hpp"
#include "utils.hpp"

using namespace std::literals::string_view_literals;

TEST_CASE("TFO Pkg index file can be decrypted and parsed", "[pkgindex]")
{
    SECTION("Can parse pkg index")
    {
        auto [bWasRead, vIndexBuffer] = ReadFileToBuffer(tfo::IndexFilename);

        REQUIRE(bWasRead == true);
        REQUIRE(vIndexBuffer.empty() == false);

        try
        {
            auto pPkgIndex = uc2::PkgIndex::Create(
                tfo::IndexRealFilename, vIndexBuffer, &tfo::IndexKeyCollection);

            pPkgIndex->ValidateHeader();
            std::uint64_t iNewSize = pPkgIndex->Parse();

            vIndexBuffer.resize(iNewSize);

            REQUIRE(pPkgIndex->GetFilenames().size() == tfo::IndexFileCount);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            throw e;
        }
    }
}

TEST_CASE("TFO Pkg index file can be decrypted and parsed with C bindings",
          "[pkgindex]")
{
    SECTION("Can parse pkg index")
    {
        auto [bWasRead, vIndexBuffer] = ReadFileToBuffer(tfo::IndexFilename);

        REQUIRE(bWasRead == true);
        REQUIRE(vIndexBuffer.empty() == false);

        PkgIndex_t pIndex = uncso2_PkgIndex_Create(
            tfo::IndexRealFilename.data(), vIndexBuffer.data(),
            vIndexBuffer.size(), &tfo::IndexKeyCollection);
        REQUIRE(pIndex != nullptr);

        bool bHeaderRead = uncso2_PkgIndex_ValidateHeader(pIndex);
        REQUIRE(bHeaderRead == true);

        bool bParsed = uncso2_PkgIndex_Parse(pIndex);
        REQUIRE(bParsed == true);

        std::uint64_t iFilenamesNum = uncso2_PkgIndex_GetFilenamesNum(pIndex);
        REQUIRE(iFilenamesNum == tfo::IndexFileCount);

        uncso2_PkgIndex_Free(pIndex);
    }
}
