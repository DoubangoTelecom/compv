/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* Copyright (C) ---- Colin Plumb
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_base64.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

// Based on source code from https://github.com/DoubangoTelecom/doubango/blob/master/tinySAK/src/tsk_base64.c
// Base64 encoder and decoder as per RFC 4648

// Pad char
#define COMPV_BASE64_PAD '='

// Encoding block size
#define COMPV_BASE64_ENCODE_BLOCK_SIZE		3 /* 24-bit input group */
// Decoding block size
#define COMPV_BASE64_DECODE_BLOCK_SIZE		4

// Guess the output(encoded) size
#define COMPV_BASE64_ENCODE_LEN(IN_LEN)		((2 + (IN_LEN) - (((IN_LEN) + 2) % 3)) * 4 / 3)

// Guess the output(decoded) size.
#define COMPV_BASE64_DECODE_LEN(IN_LEN)		(((IN_LEN * 3)/4) + 2)

/*==================================================================
Value Encoding  Value Encoding  Value Encoding  Value Encoding
0 A            17 R            34 i            51 z
1 B            18 S            35 j            52 0
2 C            19 T            36 k            53 1
3 D            20 U            37 l            54 2
4 E            21 V            38 m            55 3
5 F            22 W            39 n            56 4
6 G            23 X            40 o            57 5
7 H            24 Y            41 p            58 6
8 I            25 Z            42 q            59 7
9 J            26 a            43 r            60 8
10 K            27 b            44 s            61 9
11 L            28 c            45 t            62 +
12 M            29 d            46 u            63 /
13 N            30 e            47 v
14 O            31 f            48 w         (pad) =
15 P            32 g            49 x
16 Q            33 h            50 y
RFC 4548 - Table 1: The Base 64 Alphabet
*/

COMPV_NAMESPACE_BEGIN()

