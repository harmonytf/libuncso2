//========= Copyright Valve Corporation, All rights reserved. ============//
//
//  LZMA Codec interface for engine.
//
//  LZMA SDK 9.38 beta
//  2015-01-03 : Igor Pavlov : Public domain
//  http://www.7-zip.org/
//
//  Edited for libuncso2 (https://github.com/Ochii/libuncso2).
//  Original file can be found at
//  https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/tier1/lzmaDecoder.cpp
//
//========================================================================//

#define _LZMADECODER_CPP

#include <assert.h>
#include <algorithm>
#include <memory>

#include <7zTypes.h>
#include <LzmaDec.h>

// Ugly define to let us forward declare the anonymous-struct-typedef that is
// CLzmaDec in the header.
#define CLzmaDec_t CLzmaDec
#include "lzmaDecoder.h"

// Allocator to pass to LZMA functions
static void* SzAlloc(ISzAllocPtr, std::size_t size)
{
    return malloc(size);
}
static void SzFree(ISzAllocPtr, void* address)
{
    free(address);
}
static ISzAlloc g_Alloc = { SzAlloc, SzFree };

//-----------------------------------------------------------------------------
// Returns true if buffer is compressed.
//-----------------------------------------------------------------------------
/* static */
bool CLZMA::IsCompressed(unsigned char* pInput)
{
    lzma_header_t* pHeader = (lzma_header_t*)pInput;
    if (pHeader && pHeader->id == LZMA_ID)
    {
        return true;
    }

    // unrecognized
    return false;
}

//-----------------------------------------------------------------------------
// Returns uncompressed size of compressed input buffer. Used for allocating
// output buffer for decompression. Returns 0 if input buffer is not compressed.
//-----------------------------------------------------------------------------
/* static */
unsigned int CLZMA::GetActualSize(unsigned char* pInput)
{
    lzma_header_t* pHeader = (lzma_header_t*)pInput;
    if (pHeader && pHeader->id == LZMA_ID)
    {
        return pHeader->actualSize;
    }

    // unrecognized
    return 0;
}

//-----------------------------------------------------------------------------
// Uncompress a buffer, Returns the uncompressed size. Caller must provide an
// adequate sized output buffer or memory corruption will occur.
//-----------------------------------------------------------------------------
/* static */
size_t CLZMA::Uncompress(unsigned char* pInput, unsigned char* pOutput)
{
    lzma_header_t* pHeader = (lzma_header_t*)pInput;
    if (pHeader->id != LZMA_ID)
    {
        // not ours
        return false;
    }

    CLzmaDec state;

    LzmaDec_Construct(&state);

    if (LzmaDec_Allocate(&state, pHeader->properties, LZMA_PROPS_SIZE,
                         &g_Alloc) != SZ_OK)
    {
        assert(false);
        return 0;
    }

    // These are in/out variables
    SizeT outProcessed = pHeader->actualSize;
    SizeT inProcessed = pHeader->lzmaSize;
    ELzmaStatus status;
    SRes result = LzmaDecode(
        (Byte*)pOutput, &outProcessed, (Byte*)(pInput + sizeof(lzma_header_t)),
        &inProcessed, (Byte*)pHeader->properties, LZMA_PROPS_SIZE,
        LZMA_FINISH_END, &status, &g_Alloc);

    LzmaDec_Free(&state, &g_Alloc);

    if (result != SZ_OK || pHeader->actualSize != outProcessed)
    {
        return 0;
    }

    return outProcessed;
}

CLZMAStream::CLZMAStream()
    : m_pDecoderState(NULL), m_nActualSize(0), m_nActualBytesRead(0),
      m_nCompressedSize(0), m_nCompressedBytesRead(0), m_bParsedHeader(false),
      m_bZIPStyleHeader(false)
{
}

CLZMAStream::~CLZMAStream()
{
    FreeDecoderState();
}

void CLZMAStream::FreeDecoderState()
{
    if (m_pDecoderState)
    {
        LzmaDec_Free(m_pDecoderState, &g_Alloc);
        m_pDecoderState = NULL;
    }
}

bool CLZMAStream::CreateDecoderState(const unsigned char* pProperties)
{
    if (m_pDecoderState)
    {
        assert(!m_pDecoderState);
        FreeDecoderState();
    }

    m_pDecoderState = new CLzmaDec();

    LzmaDec_Construct(m_pDecoderState);
    if (LzmaDec_Allocate(m_pDecoderState, pProperties, LZMA_PROPS_SIZE,
                         &g_Alloc) != SZ_OK)
    {
        assert(!"Failed to allocate lzma decoder state");
        m_pDecoderState = NULL;
        return false;
    }

    LzmaDec_Init(m_pDecoderState);

    return true;
}

