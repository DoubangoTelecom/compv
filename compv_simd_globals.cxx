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

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int32_t k_0_2_4_6_0_2_4_6_i32[] = {
	0, 2, 4, 6, 0, 2, 4, 6
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int64_t kMaskstore_0_i64[] = {
	0xF000000000000000, 0x0000000000000000, 0x0000000000000000, 0x0000000000000000
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int64_t kMaskstore_0_1_i64[] = {
	0xF000000000000000, 0xF000000000000000, 0x0000000000000000, 0x0000000000000000
};

COMPV_GEXTERN COMV_ALIGN_DEFAULT() int32_t kMaskstore_0_i32[] = {
	0xF0000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000
};