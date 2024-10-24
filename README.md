# libuncso2

A library to decrypt and parse PKG files used by Nexon's videogame Counter-Strike Online 2.

PKG is an encrypted archive file format used in Counter-Strike Online 2 and Titanfall Online to store the games' file data.

It's based on the ekey's reversing work of at [XeNTaX's forums](https://forum.xentax.com/viewtopic.php?f=21&t=11117).

Used as the back-end for the GUI program [UnCSO2](https://github.com/harmonytf/UnCSO2).

Tested on:
- GNU/Linux - GCC 8.3.0 (64 bits)
- GNU/Linux - Clang 8.0.0 (64 bits)
- Windows 10 - MSVC 2022 (64 bits)

## Features

- Parse and decrypt PKG files. The AES, DES and Blowfish algorithms are supported.
- Parse PKG index files.
- Decrypt '.e*' files, such as files with .etxt, .escv or .ecfg extensions.
- Decompress LZMA deflated textures.

## Examples

Decrypting and parsing a PKG file, then, extract its entries to the disk:

```cpp
#include <uc2/uc2.hpp>

/* ... */

// Key to decrypt the PKG's header data
const std::string_view NexonEntryKey = "lkgui781kl789sd!@#%89&^sd";

// Key to decrypt the PKG's data itself
const std::string_view NexonDataKey = "^9gErg2Sx7bnk7@#sdfjnh@";

std::string szPkgFilename = "e8a0dcce7c40cf3549ee7e0093ae8041.pkg";
std::vector<std::uint8_t> vPkgData; // Contains a PKG file's data

try
{
    auto pPkgFile = uc2::PkgFile::Create(
        szPkgFilename, vPkgData, NexonEntryKey, NexonDataKey);

    pPkgFile->Parse();

    for (auto&& entry : pPkgFile->GetEntries())
    {
        auto [fileData, fileDataLen] = entry->DecryptFile();
        const std::string_view szvFilePath = entry->GetFilePath();
        /* Write the file to disk... */
    }
}
catch (const std::exception& e)
{
    std::cerr << "libuncso2 error: " << e.what() << '\n';
    throw e;
}
```

Decrypting and parsing a PKG index file and printing the number of entries:

```cpp
#include <uc2/uc2.hpp>

/* ... */

// Key collection to decrypt the index data
// Shared between index files and encrypted files
const std::uint8_t NexonIndexKeyCollection[4][16] = {
    { 0x9A, 0xA6, 0xC7, 0x59, 0x18, 0xEA, 0xD0, 0x44, 0x83, 0xA3, 0x3A,
        0x3E, 0xCE, 0xAF, 0x6F, 0x68 },
    { 0xB6, 0xBA, 0x15, 0xC7, 0x77, 0x9D, 0x9C, 0x49, 0x84, 0x62, 0x2A,
        0x9A, 0x8A, 0x61, 0x84, 0xA6 },
    { 0x68, 0x55, 0x24, 0x24, 0x2B, 0xCB, 0x88, 0x4B, 0xA7, 0xA6, 0xD2,
        0xC7, 0x94, 0xED, 0xE8, 0xD3 },
    { 0x36, 0x24, 0xD6, 0x8C, 0x6C, 0xB8, 0xE1, 0x4A, 0xB1, 0x82, 0xC0,
        0xA3, 0xDC, 0xE4, 0x16, 0xC8 },
};

std::string szIndexFilename = "1b87c6b551e518d11114ee21b7645a47.pkg";
std::vector<std::uint8_t> vIndexData; // Contains the index's file data

try
{
    auto pPkgIndex = uc2::PkgIndex::Create(
        szIndexFilename, vIndexData, NexonIndexKeyCollection);

    const std::uint64_t iNewSize = pPkgIndex->Parse();

    vIndexData.resize(iNewSize);

    std::cout << "Index entries num: " << iNewSize << "\n";
}
catch (const std::exception& e)
{
    std::cerr << "libuncso2 error: " << e.what() << '\n';
    throw e;
}
```

Decrypt an 'e' file and write its contents to a new file:

```cpp
#include <uc2/uc2.hpp>

/* ... */

// Key collection to decrypt the index data
// Shared between index files and encrypted files
const std::uint8_t NexonIndexKeyCollection[4][16] = {
    { 0x9A, 0xA6, 0xC7, 0x59, 0x18, 0xEA, 0xD0, 0x44, 0x83, 0xA3, 0x3A,
        0x3E, 0xCE, 0xAF, 0x6F, 0x68 },
    { 0xB6, 0xBA, 0x15, 0xC7, 0x77, 0x9D, 0x9C, 0x49, 0x84, 0x62, 0x2A,
        0x9A, 0x8A, 0x61, 0x84, 0xA6 },
    { 0x68, 0x55, 0x24, 0x24, 0x2B, 0xCB, 0x88, 0x4B, 0xA7, 0xA6, 0xD2,
        0xC7, 0x94, 0xED, 0xE8, 0xD3 },
    { 0x36, 0x24, 0xD6, 0x8C, 0x6C, 0xB8, 0xE1, 0x4A, 0xB1, 0x82, 0xC0,
        0xA3, 0xDC, 0xE4, 0x16, 0xC8 },
};

std::string szFilename = "attachments_dev.ecsv";
std::vector<std::uint8_t> vFileData; // Contains the index's file data

try
{
    auto pEncryptedFile = uc2::EncryptedFile::Create(
                    szFilename, vFileData, NexonIndexKeyCollection);

    auto [fileData, fileSize] = pEncryptedFile->Decrypt();
    
    /* Write the data to a file... */
}
catch (const std::exception& e)
{
    std::cerr << "libuncso2 error: " << e.what() << '\n';
    throw e;
}
```

Decompress an LZMA'd texture:

```cpp
#include <uc2/uc2.hpp>

/* ... */

std::vector<std::uint8_t> vTexData; // Contains the index's file data

try
{
    auto pTex = uc2::LzmaTexture::Create(vTexData);

    std::uint64_t iOrigSize = pTex->GetOriginalSize();

    if (iOrigSize == 0)
    {
        /* handle failure */
    }

    std::vector<std::uint8_t> buff(iOrigSize);
    bool bDecompressed = pTex->Decompress(buff.data(), buff.size());
}
catch (const std::exception& e)
{
    std::cerr << "libuncso2 error: " << e.what() << '\n';
    throw e;
}
```

## Using with CMake

You can use the following to include libuncso2 in your CMake project:

```cmake
cmake_minimum_required(VERSION 3.12.0)

project(someapp LANGUAGES CXX)

# include the CMakeLists.txt in the root of libuncso2's directory
add_subdirectory("libuncso2")

file(GLOB SOMEAPP_ALL_SOURCES
    "main.cpp")

# C++20 is required
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(someapp ${SOMEAPP_ALL_SOURCES})

target_include_directories(someapp PRIVATE ${PKG_INCLUDE_DIR})
target_link_libraries(someapp ${PKG_LIBRARY})
```

## Libraries used

libuncso2 uses the following libraries:

- [Catch2](https://github.com/catchorg/Catch2) (used in the test application), Boost Software License.
- [Crypto++](https://www.cryptopp.com/), Public domain.
- [LZMA SDK](https://www.7-zip.org/sdk.html), Public domain.

## License

libuncso2 is distributed under the GNU GPLv3 license.

The files `sources/lzmaDecoder.cpp` and `headers/lzmaDecoder.h` are owned by Valve Software, and are under the following copyright notice:

```
DISCLAIMER OF WARRANTIES.  THE SOURCE SDK AND ANY 
OTHER MATERIAL DOWNLOADED BY LICENSEE IS PROVIDED 
"AS IS".  VALVE AND ITS SUPPLIERS DISCLAIM ALL 
WARRANTIES WITH RESPECT TO THE SDK, EITHER EXPRESS 
OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, IMPLIED 
WARRANTIES OF MERCHANTABILITY, NON-INFRINGEMENT, 
TITLE AND FITNESS FOR A PARTICULAR PURPOSE.  

LIMITATION OF LIABILITY.  IN NO EVENT SHALL VALVE OR 
ITS SUPPLIERS BE LIABLE FOR ANY SPECIAL, INCIDENTAL, 
INDIRECT, OR CONSEQUENTIAL DAMAGES WHATSOEVER 
(INCLUDING, WITHOUT LIMITATION, DAMAGES FOR LOSS OF 
BUSINESS PROFITS, BUSINESS INTERRUPTION, LOSS OF 
BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS) 
ARISING OUT OF THE USE OF OR INABILITY TO USE THE 
ENGINE AND/OR THE SDK, EVEN IF VALVE HAS BEEN 
ADVISED OF THE POSSIBILITY OF SUCH DAMAGES. 
```

The files under `tests/gamefiles` are from Counter-Strike Online 2 and Titanfall Online, which are owned by NEXON Korea Corporation.