// Attempt to read up to nMaxInputBytes from the compressed stream, writing up
// to nMaxOutputBytes to pOutput. Returns false if read stops due to an error.
bool CLZMAStream::Read(unsigned char* pInput, size_t nMaxInputBytes,
                       unsigned char* pOutput, size_t nMaxOutputBytes,
                       /* out */ size_t& nCompressedBytesRead,
                       /* out */ size_t& nOutputBytesWritten)
{
    nCompressedBytesRead = 0;
    nOutputBytesWritten = 0;
    bool bStartedWithHeader = m_bParsedHeader;

    // Check for initial chunk of data
    if (!m_bParsedHeader)
    {
        size_t nBytesConsumed = 0;
        eHeaderParse parseResult =
            TryParseHeader(pInput, nMaxInputBytes, nBytesConsumed);

        if (parseResult == eHeaderParse_NeedMoreBytes)
        {
            // Not an error, just need more data to continue
            return true;
        }
        else if (parseResult != eHeaderParse_OK)
        {
            // Invalid header
            return false;
        }

        // Header consumed, fall through to continue read after it
        nCompressedBytesRead += nBytesConsumed;
        pInput += nBytesConsumed;
        nMaxInputBytes -= nBytesConsumed;
    }

    // These are input ( available size ) *and* output ( size processed ) vars
    // for lzma
    std::size_t expectedInputRemaining =
        m_nCompressedSize -
        std::min(m_nCompressedBytesRead + nCompressedBytesRead,
                 m_nCompressedSize);
    std::size_t expectedOutputRemaining = m_nActualSize - m_nActualBytesRead;
    std::size_t inSize = std::min((std::size_t)nMaxInputBytes, expectedInputRemaining);
    std::size_t outSize = std::min((std::size_t)nMaxOutputBytes, expectedOutputRemaining);
    ELzmaStatus status;
    ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
    if (inSize == expectedInputRemaining && outSize == expectedOutputRemaining)
    {
        // Expect to finish decoding this call.
        finishMode = LZMA_FINISH_END;
    }
    SRes result = LzmaDec_DecodeToBuf(m_pDecoderState, pOutput, &outSize,
                                      pInput, &inSize, finishMode, &status);

    // DevMsg("[%p] Running lzmaDecode:\n"
    //        "    pInput:             %p\n"
    //        "    nMaxInputBytes:     %i\n"
    //        "    pOutput:            %p\n"
    //        "    nMaxOutputBytes:    %u\n"
    //        "    inSize:             %u\n"
    //        "    outSize:            %u\n"
    //        "    result:             %u\n"
    //        "    status:             %i\n"
    //        "    m_nActualSize:      %u\n"
    //        "    m_nActualBytesRead: %u\n",
    //        this, pInput, nMaxInputBytes, pOutput, nMaxOutputBytes,
    //        inSize, outSize, result, status, m_nActualSize,
    //        m_nActualBytesRead);

    if (result != SZ_OK)
    {
        if (!bStartedWithHeader)
        {
            // If we're returning false, we need to pretend we didn't consume
            // anything.
            FreeDecoderState();
            m_bParsedHeader = false;
        }
        return false;
    }

    nCompressedBytesRead += inSize;
    nOutputBytesWritten += outSize;

    m_nCompressedBytesRead += nCompressedBytesRead;
    m_nActualBytesRead += nOutputBytesWritten;

    assert(m_nCompressedBytesRead <= m_nCompressedSize);
    return true;
}

bool CLZMAStream::GetExpectedBytesRemaining(
    /* out */ unsigned int& nBytesRemaining)
{
    if (!m_bParsedHeader && !m_bZIPStyleHeader)
    {
        return false;
    }

    nBytesRemaining = m_nActualSize - m_nActualBytesRead;

    return true;
}

void CLZMAStream::InitZIPHeader(unsigned int nCompressedSize,
                                unsigned int nOriginalSize)
{
    if (m_bParsedHeader || m_bZIPStyleHeader)
    {
        assert(!m_bParsedHeader && !m_bZIPStyleHeader);
        return;
    }

    m_nCompressedSize = nCompressedSize;
    m_nActualSize = nOriginalSize;
    // Signal to TryParseHeader to expect a zip-style header (which wont have
    // the size values)
    m_bZIPStyleHeader = true;
}

CLZMAStream::eHeaderParse CLZMAStream::TryParseHeader(
    unsigned char* pInput, size_t nBytesAvailable,
    /* out */ size_t& nBytesConsumed)
{
    nBytesConsumed = 0;

    if (m_bParsedHeader)
    {
        assert(!m_bParsedHeader);
        return eHeaderParse_Fail;
    }

    if (m_bZIPStyleHeader)
    {
        // ZIP Spec, 5.8.8
        //   LZMA Version Information 2 bytes
        //   LZMA Properties Size 2 bytes
        //   LZMA Properties Data variable, defined by "LZMA Properties Size"

        if (nBytesAvailable < 4)
        {
            // No error, but need more input to continue
            return eHeaderParse_NeedMoreBytes;
        }

        // Should probably check this
        // unsigned char nLZMAVer[2] = { pInput[0], pInput[1] };

        std::uint16_t nLZMAPropertiesSize = *(std::uint16_t*)(pInput + 2);

        nBytesConsumed += 4;

        if (nLZMAPropertiesSize != LZMA_PROPS_SIZE)
        {
            return eHeaderParse_Fail;
        }

        if (nBytesAvailable < static_cast<size_t>(nLZMAPropertiesSize) + 4)
        {
            return eHeaderParse_NeedMoreBytes;
        }

        // Looks reasonable, try to parse
        if (!CreateDecoderState((Byte*)pInput + 4))
        {
            return eHeaderParse_Fail;
        }

        nBytesConsumed += nLZMAPropertiesSize;
    }
    else
    {
        // Else native source engine style header
        if (nBytesAvailable < sizeof(lzma_header_t))
        {
            // need more input to continue
            return eHeaderParse_NeedMoreBytes;
        }

        m_nActualSize = CLZMA::GetActualSize(pInput);

        if (!m_nActualSize)
        {
            // unrecognized
            return eHeaderParse_Fail;
        }

        if (!CreateDecoderState(((lzma_header_t*)pInput)->properties))
        {
            return eHeaderParse_Fail;
        }

        m_nCompressedSize =
            ((lzma_header_t*)pInput)->lzmaSize + sizeof(lzma_header_t);
        nBytesConsumed += sizeof(lzma_header_t);
    }

    m_bParsedHeader = true;
    return eHeaderParse_OK;
}
