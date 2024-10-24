#include <catch2/catch_all.hpp>

#include <fstream>
#include <iostream>
#include <string_view>

#include <uc2/uc2.h>
#include <uc2/uc2.hpp>

#include "cso2/nexon/settings.hpp"
#include "utils.hpp"

using namespace std::literals::string_view_literals;

TEST_CASE("Pkg index file can be decrypted and parsed", "[pkgindex]")
{
    SECTION("Can parse pkg index")
    {
        for (std::size_t i = 0; i < cso2::NUM_PROVIDERS; i++)
        {
            auto [bWasRead, vIndexBuffer] =
                ReadFileToBuffer(cso2::IndexFilenames[i]);

            REQUIRE(bWasRead == true);
            REQUIRE(vIndexBuffer.empty() == false);

            try
            {
                auto pPkgIndex = uc2::PkgIndex::Create(
                    cso2::IndexRealFilenames[i], vIndexBuffer,
                    &cso2::IndexKeyCollections[i]);

                pPkgIndex->ValidateHeader();
                std::uint64_t iNewSize = pPkgIndex->Parse();

                vIndexBuffer.resize(iNewSize);

                REQUIRE(pPkgIndex->GetFilenames().size() ==
                        cso2::IndexFileCounts[i]);
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                throw e;
            }
        }
    }
}

TEST_CASE("Pkg index file can be decrypted and parsed with C bindings",
          "[pkgindex]")
{
    SECTION("Can parse pkg index")
    {
        for (std::size_t i = 0; i < cso2::NUM_PROVIDERS; i++)
        {
            auto [bWasRead, vIndexBuffer] =
                ReadFileToBuffer(cso2::IndexFilenames[i]);

            REQUIRE(bWasRead == true);
            REQUIRE(vIndexBuffer.empty() == false);

            PkgIndex_t pIndex = uncso2_PkgIndex_Create(
                cso2::IndexRealFilenames[i].data(), vIndexBuffer.data(),
                vIndexBuffer.size(), &cso2::IndexKeyCollections[i]);
            REQUIRE(pIndex != nullptr);

            bool bHeaderRead = uncso2_PkgIndex_ValidateHeader(pIndex);
            REQUIRE(bHeaderRead == true);

            bool bParsed = uncso2_PkgIndex_Parse(pIndex);
            REQUIRE(bParsed == true);

            std::uint64_t iFilenamesNum =
                uncso2_PkgIndex_GetFilenamesNum(pIndex);
            REQUIRE(iFilenamesNum == cso2::IndexFileCounts[i]);

            uncso2_PkgIndex_Free(pIndex);
        }
    }
}
