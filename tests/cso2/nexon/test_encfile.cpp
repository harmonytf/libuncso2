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

TEST_CASE("Can decrypt .e* files", "[encfile]")
{
    SECTION("Decrypting .e* file")
    {
        for (std::size_t i = 0; i < cso2::NUM_PROVIDERS; i++)
        {
            auto [bWasRead, vFileBuffer] =
                ReadFileToBuffer(cso2::EncryptedFileNames[i]);

            REQUIRE(bWasRead == true);
            REQUIRE(vFileBuffer.empty() == false);

            bool bIsEncryptedFile = uc2::EncryptedFile::IsEncryptedFile(
                vFileBuffer.data(), vFileBuffer.size());

            REQUIRE(bIsEncryptedFile == true);

            try
            {
                auto pEncryptedFile = uc2::EncryptedFile::Create(
                    cso2::RealEncryptedFileNames[i], vFileBuffer,
                    cso2::IndexKeyCollections[i]);
                auto [fileData, fileSize] = pEncryptedFile->Decrypt();

                REQUIRE(GetDataHash(fileData, fileSize) ==
                        cso2::EncryptedFileHashes[i]);
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << '\n';
                throw e;
            }
        }
    }
}

TEST_CASE("Can decrypt .e* files with C bindings", "[encfile]")
{
    SECTION("Decrypting .e* file")
    {
        for (std::size_t i = 0; i < cso2::NUM_PROVIDERS; i++)
        {
            auto [bWasRead, vFileBuffer] =
                ReadFileToBuffer(cso2::EncryptedFileNames[i]);

            REQUIRE(bWasRead == true);
            REQUIRE(vFileBuffer.empty() == false);

            bool bIsEncryptedFile = uncso2_EncryptedFile_IsEncryptedFile(
                vFileBuffer.data(), vFileBuffer.size());

            REQUIRE(bIsEncryptedFile == true);

            EncryptedFile_t pFile = uncso2_EncryptedFile_Create(
                cso2::RealEncryptedFileNames[i].data(), vFileBuffer.data(),
                vFileBuffer.size(), &cso2::IndexKeyCollections[i]);
            REQUIRE(pFile != nullptr);

            std::uint8_t* pOutBuf = nullptr;
            std::uint64_t iBufSize = 0;
            bool bWasDecrypted = uncso2_EncryptedFile_Decrypt(
                pFile, reinterpret_cast<void**>(&pOutBuf), &iBufSize);

            REQUIRE(bWasDecrypted == true);
            REQUIRE(GetDataHash(pOutBuf, iBufSize) ==
                    cso2::EncryptedFileHashes[i]);

            uncso2_EncryptedFile_Free(pFile);
        }
    }
}
