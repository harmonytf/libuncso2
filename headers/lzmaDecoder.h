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
//  https://github.com/ValveSoftware/source-sdk-2013/blob/master/mp/src/public/tier1/lzmaDecoder.h
//
//========================================================================//

#ifndef _LZMADECODER_H
#define _LZMADECODER_H
#pragma once

#if !defined( _X360 )
#define LZMA_ID				(('A'<<24)|('M'<<16)|('Z'<<8)|('L'))
#else
#define LZMA_ID				(('L'<<24)|('Z'<<16)|('M'<<8)|('A'))
#endif

// bind the buffer for correct identification
#pragma pack(1)
struct lzma_header_t
{
	unsigned int	id;
	unsigned int	actualSize;		// always little endian
	unsigned int	lzmaSize;		// always little endian
	unsigned char	properties[5];
};
#pragma pack()

class CLZMAStream;

class CLZMA
{
public:
	static size_t		Uncompress( unsigned char *pInput, unsigned char *pOutput );
	static bool			IsCompressed( unsigned char *pInput );
	static unsigned int	GetActualSize( unsigned char *pInput );
};

// For files besides the implementation, we forward declare a dummy struct. We can't unconditionally forward declare
// this because LzmaEnc.h typedefs this directly to an unnamed struct :-/
#ifndef CLzmaDec_t
struct _CLzmaDec_t;
#define CLzmaDec_t struct _CLzmaDec_t
#endif

class CLZMAStream
{
public:
	CLZMAStream();
	~CLZMAStream();

	// Initialize a stream to read data from a LZMA style zip file, passing the original size from the zip headers.
	// Streams with a source-engine style header (lzma_header_t) do not need an init call.
	void InitZIPHeader( unsigned int nCompressedSize, unsigned int nOriginalSize );

	// Attempt to read up to nMaxInputBytes from the compressed stream, writing up to nMaxOutputBytes to pOutput.
	// Makes progress until blocked on input or output.
	// Returns false if read stops due to an error or if called at EOF (GetExpectedBytesRemaining == 0)
    bool Read(unsigned char* pInput, size_t nMaxInputBytes,
              unsigned char* pOutput, size_t nMaxOutputBytes,
              /* out */ size_t& nCompressedBytesRead,
              /* out */ size_t& nOutputBytesWritten);

	// Get the expected uncompressed bytes yet to be read from this stream. Returns false if not yet known, such as
	// before being fed the header.
	bool GetExpectedBytesRemaining( /* out */ unsigned int &nBytesRemaining );

private:
	enum eHeaderParse
	{
		eHeaderParse_OK,
		eHeaderParse_Fail,
		eHeaderParse_NeedMoreBytes
	};

	eHeaderParse TryParseHeader(unsigned char* pInput, size_t nBytesAvailable,
                                /* out */ size_t& nBytesConsumed);

	void FreeDecoderState();
	bool CreateDecoderState( const unsigned char *pProperties );

	// Init from a zip-embedded LZMA stream. Requires the original size be passed from zip headers.
	CLzmaDec_t *m_pDecoderState;

	size_t m_nActualSize;
    size_t m_nActualBytesRead;
    size_t m_nCompressedSize;
    size_t m_nCompressedBytesRead;

	// If we have read past the header
	bool m_bParsedHeader   : 1;
	// If InitZIPHeader() was called. We're expecting a zip-style header and have size information.
	bool m_bZIPStyleHeader : 1;
};

#endif
