#include <catch2/catch_all.hpp>

#include <iostream>
#include <string_view>

#include <uc2/uc2.h>
#include <uc2/uc2.hpp>

#include "cso2/nexon/settings.hpp"
#include "utils.hpp"

using namespace std::literals::string_view_literals;

TEST_CASE("Pkg file can be decrypted and parsed", "[pkgfile]")
{
    SECTION("Can parse entries")
    {
        for (std::size_t i = 0; i < cso2::NUM_PROVIDERS; i++)
        {
            auto [bWasRead, vFileBuffer] =
                ReadFileToBuffer(cso2::PkgFilenames[i]);

            REQUIRE(bWasRead == true);
            REQUIRE(vFileBuffer.empty() == false);

            try
            {
                auto pPkgFile = uc2::PkgFile::Create(
                    cso2::PkgFilenames[i], vFileBuffer,
                    cso2::PackageEntryKeys[i], cso2::PackageFileKeys[i]);

                pPkgFile->DecryptHeader();
                pPkgFile->Parse();

                REQUIRE(pPkgFile->GetEntries().size() ==
                        cso2::PackageFileCounts[i]);

                std::size_t iCurIndex = 0;
                for (auto&& entry : pPkgFile->GetEntries())
                {
                    auto [fileData, fileDataLen] = entry->DecryptFile();
                    REQUIRE(GetDataHash(fileData, fileDataLen) ==
                            cso2::PackageFilesHashes[i][iCurIndex]);

                    iCurIndex++;
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                throw e;
            }
        }
    }
}

TEST_CASE("Pkg file partially decrypting an entry", "[pkgfile]")
{
    SECTION("Can decrypt 16 bytes of an entry")
    {
        for (std::size_t i = 0; i < cso2::NUM_PROVIDERS; i++)
        {
            auto [bWasRead, vFileBuffer] =
                ReadFileToBuffer(cso2::PkgFilenames[i]);

            REQUIRE(bWasRead == true);
            REQUIRE(vFileBuffer.empty() == false);

            try
            {
                auto pPkgFile = uc2::PkgFile::Create(
                    cso2::PkgFilenames[i], vFileBuffer,
                    cso2::PackageEntryKeys[i], cso2::PackageFileKeys[i]);

                REQUIRE(pPkgFile);

                pPkgFile->DecryptHeader();
                pPkgFile->Parse();

                REQUIRE(pPkgFile->GetEntries().size() ==
                        cso2::PackageFileCounts[i]);

                constexpr const std::uint64_t iTargetFileLen = 16;

                auto&& entry = pPkgFile->GetEntries().at(0);
                auto [fileData, fileDataLen] =
                    entry->DecryptFile(iTargetFileLen);

                REQUIRE(fileDataLen == iTargetFileLen);
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                throw e;
            }
        }
    }
    SECTION("Can decrypt 23 bytes of an entry")
    {
        for (std::size_t i = 0; i < cso2::NUM_PROVIDERS; i++)
        {
            auto [bWasRead, vFileBuffer] =
                ReadFileToBuffer(cso2::PkgFilenames[i]);

            REQUIRE(bWasRead == true);
            REQUIRE(vFileBuffer.empty() == false);

            try
            {
                auto pPkgFile = uc2::PkgFile::Create(
                    cso2::PkgFilenames[i], vFileBuffer,
                    cso2::PackageEntryKeys[i], cso2::PackageFileKeys[i]);

                pPkgFile->DecryptHeader();
                pPkgFile->Parse();

                REQUIRE(pPkgFile->GetEntries().size() ==
                        cso2::PackageFileCounts[i]);

                constexpr const std::uint64_t iTargetFileLen = 23;
                constexpr const std::uint64_t iExpectedFileLen = 32;

                auto&& entry = pPkgFile->GetEntries().at(0);
                auto [fileData, fileDataLen] =
                    entry->DecryptFile(iTargetFileLen);

                REQUIRE(fileDataLen == iExpectedFileLen);
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                throw e;
            }
        }
    }
}

TEST_CASE("Pkg file can be decrypted and parsed using C bindings", "[pkgfile]")
{
    SECTION("Can parse entries")
    {
        for (std::size_t i = 0; i < cso2::NUM_PROVIDERS; i++)
        {
            auto [bWasRead, vFileBuffer] =
                ReadFileToBuffer(cso2::PkgFilenames[i]);

            REQUIRE(bWasRead == true);
            REQUIRE(vFileBuffer.empty() == false);

            PkgFile_t pPkg = uncso2_PkgFile_Create(
                cso2::PkgFilenames[i].data(), vFileBuffer.data(),
                vFileBuffer.size(), cso2::PackageEntryKeys[i].data(),
                cso2::PackageFileKeys[i].data());
            REQUIRE(pPkg != nullptr);

            bool bHeaderDecrypted = uncso2_PkgFile_DecryptHeader(pPkg);
            REQUIRE(bHeaderDecrypted == true);

            bool bParsed = uncso2_PkgFile_Parse(pPkg);
            REQUIRE(bParsed == true);

            std::uint64_t iEntriesNum = uncso2_PkgFile_GetEntriesNum(pPkg);
            PkgEntry_t* pEntries = uncso2_PkgFile_GetEntries(pPkg);

            REQUIRE(iEntriesNum == cso2::PackageFileCounts[i]);

            for (std::size_t y = 0; y < iEntriesNum; y++)
            {
                void* pOutBuffer;
                std::uint64_t iOutBufferSize;
                bool bValidEntry = uncso2_PkgEntry_Decrypt(
                    pEntries[y], &pOutBuffer, &iOutBufferSize);

                REQUIRE(bValidEntry == true);
                REQUIRE(GetDataHash(reinterpret_cast<std::uint8_t*>(pOutBuffer),
                                    iOutBufferSize) ==
                        cso2::PackageFilesHashes[i][y]);
            }

            uncso2_PkgFile_Free(pPkg);
        }
    }
}
