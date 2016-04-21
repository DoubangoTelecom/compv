/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*  Copyright (C) ---- Colin Plumb
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#include "compv/compv_md5.h"
#include "compv/compv_engine.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// TODO(dmi): use intrinsics
static void __byteReverse(uint32_t *buf, unsigned words)
{
    if (CompVEngine::isBigEndian()) {
        COMPV_DEBUG_INFO_CODE_NOT_TESTED();
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
        uint8_t *p = (uint8_t *)buf;
        do {
            *buf++ = (uint32_t)((unsigned)p[3] << 8 | p[2]) << 16 | ((unsigned)p[1] << 8 | p[0]);
            p += 4;
        }
        while (--words);
    }
}

/*
* Converts hexadecimal bytes into string representation.
**/
static void __string_from_hex(const uint8_t *hex, size_t size, char* str)
{
    static const char *COMPV_HEXA_VALUES = { "0123456789abcdef" };
    size_t i;
    for (i = 0; i < size; i++) {
        str[2 * i] = COMPV_HEXA_VALUES[(*(hex + i) & 0xf0) >> 4];
        str[(2 * i) + 1] = COMPV_HEXA_VALUES[(*(hex + i) & 0x0f)];
    }
}

CompVMd5::CompVMd5()
{
    init();
}

CompVMd5::~CompVMd5()
{

}

void CompVMd5::init()
{
    m_buf[0] = 0x67452301;
    m_buf[1] = 0xefcdab89;
    m_buf[2] = 0x98badcfe;
    m_buf[3] = 0x10325476;

    m_bytes[0] = 0;
    m_bytes[1] = 0;
}

/*
* Update context to reflect the concatenation of another buffer full of bytes.
*/
COMPV_ERROR_CODE CompVMd5::update(uint8_t const *buf, size_t len)
{
    uint32_t t;

    /* Update byte count */

    t = m_bytes[0];
    if ((m_bytes[0] = t + (uint32_t)len) < t) {
        m_bytes[1]++;    /* Carry from low to high */
    }

    t = 64 - (t & 0x3f); 	/* Space available in m_in (at least 1) */
    if (t > len) {
        CompVMem::copy((uint8_t *)m_in + 64 - t, buf, len);
        return COMPV_ERROR_CODE_S_OK;
    }
    /* First chunk is an odd size */
    CompVMem::copy((uint8_t *)m_in + 64 - t, buf, t);
    __byteReverse(m_in, 16);
    transform(m_buf, m_in);
    buf += t;
    len -= t;

    /* Process data in 64-byte chunks */
    while (len >= 64) {
        CompVMem::copy(m_in, buf, 64);
        __byteReverse(m_in, 16);
        transform(m_buf, m_in);
        buf += 64;
        len -= 64;
    }

    /* Handle any remaining bytes of data. */
    if (len > 0) {
        CompVMem::copy(m_in, buf, len);
    }
    return COMPV_ERROR_CODE_S_OK;
}

/*
* Final wrapup - pad to 64-byte boundary with the bit pattern
* 1 0* (64-bit count of bits processed, MSB-first)
*/
COMPV_ERROR_CODE CompVMd5::final(compv_md5digest_t digest)
{
    int count = m_bytes[0] & 0x3f; 	/* Number of bytes in m_in */
    uint8_t *p = (uint8_t *)m_in + count;

    /* Set the first char of padding to 0x80.  There is always room. */
    *p++ = 0x80;

    /* Bytes of padding needed to make 56 bytes (-8..55) */
    count = 56 - 1 - count;

    if (count < 0) {	/* Padding forces an extra block */
        memset(p, 0, count + 8);
        __byteReverse(m_in, 16);
        transform(m_buf, m_in);
        p = (uint8_t *)m_in;
        count = 56;
    }
    memset(p, 0, count);
    __byteReverse(m_in, 14);

    /* Append length in bits and transform */
    m_in[14] = m_bytes[0] << 3;
    m_in[15] = m_bytes[1] << 3 | m_bytes[0] >> 29;
    transform(m_buf, m_in);

    __byteReverse(m_buf, 4);
    memcpy(digest, m_buf, 16);
#if 0
    memset(ctx, 0, sizeof(*ctx)); 	/* In case it's sensitive */
#else
    memset(&m_buf, 0, sizeof(m_buf));
    memset(&m_bytes, 0, sizeof(m_bytes));
    memset(&m_in, 0, sizeof(m_in));
#endif

    return COMPV_ERROR_CODE_S_OK;
}

