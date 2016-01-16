/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
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
#include "compv/compv_simd_globals.h"
#include "compv/intrinsics/x86/compv_intrin_avx.h"

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t k5_i8[] = {
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5, 5,
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t k16_i8[] = {
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t k127_i8[] = {
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
    127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127, 127,
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() uint8_t k128_u8[] = {
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() uint8_t k255_u8[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k16_i16[] = {
    16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16, 16,
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k128_i16[] = {
    128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k255_i16[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k7120_i16[] = {
    7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120,
    7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120, 7120,
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k8912_i16[] = {
    8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912,
    8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912, 8912,
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k4400_i16[] = {
    4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400,
    4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400, 4400,
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int64_t kAVXMaskstore_0_i64[] = {
    0xF000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int64_t kAVXMaskstore_0_1_i64[] = {
    0xF000000000000000, 0xF000000000000000, 0x0000000000000000, 0x0000000000000000
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int32_t kAVXMaskstore_0_i32[] = {
    0xF0000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_AEBFCGDH_i32[] = { // = k_0_4_1_5_2_6_3_7
    COMPV_AVX_A64, COMPV_AVX_E64, COMPV_AVX_B64, COMPV_AVX_F64, COMPV_AVX_C64, COMPV_AVX_G64, COMPV_AVX_D64, COMPV_AVX_H64
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_ABCDDEFG_i32[] = { // = k_0_1_2_3_3_4_5_6
	COMPV_AVX_A64, COMPV_AVX_B64, COMPV_AVX_C64, COMPV_AVX_D64, COMPV_AVX_D64, COMPV_AVX_E64, COMPV_AVX_F64, COMPV_AVX_G64
};