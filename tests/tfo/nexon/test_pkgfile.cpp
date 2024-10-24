#include <catch2/catch_all.hpp>

#include <iostream>
#include <string_view>
#include <cstring>

#include <uc2/uc2.h>
#include <uc2/uc2.hpp>

#include "tfo/nexon/settings.hpp"
#include "utils.hpp"

using namespace std::literals::string_view_literals;

TEST_CASE("TFO Pkg file can be decrypted and parsed", "[pkgfile]")
{
    SECTION("Can parse entries and verify their hashes")
    {
        try
        {
            for (auto&& pair : tfo::Packages)
            {
                auto pPkgFilename = pair.first;
                auto iPackageFileCount = pair.second.size();
                auto pPackageFileHashes = pair.second;

                auto [bWasRead, vFileBuffer] = ReadFileToBuffer(pPkgFilename);

                REQUIRE(bWasRead == true);
                REQUIRE(vFileBuffer.empty() == false);

                auto pPkgOptions = uc2::PkgFileOptions::Create();
                pPkgOptions->SetTfoPkg(true);

                auto pPkgFile = uc2::PkgFile::Create(
                    pPkgFilename, vFileBuffer, tfo::PackageEntryKey,
                    tfo::PackageFileKey, pPkgOptions.get());

                pPkgFile->DecryptHeader();
                pPkgFile->Parse();

                REQUIRE(pPkgFile->GetEntries().size() == iPackageFileCount);

                std::size_t iCurIndex = 0;
                for (auto&& entry : pPkgFile->GetEntries())
                {
                    fs::path entryPath = entry->GetFilePath();
                    auto [fileData, fileDataLen] = entry->DecryptFile();
                    REQUIRE(GetDataHash(fileData, fileDataLen) ==
                            pPackageFileHashes[iCurIndex]);

                    iCurIndex++;
                }
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            throw e;
        }
    }
}

TEST_CASE("TFO Pkg file partially decrypting an entry", "[pkgfile]")
{
    SECTION("Can decrypt 16 bytes of an entry")
    {
        auto pPkgFilename = tfo::Packages[0].first;
        auto pPackageFileCount = tfo::Packages[0].second.size();

        auto [bWasRead, vFileBuffer] = ReadFileToBuffer(pPkgFilename);

        REQUIRE(bWasRead == true);
        REQUIRE(vFileBuffer.empty() == false);

        try
        {
            auto pPkgOptions = uc2::PkgFileOptions::Create();
            pPkgOptions->SetTfoPkg(true);

            auto pPkgFile = uc2::PkgFile::Create(
                pPkgFilename, vFileBuffer, tfo::PackageEntryKey,
                tfo::PackageFileKey, pPkgOptions.get());

            pPkgFile->DecryptHeader();
            pPkgFile->Parse();

            REQUIRE(pPkgFile->GetEntries().size() == pPackageFileCount);

            constexpr const std::uint64_t iTargetFileLen = 16;

            auto&& entry = pPkgFile->GetEntries().at(0);
            auto [fileData, fileDataLen] = entry->DecryptFile(iTargetFileLen);

            REQUIRE(fileDataLen == iTargetFileLen);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            throw e;
        }
    }
    SECTION("Can decrypt 23 bytes of an entry")
    {
        auto pPkgFilename = tfo::Packages[0].first;
        auto pPackageFileCount = tfo::Packages[0].second.size();

        auto [bWasRead, vFileBuffer] = ReadFileToBuffer(pPkgFilename);

        REQUIRE(bWasRead == true);
        REQUIRE(vFileBuffer.empty() == false);

        try
        {
            auto pPkgOptions = uc2::PkgFileOptions::Create();
            pPkgOptions->SetTfoPkg(true);

            auto pPkgFile = uc2::PkgFile::Create(
                pPkgFilename, vFileBuffer, tfo::PackageEntryKey,
                tfo::PackageFileKey, pPkgOptions.get());

            pPkgFile->DecryptHeader();
            pPkgFile->Parse();

            REQUIRE(pPkgFile->GetEntries().size() == pPackageFileCount);

            constexpr const std::uint64_t iTargetFileLen = 23;
            constexpr const std::uint64_t iExpectedFileLen = 32;

            auto&& entry = pPkgFile->GetEntries().at(0);
            auto [fileData, fileDataLen] = entry->DecryptFile(iTargetFileLen);

            REQUIRE(fileDataLen == iExpectedFileLen);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            throw e;
        }
    }
    SECTION("Can decrypt an 8 byte entry")
    {
        auto pPkgFilename = tfo::Packages[1].first;
        auto pPackageFileCount = tfo::Packages[1].second.size();

        auto [bWasRead, vFileBuffer] = ReadFileToBuffer(pPkgFilename);

        REQUIRE(bWasRead == true);
        REQUIRE(vFileBuffer.empty() == false);

        try
        {
            auto pPkgOptions = uc2::PkgFileOptions::Create();
            pPkgOptions->SetTfoPkg(true);

            auto pPkgFile = uc2::PkgFile::Create(
                pPkgFilename, vFileBuffer, tfo::PackageEntryKey,
                tfo::PackageFileKey, pPkgOptions.get());

            pPkgFile->DecryptHeader();
            pPkgFile->Parse();

            REQUIRE(pPkgFile->GetEntries().size() == pPackageFileCount);

            constexpr const std::uint64_t iEncryptedFileLen = 16;
            constexpr const std::uint64_t iTargetFileLen = 8;

            auto&& entry = pPkgFile->GetEntries().at(0);
            REQUIRE(entry->IsEncrypted());
            REQUIRE(entry->GetEncryptedSize() == iEncryptedFileLen);
            REQUIRE(entry->GetDecryptedSize() == iTargetFileLen);
            REQUIRE(entry->GetFilePath() == "/r1/datarevision.txt");
            auto [fileData, fileDataLen] = entry->DecryptFile(entry->GetDecryptedSize());

            REQUIRE(fileDataLen == iTargetFileLen);
            REQUIRE(strcmp(reinterpret_cast<char*>(fileData), "17973 \r\n") == 0);
        }
        catch (const std::exception& e)
        {
            std::cerr << e.what() << '\n';
            throw e;
        }
    }
}

TEST_CASE("TFO Pkg file can be decrypted and parsed using C bindings",
          "[pkgfile]")
{
    SECTION("Can parse entries")
    {
        auto pPkgFilename = tfo::Packages[0].first;
        auto pPackageFileCount = tfo::Packages[0].second.size();
        auto pPackageFileHashes = tfo::Packages[0].second;

        auto [bWasRead, vFileBuffer] = ReadFileToBuffer(pPkgFilename);

        REQUIRE(bWasRead == true);
        REQUIRE(vFileBuffer.empty() == false);

        PkgFileOptions_t pOptions = uncso2_PkgFileOptions_Create();
        REQUIRE(pOptions != nullptr);

        uncso2_PkgFileOptions_SetTfoPkg(pOptions, true);

        PkgFile_t pPkg = uncso2_PkgFile_Create(
            pPkgFilename.data(), vFileBuffer.data(), vFileBuffer.size(),
            tfo::PackageEntryKey.data(), tfo::PackageFileKey.data(), pOptions);
        REQUIRE(pPkg != nullptr);

        uncso2_PkgFileOptions_Free(pOptions);

        bool bHeaderDecrypted = uncso2_PkgFile_DecryptHeader(pPkg);
        REQUIRE(bHeaderDecrypted == true);

        bool bParsed = uncso2_PkgFile_Parse(pPkg);
        REQUIRE(bParsed == true);

        std::uint64_t iEntriesNum = uncso2_PkgFile_GetEntriesNum(pPkg);
        PkgEntry_t* pEntries = uncso2_PkgFile_GetEntries(pPkg);

        REQUIRE(iEntriesNum == pPackageFileCount);

        for (std::size_t y = 0; y < iEntriesNum; y++)
        {
            void* pOutBuffer;
            std::uint64_t iOutBufferSize;
            bool bValidEntry = uncso2_PkgEntry_Decrypt(pEntries[y], &pOutBuffer,
                                                       &iOutBufferSize);

            REQUIRE(bValidEntry == true);
            REQUIRE(GetDataHash(reinterpret_cast<std::uint8_t*>(pOutBuffer),
                                iOutBufferSize) == pPackageFileHashes[y]);
        }

        uncso2_PkgFile_Free(pPkg);
    }
}
