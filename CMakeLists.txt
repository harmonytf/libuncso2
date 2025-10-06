cmake_minimum_required(VERSION 3.13.0)

if(POLICY CMP0077)
  cmake_policy(SET CMP0077 NEW)
endif()

project(LibPkg LANGUAGES CXX)

#
# Put Visual Studio resulting binaries out of their config directory
#
if(MSVC)
  set(PKG_POSSIBLE_CONFIGS "Debug;Release;MinSizeRel;RelWithDebInfo")
  foreach(CUR_CONFIG ${PKG_POSSIBLE_CONFIGS})
    string(TOUPPER ${CUR_CONFIG} CUR_CONFIG)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_${CUR_CONFIG} "${OUTPUT_DIRECTORY}")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_${CUR_CONFIG} "${OUTPUT_DIRECTORY}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_${CUR_CONFIG} "${OUTPUT_DIRECTORY}")
  endforeach()
endif()

# are we being built as a dependency?
set(PKG_IS_STANDALONE OFF)
if(CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
  message("libuncso2: Building ourselves as a standalone project")
  set(PKG_IS_STANDALONE ON)
endif()

#
# User optinos
#
option(PKG_BUILD_SHARED "Build libuncso2 as a shared library" ON)
option(PKG_BUILD_TESTS "Build tests" ${PKG_IS_STANDALONE})

set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake_modules")

#
# Global paths
#
set(PKG_ROOT_DIR "${PROJECT_SOURCE_DIR}")
set(PKG_LIBS_DIR "${PKG_ROOT_DIR}/external")
set(PKG_PUBLIC_HEADERS_ROOT_DIR "${PKG_ROOT_DIR}/public_headers")
set(PKG_PUBLIC_HEADERS_DIR "${PKG_PUBLIC_HEADERS_ROOT_DIR}/uc2")
set(PKG_GENERATED_DIR "${CMAKE_BINARY_DIR}/generated")

set(PKG_LIB_CRYPTOPP_DIR "${PKG_LIBS_DIR}/cryptopp")
set(PKG_LIB_CRYPTOPP_CMAKE_DIR "${PKG_LIBS_DIR}/cryptopp-cmake")
set(PKG_LIB_LZMA_DIR "${PKG_LIBS_DIR}/lzma")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(PKG_VERSION_FILE "${PKG_ROOT_DIR}/version.txt")
set(PKG_VERSION_TEMPLATE "${PKG_ROOT_DIR}/headers/lib_version.hpp.in")
set(PKG_VERSION_OUT "${PKG_GENERATED_DIR}/lib_version.hpp")

include(libuc2_version)
generate_libuc2_version(VERSION_FILE
                        ${PKG_VERSION_FILE}
                        HEADER_TEMPLATE
                        ${PKG_VERSION_TEMPLATE}
                        HEADER_OUT
                        ${PKG_VERSION_OUT})

#
# Dependencies
#

set(CRYPTOPP_SOURCES "${PKG_LIB_CRYPTOPP_DIR}")
add_subdirectory("${PKG_LIB_CRYPTOPP_CMAKE_DIR}")

# Enable weak namespace for MD5
add_definitions(-DCRYPTOPP_ENABLE_NAMESPACE_WEAK=1)

add_subdirectory("${PKG_LIB_LZMA_DIR}")

#
# add source files to the project
#
set(PKG_SOURCES_BASE
    "sources/bindings/encryptedfile.cpp"
    "sources/bindings/lzmatexture.cpp"
    "sources/bindings/pkgentry.cpp"
    "sources/bindings/pkgfile.cpp"
    "sources/bindings/pkgfileoptions.cpp"
    "sources/bindings/pkgindex.cpp"
    "sources/bindings/uc2version.cpp"
    "sources/ciphers/aescipher.cpp"
    "sources/ciphers/blowfishcipher.cpp"
    "sources/ciphers/descipher.cpp"
    "sources/pkg/pkgentry.cpp"
    "sources/pkg/pkgfile.cpp"
    "sources/pkg/pkgfileoptions.cpp"
    "sources/pkg/pkgindex.cpp"
    "sources/decryptor.cpp"
    "sources/encryptedfile.cpp"
    "sources/keyhashes.cpp"
    "sources/lzmaDecoder.cpp"
    "sources/lzmatexture.cpp"
    "sources/uc2version.cpp")

set(PKG_PUBLIC_HEADERS_BASE
    "${PKG_PUBLIC_HEADERS_DIR}/encryptedfile.h"
    "${PKG_PUBLIC_HEADERS_DIR}/encryptedfile.hpp"
    "${PKG_PUBLIC_HEADERS_DIR}/lzmatexture.h"
    "${PKG_PUBLIC_HEADERS_DIR}/lzmatexture.hpp"
    "${PKG_PUBLIC_HEADERS_DIR}/pkgentry.h"
    "${PKG_PUBLIC_HEADERS_DIR}/pkgentry.hpp"
    "${PKG_PUBLIC_HEADERS_DIR}/pkgfile.h"
    "${PKG_PUBLIC_HEADERS_DIR}/pkgfile.hpp"
    "${PKG_PUBLIC_HEADERS_DIR}/pkgfileoptions.h"
    "${PKG_PUBLIC_HEADERS_DIR}/pkgfileoptions.hpp"
    "${PKG_PUBLIC_HEADERS_DIR}/pkgindex.h"
    "${PKG_PUBLIC_HEADERS_DIR}/pkgindex.hpp"
    "${PKG_PUBLIC_HEADERS_DIR}/uc2.h"
    "${PKG_PUBLIC_HEADERS_DIR}/uc2.hpp"
    "${PKG_PUBLIC_HEADERS_DIR}/uc2defs.h"
    "${PKG_PUBLIC_HEADERS_DIR}/uc2version.h"
    "${PKG_PUBLIC_HEADERS_DIR}/uc2version.hpp")

set(PKG_HEADERS_BASE
    "headers/ciphers/aescipher.hpp"
    "headers/ciphers/basecipher.hpp"
    "headers/ciphers/blowfishcipher.hpp"
    "headers/ciphers/descipher.hpp"
    "headers/pkg/pkgentryimpl.hpp"
    "headers/pkg/pkgfileimpl.hpp"
    "headers/pkg/pkgfileoptionsimpl.hpp"
    "headers/pkg/pkgindeximpl.hpp"
    "headers/pkg/pkgstructures.hpp"
    "headers/decryptor.hpp"
    "headers/encryptedfileimpl.hpp"
    "headers/keyhashes.hpp"
    "headers/lzmaDecoder.h"
    "headers/lzmatextureimpl.hpp"
    "headers/util.hpp"
    ${PKG_VERSION_OUT})

file(GLOB
     PKG_ALL_SOURCES
     ${PKG_SOURCES_BASE}
     ${PKG_PUBLIC_HEADERS_BASE}
     ${PKG_HEADERS_BASE})

source_group("Source Files" FILES ${PKG_SOURCES_BASE})
source_group("Public Header Files" FILES ${PKG_PUBLIC_HEADERS_BASE})
source_group("Header Files" FILES ${PKG_HEADERS_BASE})

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_definitions(-DCMAKE_EXPORT_COMPILE_COMMANDS=ON)

#
# Add library to build.
#
if(PKG_BUILD_SHARED)
  message("libuncso2: Building ourselves as a shared library")
  add_definitions(-DUNCSO2_EXPORT)
  add_library(uncso2 SHARED ${PKG_ALL_SOURCES})
else()
  message("libuncso2: Building ourselves as a static library")
  add_definitions(-DUNCSO2_STATIC)
  add_library(uncso2 STATIC ${PKG_ALL_SOURCES})
endif()

set_target_properties(uncso2 PROPERTIES VERSION ${LIB_VERSION})

#
# Enable all warnings
#
if(MSVC)
  target_compile_options(uncso2 PRIVATE /DWIN32 /D_WINDOWS /W4 /EHsc)
else()
  target_compile_options(uncso2 PRIVATE -Wall -Wextra -pedantic)
endif()

#
# Setup include directories
#
target_include_directories(uncso2 PRIVATE "headers")
target_include_directories(uncso2 PRIVATE "${PKG_PUBLIC_HEADERS_DIR}")

target_include_directories(uncso2 PRIVATE "external")
target_include_directories(uncso2 PRIVATE "${LZMA_INCLUDE_DIR}")

# the generated version header's directory
target_include_directories(uncso2 PRIVATE ${PKG_GENERATED_DIR})

#
# Setup library directories
#

target_link_libraries(uncso2 lzma cryptopp)

#
# Set include directory for dependent projects
#
set(PKG_INCLUDE_DIR
    ${PKG_PUBLIC_HEADERS_ROOT_DIR}
    CACHE INTERNAL "${PROJECT_NAME}: Include Directories" FORCE)

set(PKG_LIBRARY
    ${PROJECT_NAME}
    CACHE INTERNAL "${PROJECT_NAME}: Library Name" FORCE)

#
# Build tests (if the user wants to)
#
if(PKG_BUILD_TESTS)
  message("libuncso2: Building tests")
  add_subdirectory(tests)
endif()