/* The four core functions - F1 is optimized somewhat */

/* #define F1(x, y, z) (x & y | ~x & z) */
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define F2(x, y, z) F1(z, x, y)
#define F3(x, y, z) (x ^ y ^ z)
#define F4(x, y, z) (y ^ (x | ~z))

/* This is the central step in the MD5 algorithm. */
#define MD5STEP(f,w,x,y,z,in,s) \
(w += f(x,y,z) + in, w = (w<<s | w>>(32-s)) + x)

/*
* The core of the MD5 algorithm, this alters an existing MD5 hash to
* reflect the addition of 16 longwords of new data.  MD5Update blocks
* the data and converts bytes into longwords for this routine.
*/
COMPV_ERROR_CODE CompVMd5::transform(uint32_t buf[4], uint32_t const in[COMPV_MD5_DIGEST_SIZE])
{
    register uint32_t a, b, c, d;

    a = buf[0];
    b = buf[1];
    c = buf[2];
    d = buf[3];

    MD5STEP(F1, a, b, c, d, in[0] + 0xd76aa478, 7);
    MD5STEP(F1, d, a, b, c, in[1] + 0xe8c7b756, 12);
    MD5STEP(F1, c, d, a, b, in[2] + 0x242070db, 17);
    MD5STEP(F1, b, c, d, a, in[3] + 0xc1bdceee, 22);
    MD5STEP(F1, a, b, c, d, in[4] + 0xf57c0faf, 7);
    MD5STEP(F1, d, a, b, c, in[5] + 0x4787c62a, 12);
    MD5STEP(F1, c, d, a, b, in[6] + 0xa8304613, 17);
    MD5STEP(F1, b, c, d, a, in[7] + 0xfd469501, 22);
    MD5STEP(F1, a, b, c, d, in[8] + 0x698098d8, 7);
    MD5STEP(F1, d, a, b, c, in[9] + 0x8b44f7af, 12);
    MD5STEP(F1, c, d, a, b, in[10] + 0xffff5bb1, 17);
    MD5STEP(F1, b, c, d, a, in[11] + 0x895cd7be, 22);
    MD5STEP(F1, a, b, c, d, in[12] + 0x6b901122, 7);
    MD5STEP(F1, d, a, b, c, in[13] + 0xfd987193, 12);
    MD5STEP(F1, c, d, a, b, in[14] + 0xa679438e, 17);
    MD5STEP(F1, b, c, d, a, in[15] + 0x49b40821, 22);

    MD5STEP(F2, a, b, c, d, in[1] + 0xf61e2562, 5);
    MD5STEP(F2, d, a, b, c, in[6] + 0xc040b340, 9);
    MD5STEP(F2, c, d, a, b, in[11] + 0x265e5a51, 14);
    MD5STEP(F2, b, c, d, a, in[0] + 0xe9b6c7aa, 20);
    MD5STEP(F2, a, b, c, d, in[5] + 0xd62f105d, 5);
    MD5STEP(F2, d, a, b, c, in[10] + 0x02441453, 9);
    MD5STEP(F2, c, d, a, b, in[15] + 0xd8a1e681, 14);
    MD5STEP(F2, b, c, d, a, in[4] + 0xe7d3fbc8, 20);
    MD5STEP(F2, a, b, c, d, in[9] + 0x21e1cde6, 5);
    MD5STEP(F2, d, a, b, c, in[14] + 0xc33707d6, 9);
    MD5STEP(F2, c, d, a, b, in[3] + 0xf4d50d87, 14);
    MD5STEP(F2, b, c, d, a, in[8] + 0x455a14ed, 20);
    MD5STEP(F2, a, b, c, d, in[13] + 0xa9e3e905, 5);
    MD5STEP(F2, d, a, b, c, in[2] + 0xfcefa3f8, 9);
    MD5STEP(F2, c, d, a, b, in[7] + 0x676f02d9, 14);
    MD5STEP(F2, b, c, d, a, in[12] + 0x8d2a4c8a, 20);

    MD5STEP(F3, a, b, c, d, in[5] + 0xfffa3942, 4);
    MD5STEP(F3, d, a, b, c, in[8] + 0x8771f681, 11);
    MD5STEP(F3, c, d, a, b, in[11] + 0x6d9d6122, 16);
    MD5STEP(F3, b, c, d, a, in[14] + 0xfde5380c, 23);
    MD5STEP(F3, a, b, c, d, in[1] + 0xa4beea44, 4);
    MD5STEP(F3, d, a, b, c, in[4] + 0x4bdecfa9, 11);
    MD5STEP(F3, c, d, a, b, in[7] + 0xf6bb4b60, 16);
    MD5STEP(F3, b, c, d, a, in[10] + 0xbebfbc70, 23);
    MD5STEP(F3, a, b, c, d, in[13] + 0x289b7ec6, 4);
    MD5STEP(F3, d, a, b, c, in[0] + 0xeaa127fa, 11);
    MD5STEP(F3, c, d, a, b, in[3] + 0xd4ef3085, 16);
    MD5STEP(F3, b, c, d, a, in[6] + 0x04881d05, 23);
    MD5STEP(F3, a, b, c, d, in[9] + 0xd9d4d039, 4);
    MD5STEP(F3, d, a, b, c, in[12] + 0xe6db99e5, 11);
    MD5STEP(F3, c, d, a, b, in[15] + 0x1fa27cf8, 16);
    MD5STEP(F3, b, c, d, a, in[2] + 0xc4ac5665, 23);

    MD5STEP(F4, a, b, c, d, in[0] + 0xf4292244, 6);
    MD5STEP(F4, d, a, b, c, in[7] + 0x432aff97, 10);
    MD5STEP(F4, c, d, a, b, in[14] + 0xab9423a7, 15);
    MD5STEP(F4, b, c, d, a, in[5] + 0xfc93a039, 21);
    MD5STEP(F4, a, b, c, d, in[12] + 0x655b59c3, 6);
    MD5STEP(F4, d, a, b, c, in[3] + 0x8f0ccc92, 10);
    MD5STEP(F4, c, d, a, b, in[10] + 0xffeff47d, 15);
    MD5STEP(F4, b, c, d, a, in[1] + 0x85845dd1, 21);
    MD5STEP(F4, a, b, c, d, in[8] + 0x6fa87e4f, 6);
    MD5STEP(F4, d, a, b, c, in[15] + 0xfe2ce6e0, 10);
    MD5STEP(F4, c, d, a, b, in[6] + 0xa3014314, 15);
    MD5STEP(F4, b, c, d, a, in[13] + 0x4e0811a1, 21);
    MD5STEP(F4, a, b, c, d, in[4] + 0xf7537e82, 6);
    MD5STEP(F4, d, a, b, c, in[11] + 0xbd3af235, 10);
    MD5STEP(F4, c, d, a, b, in[2] + 0x2ad7d2bb, 15);
    MD5STEP(F4, b, c, d, a, in[9] + 0xeb86d391, 21);

    buf[0] += a;
    buf[1] += b;
    buf[2] += c;
    buf[3] += d;

    return COMPV_ERROR_CODE_S_OK;
}

std::string CompVMd5::compute(const void* input /*= NULL*/, size_t size /*= 0*/)
{
    if (input && size) {
        if (COMPV_ERROR_CODE_IS_NOK(update((const uint8_t*)(input), size))) {
            return "";
        }
    }
    compv_md5digest_t digest;
    if (COMPV_ERROR_CODE_IS_NOK((final(digest)))) {
        return "";
    }

    compv_md5string_t result;
    result[COMPV_MD5_STRING_SIZE] = '\0';
    __string_from_hex(digest, COMPV_MD5_DIGEST_SIZE, result);
    return std::string(result);
}

std::string CompVMd5::compute2(const void* input, size_t size)
{
    CompVObjWrapper<CompVMd5*> md5;
    if (COMPV_ERROR_CODE_IS_OK(CompVMd5::newObj(&md5))) {
        return md5->compute(input, size);
    }
    return "";
}

COMPV_ERROR_CODE CompVMd5::newObj(CompVObjWrapper<CompVMd5*>* md5)
{
    COMPV_CHECK_EXP_RETURN(!md5, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVObjWrapper<CompVMd5*> md5_;

    md5_ = new CompVMd5();
    if (!md5_) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    *md5 = md5_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
