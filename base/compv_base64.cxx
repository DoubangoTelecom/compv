/*
base64.cpp and base64.h

base64 encoding and decoding with C++.

Version: 1.01.00

Copyright (C) 2004-2017 René Nyffenegger

This source code is provided 'as-is', without any express or implied
warranty. In no event will the author be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

1. The origin of this source code must not be misrepresented; you must not
claim that you wrote the original source code. If you use this source code
in a product, an acknowledgment in the product documentation would be
appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be
misrepresented as being the original source code.

3. This notice may not be removed or altered from any source distribution.

René Nyffenegger rene.nyffenegger@adp-gmbh.ch

*/
#include "compv/base/compv_base64.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"

// TODO(dmi): My original Base64 implementation at https://github.com/DoubangoTelecom/doubango/blob/master/tinySAK/src/tsk_base64.c
// is buggy (the encoder at least). Have to fix it.

COMPV_NAMESPACE_BEGIN()

static const std::string base64_chars =
"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";


static inline bool is_base64(unsigned char c) {
	return (isalnum(c) || (c == '+') || (c == '/'));
}

COMPV_ERROR_CODE CompVBase64::encode(const uint8_t* bufPtr, const size_t buffSize, std::string& base64)
{
	COMPV_CHECK_EXP_RETURN(!bufPtr || !buffSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	int i = 0;
	int j = 0;
	int in_len = (int)buffSize;
	uint8_t char_array_3[3];
	uint8_t char_array_4[4];

	base64 = "";

	while (in_len--) {
		char_array_3[i++] = *(bufPtr++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for (i = 0; (i <4); i++)
				base64 += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);

		for (j = 0; (j < i + 1); j++)
			base64 += base64_chars[char_array_4[j]];

		while ((i++ < 3))
			base64 += '=';

	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBase64::decode(const std::string& base64, CompVBufferPtrPtr data)
{
	COMPV_CHECK_EXP_RETURN(base64.empty() || !data, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	int in_len = (int)base64.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	uint8_t char_array_4[4], char_array_3[3];
	std::string ret;

	while (in_len-- && (base64[in_] != '=') && is_base64(base64[in_])) {
		char_array_4[i++] = base64[in_]; in_++;
		if (i == 4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = (uint8_t)base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = 0; j < i; j++)
			char_array_4[j] = (uint8_t)base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);

		for (j = 0; (j < i - 1); j++) ret += char_array_3[j];
	}

	COMPV_CHECK_EXP_RETURN(ret.empty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Not base64 data");

	COMPV_CHECK_CODE_RETURN(CompVBuffer::newObj(ret.c_str(), ret.size(), data));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
