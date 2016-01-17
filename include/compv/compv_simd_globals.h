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
#if !defined(_COMPV_SIMD_GLOBALS_H_)
#define _COMPV_SIMD_GLOBALS_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_EXTERNC_BEGIN()

COMPV_GEXTERN COMV_ALIGN_DEFAULT() uint8_t k_0_0_0_255_u8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t k5_i8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t k16_i8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int8_t k127_i8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() uint8_t k128_u8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() uint8_t k255_u8[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k16_i16[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k128_i16[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k255_i16[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k7120_i16[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k8912_i16[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int16_t k4400_i16[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int32_t k_0_2_4_6_0_2_4_6_i32[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int64_t kAVXMaskstore_0_i64[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int64_t kAVXMaskstore_0_1_i64[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int32_t kAVXMaskstore_0_i32[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_AEBFCGDH_i32[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_ABCDDEFG_i32[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_CDEFFGHX_i32[];
COMPV_GEXTERN COMV_ALIGN_DEFAULT() int32_t kAVXPermutevar8x32_XXABBCDE_i32[];

COMPV_EXTERNC_END()

#endif /* _COMPV_SIMD_GLOBALS_H_ */