COMPV_ERROR_CODE CompVBase64::encode(const uint8_t* bufPtr, const size_t buffSize, std::string& base64)
{
	COMPV_CHECK_EXP_RETURN(!bufPtr || !buffSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	static const char COMPV_BASE64_ENCODE_ALPHABET[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	/*=================================================================================
	content					T					S					K
	ASCII					0x54				0x53				0x4B
	Binary					0101 0100			0101 0011			0100 1011
	------------------------------------------------------------------------------
	------------------------------------------------------------------------------
	Packs of 6bits			010101			000101			001101			001011
	Indexes					21				5				13				11
	Base64 encoded			V				F				N				L			<=== HERE IS THE RESULT OF tsk_base64_encode("TSK")
	*/

	size_t i = 0;
	base64 = "";

	/* Too short? */
	if (buffSize < COMPV_BASE64_ENCODE_BLOCK_SIZE) {
		goto quantum;
	}

	do {
		base64 += COMPV_BASE64_ENCODE_ALPHABET[(bufPtr[i] >> 2) & 0x3F];
		base64 += COMPV_BASE64_ENCODE_ALPHABET[((bufPtr[i] << 4) | (bufPtr[i + 1] >> 4)) & 0x3F];
		base64 += COMPV_BASE64_ENCODE_ALPHABET[((bufPtr[i + 1] << 2) | (bufPtr[i + 2] >> 6)) & 0x3F];
		base64 += COMPV_BASE64_ENCODE_ALPHABET[bufPtr[i + 2] & 0x3F];

		i += COMPV_BASE64_ENCODE_BLOCK_SIZE;
	} while ((i + COMPV_BASE64_ENCODE_BLOCK_SIZE) <= buffSize);

quantum:

	if ((buffSize - i) == 1) {
		/* The final quantum of encoding input is exactly 8 bits; here, the
		final unit of encoded output will be two characters followed by
		two "=" padding characters.
		*/
		base64 += COMPV_BASE64_ENCODE_ALPHABET[(bufPtr[i] >> 2) & 0x3F];
		base64 += COMPV_BASE64_ENCODE_ALPHABET[(bufPtr[i] << 4) & 0x3F];
		base64 += COMPV_BASE64_PAD, base64 += COMPV_BASE64_PAD;
	}
	else if ((buffSize - i) == 2) {
		/*	The final quantum of encoding input is exactly 16 bits; here, the
		final unit of encoded output will be three characters followed by
		one "=" padding character.
		*/
		base64 += COMPV_BASE64_ENCODE_ALPHABET[(bufPtr[i] >> 2) & 0x3F];
		base64 += COMPV_BASE64_ENCODE_ALPHABET[((bufPtr[i] << 4) | (bufPtr[i + 1] >> 4)) & 0x3F];
		base64 += COMPV_BASE64_ENCODE_ALPHABET[((bufPtr[i + 1] << 2) | (bufPtr[i + 2] >> 6)) & 0x3F];
		base64 += COMPV_BASE64_PAD;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBase64::decode(const std::string& base64, CompVBufferPtrPtr data)
{
	COMPV_CHECK_EXP_RETURN(base64.empty() || !data, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	static const uint8_t COMPV_BASE64_DECODE_ALPHABET[256] = {
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)62,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)63, (uint8_t)52, (uint8_t)53, (uint8_t)54, (uint8_t)55, (uint8_t)56, (uint8_t)57, (uint8_t)58, (uint8_t)59, (uint8_t)60, (uint8_t)61,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)0, (uint8_t)1, (uint8_t)2, (uint8_t)3, (uint8_t)4, (uint8_t)5, (uint8_t)6, (uint8_t)7, (uint8_t)8, (uint8_t)9, (uint8_t)10, (uint8_t)11, (uint8_t)12, (uint8_t)13, (uint8_t)14, (uint8_t)15, (uint8_t)16, (uint8_t)17, (uint8_t)18, (uint8_t)19, (uint8_t)20, (uint8_t)21, (uint8_t)22, (uint8_t)23, (uint8_t)24, (uint8_t)25,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)26, (uint8_t)27, (uint8_t)28, (uint8_t)29, (uint8_t)30, (uint8_t)31, (uint8_t)32, (uint8_t)33, (uint8_t)34, (uint8_t)35, (uint8_t)36, (uint8_t)37, (uint8_t)38, (uint8_t)39, (uint8_t)40, (uint8_t)41, (uint8_t)42, (uint8_t)43, (uint8_t)44, (uint8_t)45, (uint8_t)46, (uint8_t)47, (uint8_t)48, (uint8_t)49, (uint8_t)50, (uint8_t)51,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1, (uint8_t)-1,
		(uint8_t)-1,
	};

	size_t i, pay_size;
	size_t output_size = 0;
	const size_t input_size = base64.size();
	
	char* output = reinterpret_cast<char*>(CompVMem::calloc(1, (COMPV_BASE64_DECODE_LEN(input_size) + 1)));
	COMPV_CHECK_EXP_RETURN(!output, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	/* Count pads and remove them from the base64 string */
	for (i = input_size, pay_size = input_size; i > 0; i--) {
		if (base64[i - 1] == COMPV_BASE64_PAD) {
			pay_size--;
		}
		else {
			break;
		}
	}

	/* Reset i */
	i = 0;

	if (pay_size < COMPV_BASE64_DECODE_BLOCK_SIZE) {
		goto quantum;
	}

	do {
		*(output + output_size++) = (COMPV_BASE64_DECODE_ALPHABET[base64[i]] << 2
			| COMPV_BASE64_DECODE_ALPHABET[base64[i + 1]] >> 4);
		*(output + output_size++) = (COMPV_BASE64_DECODE_ALPHABET[base64[i + 1]] << 4
			| COMPV_BASE64_DECODE_ALPHABET[base64[i + 2]] >> 2);
		*(output + output_size++) = (COMPV_BASE64_DECODE_ALPHABET[base64[i + 2]] << 6
			| COMPV_BASE64_DECODE_ALPHABET[base64[i + 3]]);

		i += COMPV_BASE64_DECODE_BLOCK_SIZE;
	} while ((i + COMPV_BASE64_DECODE_BLOCK_SIZE) <= pay_size);

quantum:

	if ((input_size - pay_size) == 1) {
		*(output + output_size++) = (COMPV_BASE64_DECODE_ALPHABET[base64[i]] << 2
			| COMPV_BASE64_DECODE_ALPHABET[base64[i + 1]] >> 4);
		*(output + output_size++) = (COMPV_BASE64_DECODE_ALPHABET[base64[i + 1]] << 4
			| COMPV_BASE64_DECODE_ALPHABET[base64[i + 2]] >> 2);
	}
	else if ((input_size - pay_size) == 2) {
		*(output + output_size++) = (COMPV_BASE64_DECODE_ALPHABET[base64[i]] << 2
			| COMPV_BASE64_DECODE_ALPHABET[base64[i + 1]] >> 4);
	}

	const COMPV_ERROR_CODE err = CompVBuffer::newObjAndCopyData(
		output, output_size, data
	);
	CompVMem::free(reinterpret_cast<void**>(&output));
	COMPV_CHECK_CODE_RETURN(err);

	return err;
}

COMPV_NAMESPACE_END()
