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
/* @description
* This class implements ORB (Oriented FAST and Rotated BRIEF) feature descriptor.
* Some literature:
* ORB final: https://www.willowgarage.com/sites/default/files/orb_final.pdf
* BRIEF descriptor: https://www.robots.ox.ac.uk/~vgg/rg/papers/CalonderLSF10.pdf
* Measuring Corner Properties: http://users.cs.cf.ac.uk/Paul.Rosin/corner2.pdf (check "Intensity centroid" used in ORB vs "Gradient centroid")
* Image moments: https://en.wikipedia.org/wiki/Image_moment
* Centroid: https://en.wikipedia.org/wiki/Centroid
*/
#include "compv/features/orb/compv_feature_orb_desc.h"
#include "compv/features/orb/compv_feature_orb_dete.h"
#include "compv/math/compv_math_utils.h"
#include "compv/compv_engine.h"
#include "compv/compv_mem.h"
#include "compv/compv_cpu.h"
#include "compv/compv_gauss.h"
#include "compv/compv_debug.h"

#include "compv/intrinsics/x86/features/orb/compv_feature_orb_desc_intrin_sse.h"
#include "compv/intrinsics/x86/features/orb/compv_feature_orb_desc_intrin_avx2.h"

#include <algorithm>

#if COMPV_ARCH_X86 && COMPV_ASM
COMPV_EXTERNC void Brief256_31_Asm_X86_SSE41(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
COMPV_EXTERNC void Brief256_31_Asm_X86_AVX2(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
COMPV_EXTERNC void Brief256_31_Asm_X86_FMA3_AVX2(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */
#if COMPV_ARCH_X64 && COMPV_ASM
COMPV_EXTERNC void Brief256_31_Asm_X64_SSE41(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
COMPV_EXTERNC void Brief256_31_Asm_X64_AVX2(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
COMPV_EXTERNC void Brief256_31_Asm_X64_FMA3_AVX2(const uint8_t* img_center, compv::compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(SSE) void* out);
#endif /* COMPV_ARCH_X86 && COMPV_ASM */

#define _kBrief256Pattern31AX_(q) \
	8*(1<<(q)), 4*(1<<(q)), -11*(1<<(q)), 7*(1<<(q)), 2*(1<<(q)), 1*(1<<(q)), -2*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), 10*(1<<(q)), -13*(1<<(q)), -11*(1<<(q)), 7*(1<<(q)), -4*(1<<(q)), -13*(1<<(q)), -9*(1<<(q)), 12*(1<<(q)), -3*(1<<(q)), -6*(1<<(q)), 11*(1<<(q)),  \
	4*(1<<(q)), 5*(1<<(q)), 3*(1<<(q)), -8*(1<<(q)), -2*(1<<(q)), -13*(1<<(q)), -7*(1<<(q)), -4*(1<<(q)), -10*(1<<(q)), 5*(1<<(q)), 5*(1<<(q)), 1*(1<<(q)), 9*(1<<(q)), 4*(1<<(q)), 2*(1<<(q)), -4*(1<<(q)), -8*(1<<(q)), 4*(1<<(q)), 0*(1<<(q)), -13*(1<<(q)), -3*(1<<(q)), -6*(1<<(q)), \
	8*(1<<(q)), 0*(1<<(q)), 7*(1<<(q)), -13*(1<<(q)), 10*(1<<(q)), -6*(1<<(q)), 10*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), 3*(1<<(q)), 5*(1<<(q)), -1*(1<<(q)), 3*(1<<(q)), 2*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), -7*(1<<(q)), 6*(1<<(q)), -9*(1<<(q)), -2*(1<<(q)), \
	-12*(1<<(q)), 3*(1<<(q)), -7*(1<<(q)), -3*(1<<(q)), 2*(1<<(q)), -11*(1<<(q)), -1*(1<<(q)), 5*(1<<(q)), -4*(1<<(q)), -9*(1<<(q)), -12*(1<<(q)), 10*(1<<(q)), 7*(1<<(q)), -7*(1<<(q)), -4*(1<<(q)), 7*(1<<(q)), -7*(1<<(q)), -13*(1<<(q)), -3*(1<<(q)), 7*(1<<(q)), \
	-13*(1<<(q)), 1*(1<<(q)), 2*(1<<(q)), -4*(1<<(q)), -1*(1<<(q)), 7*(1<<(q)), 1*(1<<(q)), 9*(1<<(q)), -1*(1<<(q)), -13*(1<<(q)), 7*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), 5*(1<<(q)), 2*(1<<(q)), 3*(1<<(q)), 2*(1<<(q)), 9*(1<<(q)), -8*(1<<(q)), -11*(1<<(q)), 1*(1<<(q)), 6*(1<<(q)), 2*(1<<(q)), \
	6*(1<<(q)), 3*(1<<(q)), 7*(1<<(q)), -11*(1<<(q)), -10*(1<<(q)), -5*(1<<(q)), -10*(1<<(q)), 8*(1<<(q)), 4*(1<<(q)), -10*(1<<(q)), 4*(1<<(q)), -2*(1<<(q)), -5*(1<<(q)), 7*(1<<(q)), -9*(1<<(q)), -5*(1<<(q)), 8*(1<<(q)), -9*(1<<(q)), 1*(1<<(q)), 7*(1<<(q)), -2*(1<<(q)), 11*(1<<(q)), \
	-12*(1<<(q)), 3*(1<<(q)), 5*(1<<(q)), 0*(1<<(q)), -9*(1<<(q)), 0*(1<<(q)), -1*(1<<(q)), 5*(1<<(q)), 3*(1<<(q)), -13*(1<<(q)), -5*(1<<(q)), -4*(1<<(q)), 6*(1<<(q)), -7*(1<<(q)), -13*(1<<(q)), 1*(1<<(q)), 4*(1<<(q)), -2*(1<<(q)), 2*(1<<(q)), -2*(1<<(q)), 4*(1<<(q)), -6*(1<<(q)), \
	-3*(1<<(q)), 7*(1<<(q)), 4*(1<<(q)), -13*(1<<(q)), 7*(1<<(q)), 7*(1<<(q)), -7*(1<<(q)), -8*(1<<(q)), -13*(1<<(q)), 2*(1<<(q)), 10*(1<<(q)), -6*(1<<(q)), 8*(1<<(q)), 2*(1<<(q)), -11*(1<<(q)), -12*(1<<(q)), -11*(1<<(q)), 5*(1<<(q)), -2*(1<<(q)), -1*(1<<(q)), -13*(1<<(q)), \
	-10*(1<<(q)), -3*(1<<(q)), 2*(1<<(q)), -9*(1<<(q)), -4*(1<<(q)), -4*(1<<(q)), -6*(1<<(q)), 6*(1<<(q)), -13*(1<<(q)), 11*(1<<(q)), 7*(1<<(q)), -1*(1<<(q)), -4*(1<<(q)), -7*(1<<(q)), -13*(1<<(q)), -7*(1<<(q)), -8*(1<<(q)), -5*(1<<(q)), -13*(1<<(q)), \
	1*(1<<(q)), 1*(1<<(q)), 9*(1<<(q)), 5*(1<<(q)), -1*(1<<(q)), -9*(1<<(q)), -1*(1<<(q)), -13*(1<<(q)), 8*(1<<(q)), 2*(1<<(q)), 7*(1<<(q)), -10*(1<<(q)), -10*(1<<(q)), 4*(1<<(q)), 3*(1<<(q)), -4*(1<<(q)), 5*(1<<(q)), 4*(1<<(q)), -9*(1<<(q)), 0*(1<<(q)), -12*(1<<(q)), 3*(1<<(q)), \
	-10*(1<<(q)), 8*(1<<(q)), -8*(1<<(q)), 2*(1<<(q)), 10*(1<<(q)), 6*(1<<(q)), -7*(1<<(q)), -3*(1<<(q)), -1*(1<<(q)), -3*(1<<(q)), -8*(1<<(q)), 4*(1<<(q)), 2*(1<<(q)), 6*(1<<(q)), 3*(1<<(q)), 11*(1<<(q)), -3*(1<<(q)), 4*(1<<(q)), 2*(1<<(q)), -10*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), \
	6*(1<<(q)), 0*(1<<(q)), -13*(1<<(q)), -9*(1<<(q)), -13*(1<<(q)), 5*(1<<(q)), 2*(1<<(q)), -1*(1<<(q)), 9*(1<<(q)), 11*(1<<(q)), 3*(1<<(q)), -1*(1<<(q)), 3*(1<<(q)), -13*(1<<(q)), 5*(1<<(q)), 8*(1<<(q)), 7*(1<<(q)), -10*(1<<(q)), 7*(1<<(q)), 9*(1<<(q)), 7*(1<<(q)), -1*(1<<(q))
#define _kBrief256Pattern31AY_(q) \
	-3*(1<<(q)), 2*(1<<(q)), 9*(1<<(q)), -12*(1<<(q)), -13*(1<<(q)), -7*(1<<(q)), -10*(1<<(q)), -13*(1<<(q)), -3*(1<<(q)), 4*(1<<(q)), -8*(1<<(q)), 7*(1<<(q)), 7*(1<<(q)), -5*(1<<(q)), 2*(1<<(q)), 0*(1<<(q)), -6*(1<<(q)), 6*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), 7*(1<<(q)), \
	-3*(1<<(q)), -7*(1<<(q)), -7*(1<<(q)), 11*(1<<(q)), 12*(1<<(q)), 3*(1<<(q)), 2*(1<<(q)), -12*(1<<(q)), -12*(1<<(q)), -6*(1<<(q)), 0*(1<<(q)), 11*(1<<(q)), 7*(1<<(q)), -1*(1<<(q)), -12*(1<<(q)), -5*(1<<(q)), 11*(1<<(q)), -8*(1<<(q)), -2*(1<<(q)), -2*(1<<(q)), \
	9*(1<<(q)), 12*(1<<(q)), 9*(1<<(q)), -5*(1<<(q)), -6*(1<<(q)), 7*(1<<(q)), -3*(1<<(q)), -9*(1<<(q)), 8*(1<<(q)), 0*(1<<(q)), 3*(1<<(q)), 7*(1<<(q)), 7*(1<<(q)), -10*(1<<(q)), -4*(1<<(q)), 0*(1<<(q)), -7*(1<<(q)), 3*(1<<(q)), 12*(1<<(q)), -10*(1<<(q)), -1*(1<<(q)), -5*(1<<(q)), \
	5*(1<<(q)), -10*(1<<(q)), -7*(1<<(q)), -2*(1<<(q)), 9*(1<<(q)), -13*(1<<(q)), 6*(1<<(q)), -3*(1<<(q)), -13*(1<<(q)), -6*(1<<(q)), -10*(1<<(q)), 2*(1<<(q)), 12*(1<<(q)), -13*(1<<(q)), 9*(1<<(q)), -1*(1<<(q)), 6*(1<<(q)), 11*(1<<(q)), 7*(1<<(q)), -8*(1<<(q)), -7*(1<<(q)), \
	-3*(1<<(q)), -6*(1<<(q)), 3*(1<<(q)), -13*(1<<(q)), 1*(1<<(q)), -1*(1<<(q)), 1*(1<<(q)), -9*(1<<(q)), -13*(1<<(q)), 7*(1<<(q)), -5*(1<<(q)), 3*(1<<(q)), -13*(1<<(q)), -12*(1<<(q)), 8*(1<<(q)), 6*(1<<(q)), -12*(1<<(q)), 4*(1<<(q)), 12*(1<<(q)), 12*(1<<(q)), -9*(1<<(q)), \
	3*(1<<(q)), 3*(1<<(q)), -3*(1<<(q)), 8*(1<<(q)), -5*(1<<(q)), 11*(1<<(q)), -8*(1<<(q)), 5*(1<<(q)), -1*(1<<(q)), -6*(1<<(q)), 12*(1<<(q)), -2*(1<<(q)), 0*(1<<(q)), -8*(1<<(q)), -6*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), -8*(1<<(q)), -11*(1<<(q)), -8*(1<<(q)), \
	-4*(1<<(q)), 1*(1<<(q)), -6*(1<<(q)), -9*(1<<(q)), 7*(1<<(q)), 5*(1<<(q)), -4*(1<<(q)), 12*(1<<(q)), 7*(1<<(q)), 2*(1<<(q)), 11*(1<<(q)), 5*(1<<(q)), -4*(1<<(q)), 9*(1<<(q)), -7*(1<<(q)), 5*(1<<(q)), 6*(1<<(q)), 6*(1<<(q)), -10*(1<<(q)), 1*(1<<(q)), -2*(1<<(q)), -12*(1<<(q)), \
	-13*(1<<(q)), 1*(1<<(q)), -10*(1<<(q)), -13*(1<<(q)), 5*(1<<(q)), -2*(1<<(q)), 9*(1<<(q)), 1*(1<<(q)), -8*(1<<(q)), -4*(1<<(q)), 11*(1<<(q)), 6*(1<<(q)), 4*(1<<(q)), -5*(1<<(q)), -5*(1<<(q)), -3*(1<<(q)), -12*(1<<(q)), -2*(1<<(q)), -13*(1<<(q)), 0*(1<<(q)), -3*(1<<(q)), \
	-13*(1<<(q)), -8*(1<<(q)), -11*(1<<(q)), -2*(1<<(q)), 9*(1<<(q)), -3*(1<<(q)), -13*(1<<(q)), 6*(1<<(q)), 12*(1<<(q)), -11*(1<<(q)), -3*(1<<(q)), 11*(1<<(q)), 11*(1<<(q)), -5*(1<<(q)), 12*(1<<(q)), -8*(1<<(q)), 1*(1<<(q)), -12*(1<<(q)), -2*(1<<(q)), \
	5*(1<<(q)), -1*(1<<(q)), 7*(1<<(q)), 5*(1<<(q)), 0*(1<<(q)), 12*(1<<(q)), -8*(1<<(q)), 11*(1<<(q)), -3*(1<<(q)), -10*(1<<(q)), 1*(1<<(q)), -11*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), -10*(1<<(q)), -8*(1<<(q)), -6*(1<<(q)), 12*(1<<(q)), 2*(1<<(q)), -13*(1<<(q)), \
	-13*(1<<(q)), 9*(1<<(q)), 3*(1<<(q)), 1*(1<<(q)), 2*(1<<(q)), -10*(1<<(q)), -13*(1<<(q)), -12*(1<<(q)), 2*(1<<(q)), 6*(1<<(q)), 8*(1<<(q)), 10*(1<<(q)), -9*(1<<(q)), -13*(1<<(q)), -7*(1<<(q)), -2*(1<<(q)), 2*(1<<(q)), -5*(1<<(q)), -9*(1<<(q)), -1*(1<<(q)), -1*(1<<(q)), \
	0*(1<<(q)), -11*(1<<(q)), -4*(1<<(q)), -6*(1<<(q)), 7*(1<<(q)), 12*(1<<(q)), 0*(1<<(q)), -1*(1<<(q)), 3*(1<<(q)), 8*(1<<(q)), -6*(1<<(q)), -9*(1<<(q)), 7*(1<<(q)), -6*(1<<(q)), 5*(1<<(q)), -3*(1<<(q)), 0*(1<<(q)), 4*(1<<(q)), -6*(1<<(q)), 0*(1<<(q)), 8*(1<<(q)), 9*(1<<(q)), -4*(1<<(q)), \
	4*(1<<(q)), 3*(1<<(q)), -7*(1<<(q)), 0*(1<<(q)), -6*(1<<(q))
#define _kBrief256Pattern31BX_(q) \
	9*(1<<(q)), 7*(1<<(q)), -8*(1<<(q)), 12*(1<<(q)), 2*(1<<(q)), 1*(1<<(q)), -2*(1<<(q)), -11*(1<<(q)), -12*(1<<(q)), 11*(1<<(q)), -8*(1<<(q)), -9*(1<<(q)), 12*(1<<(q)), -3*(1<<(q)), -12*(1<<(q)), -7*(1<<(q)), 12*(1<<(q)), -2*(1<<(q)), -4*(1<<(q)), 12*(1<<(q)), 5*(1<<(q)), \
	10*(1<<(q)), 6*(1<<(q)), -6*(1<<(q)), -1*(1<<(q)), -8*(1<<(q)), -5*(1<<(q)), -3*(1<<(q)), -6*(1<<(q)), 6*(1<<(q)), 7*(1<<(q)), 4*(1<<(q)), 11*(1<<(q)), 4*(1<<(q)), 4*(1<<(q)), -2*(1<<(q)), -7*(1<<(q)), 9*(1<<(q)), 1*(1<<(q)), -8*(1<<(q)), -2*(1<<(q)), -4*(1<<(q)), 10*(1<<(q)), \
	1*(1<<(q)), 11*(1<<(q)), -11*(1<<(q)), 12*(1<<(q)), -6*(1<<(q)), 12*(1<<(q)), -8*(1<<(q)), -8*(1<<(q)), 7*(1<<(q)), 10*(1<<(q)), 1*(1<<(q)), 5*(1<<(q)), 3*(1<<(q)), -13*(1<<(q)), -12*(1<<(q)), -11*(1<<(q)), -4*(1<<(q)), 12*(1<<(q)), -7*(1<<(q)), 0*(1<<(q)), \
	-7*(1<<(q)), 8*(1<<(q)), -4*(1<<(q)), -1*(1<<(q)), 5*(1<<(q)), -5*(1<<(q)), 0*(1<<(q)), 5*(1<<(q)), -4*(1<<(q)), -9*(1<<(q)), -8*(1<<(q)), 12*(1<<(q)), 12*(1<<(q)), -6*(1<<(q)), -3*(1<<(q)), 12*(1<<(q)), -5*(1<<(q)), -12*(1<<(q)), -2*(1<<(q)), 12*(1<<(q)), -11*(1<<(q)), \
	12*(1<<(q)), 3*(1<<(q)), -2*(1<<(q)), 1*(1<<(q)), 8*(1<<(q)), 3*(1<<(q)), 12*(1<<(q)), -1*(1<<(q)), -10*(1<<(q)), 10*(1<<(q)), 12*(1<<(q)), 7*(1<<(q)), 6*(1<<(q)), 2*(1<<(q)), 4*(1<<(q)), 12*(1<<(q)), 10*(1<<(q)), -7*(1<<(q)), -4*(1<<(q)), 2*(1<<(q)), 7*(1<<(q)), 3*(1<<(q)), \
	11*(1<<(q)), 8*(1<<(q)), 9*(1<<(q)), -6*(1<<(q)), -5*(1<<(q)), -3*(1<<(q)), -9*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), -8*(1<<(q)), 6*(1<<(q)), -2*(1<<(q)), -5*(1<<(q)), 10*(1<<(q)), -8*(1<<(q)), -5*(1<<(q)), 9*(1<<(q)), -9*(1<<(q)), 1*(1<<(q)), 9*(1<<(q)), -1*(1<<(q)), 12*(1<<(q)), \
	-6*(1<<(q)), 7*(1<<(q)), 10*(1<<(q)), 2*(1<<(q)), -5*(1<<(q)), 2*(1<<(q)), 1*(1<<(q)), 7*(1<<(q)), 6*(1<<(q)), -8*(1<<(q)), -3*(1<<(q)), -3*(1<<(q)), 8*(1<<(q)), -6*(1<<(q)), -5*(1<<(q)), 3*(1<<(q)), 8*(1<<(q)), 2*(1<<(q)), 12*(1<<(q)), 0*(1<<(q)), 9*(1<<(q)), -3*(1<<(q)), -1*(1<<(q)), \
	12*(1<<(q)), 5*(1<<(q)), -9*(1<<(q)), 8*(1<<(q)), 7*(1<<(q)), -7*(1<<(q)), -7*(1<<(q)), -12*(1<<(q)), 3*(1<<(q)), 12*(1<<(q)), -6*(1<<(q)), 9*(1<<(q)), 2*(1<<(q)), -10*(1<<(q)), -7*(1<<(q)), -10*(1<<(q)), 11*(1<<(q)), -1*(1<<(q)), 0*(1<<(q)), -12*(1<<(q)), -10*(1<<(q)), \
	-2*(1<<(q)), 3*(1<<(q)), -4*(1<<(q)), -3*(1<<(q)), -2*(1<<(q)), -4*(1<<(q)), 6*(1<<(q)), -5*(1<<(q)), 12*(1<<(q)), 12*(1<<(q)), 0*(1<<(q)), -3*(1<<(q)), -6*(1<<(q)), -8*(1<<(q)), -6*(1<<(q)), -6*(1<<(q)), -4*(1<<(q)), -8*(1<<(q)), 5*(1<<(q)), 10*(1<<(q)), 10*(1<<(q)), \
	10*(1<<(q)), 1*(1<<(q)), -6*(1<<(q)), 1*(1<<(q)), -8*(1<<(q)), 10*(1<<(q)), 3*(1<<(q)), 12*(1<<(q)), -5*(1<<(q)), -8*(1<<(q)), 8*(1<<(q)), 8*(1<<(q)), -3*(1<<(q)), 10*(1<<(q)), 5*(1<<(q)), -4*(1<<(q)), 3*(1<<(q)), -6*(1<<(q)), 4*(1<<(q)), -10*(1<<(q)), 12*(1<<(q)), \
	-6*(1<<(q)), 3*(1<<(q)), 11*(1<<(q)), 8*(1<<(q)), -6*(1<<(q)), -3*(1<<(q)), -1*(1<<(q)), -3*(1<<(q)), -8*(1<<(q)), 12*(1<<(q)), 3*(1<<(q)), 11*(1<<(q)), 7*(1<<(q)), 12*(1<<(q)), -3*(1<<(q)), 4*(1<<(q)), 2*(1<<(q)), -8*(1<<(q)), -11*(1<<(q)), -11*(1<<(q)), 11*(1<<(q)), \
	1*(1<<(q)), -9*(1<<(q)), -6*(1<<(q)), -8*(1<<(q)), 8*(1<<(q)), 3*(1<<(q)), -1*(1<<(q)), 11*(1<<(q)), 12*(1<<(q)), 3*(1<<(q)), 0*(1<<(q)), 4*(1<<(q)), -10*(1<<(q)), 12*(1<<(q)), 9*(1<<(q)), 8*(1<<(q)), -10*(1<<(q)), 12*(1<<(q)), 10*(1<<(q)), 12*(1<<(q)), 0*(1<<(q))
#define _kBrief256Pattern31BY_(q) \
	5*(1<<(q)), -12*(1<<(q)), 2*(1<<(q)), -13*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), -4*(1<<(q)), -8*(1<<(q)), -9*(1<<(q)), 9*(1<<(q)), -9*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), 0*(1<<(q)), -3*(1<<(q)), 5*(1<<(q)), -1*(1<<(q)), 12*(1<<(q)), -8*(1<<(q)), -8*(1<<(q)), 1*(1<<(q)), -3*(1<<(q)), \
	12*(1<<(q)), -2*(1<<(q)), -10*(1<<(q)), 10*(1<<(q)), -3*(1<<(q)), 7*(1<<(q)), 11*(1<<(q)), -7*(1<<(q)), -1*(1<<(q)), -5*(1<<(q)), -13*(1<<(q)), 12*(1<<(q)), 4*(1<<(q)), 7*(1<<(q)), -10*(1<<(q)), 12*(1<<(q)), -13*(1<<(q)), 2*(1<<(q)), 3*(1<<(q)), -9*(1<<(q)), \
	7*(1<<(q)), 3*(1<<(q)), -10*(1<<(q)), 0*(1<<(q)), 1*(1<<(q)), 12*(1<<(q)), -4*(1<<(q)), -12*(1<<(q)), -4*(1<<(q)), 8*(1<<(q)), -7*(1<<(q)), -12*(1<<(q)), 6*(1<<(q)), -10*(1<<(q)), 5*(1<<(q)), 12*(1<<(q)), 8*(1<<(q)), 7*(1<<(q)), 8*(1<<(q)), -6*(1<<(q)), 12*(1<<(q)), 5*(1<<(q)), \
	-13*(1<<(q)), 5*(1<<(q)), -7*(1<<(q)), -11*(1<<(q)), -13*(1<<(q)), -1*(1<<(q)), 2*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), -4*(1<<(q)), -3*(1<<(q)), 12*(1<<(q)), 5*(1<<(q)), 4*(1<<(q)), 2*(1<<(q)), 1*(1<<(q)), 5*(1<<(q)), -6*(1<<(q)), -7*(1<<(q)), -12*(1<<(q)), 12*(1<<(q)), \
	0*(1<<(q)), -13*(1<<(q)), 9*(1<<(q)), -6*(1<<(q)), 12*(1<<(q)), 6*(1<<(q)), 3*(1<<(q)), 5*(1<<(q)), 12*(1<<(q)), 9*(1<<(q)), 11*(1<<(q)), 10*(1<<(q)), 3*(1<<(q)), -6*(1<<(q)), -13*(1<<(q)), 3*(1<<(q)), 9*(1<<(q)), -6*(1<<(q)), -8*(1<<(q)), -4*(1<<(q)), -2*(1<<(q)), 0*(1<<(q)), \
	-8*(1<<(q)), 3*(1<<(q)), -4*(1<<(q)), 10*(1<<(q)), 12*(1<<(q)), 0*(1<<(q)), -6*(1<<(q)), -11*(1<<(q)), 7*(1<<(q)), 7*(1<<(q)), 12*(1<<(q)), 2*(1<<(q)), 12*(1<<(q)), -8*(1<<(q)), -2*(1<<(q)), -13*(1<<(q)), 0*(1<<(q)), -2*(1<<(q)), 1*(1<<(q)), -4*(1<<(q)), -11*(1<<(q)), \
	4*(1<<(q)), 12*(1<<(q)), 8*(1<<(q)), 8*(1<<(q)), -13*(1<<(q)), 12*(1<<(q)), 7*(1<<(q)), -9*(1<<(q)), -8*(1<<(q)), 9*(1<<(q)), -3*(1<<(q)), -12*(1<<(q)), 0*(1<<(q)), 12*(1<<(q)), -2*(1<<(q)), 10*(1<<(q)), -4*(1<<(q)), -13*(1<<(q)), 12*(1<<(q)), -6*(1<<(q)), 3*(1<<(q)), \
	-5*(1<<(q)), 1*(1<<(q)), -11*(1<<(q)), -7*(1<<(q)), -5*(1<<(q)), 6*(1<<(q)), 6*(1<<(q)), 1*(1<<(q)), -8*(1<<(q)), -8*(1<<(q)), 9*(1<<(q)), 3*(1<<(q)), 7*(1<<(q)), -8*(1<<(q)), 8*(1<<(q)), 3*(1<<(q)), -9*(1<<(q)), -5*(1<<(q)), 8*(1<<(q)), 12*(1<<(q)), 9*(1<<(q)), -5*(1<<(q)), \
	11*(1<<(q)), -13*(1<<(q)), 2*(1<<(q)), 0*(1<<(q)), -10*(1<<(q)), -7*(1<<(q)), 9*(1<<(q)), 11*(1<<(q)), 5*(1<<(q)), 6*(1<<(q)), -2*(1<<(q)), 7*(1<<(q)), -2*(1<<(q)), 7*(1<<(q)), -13*(1<<(q)), -8*(1<<(q)), -9*(1<<(q)), 5*(1<<(q)), 10*(1<<(q)), -13*(1<<(q)), -13*(1<<(q)), \
	-1*(1<<(q)), -9*(1<<(q)), -13*(1<<(q)), 2*(1<<(q)), 12*(1<<(q)), -10*(1<<(q)), -6*(1<<(q)), -6*(1<<(q)), -9*(1<<(q)), -7*(1<<(q)), -13*(1<<(q)), 5*(1<<(q)), -13*(1<<(q)), -3*(1<<(q)), -12*(1<<(q)), -1*(1<<(q)), 3*(1<<(q)), -9*(1<<(q)), 1*(1<<(q)), -8*(1<<(q)), \
	9*(1<<(q)), 12*(1<<(q)), -5*(1<<(q)), 7*(1<<(q)), -8*(1<<(q)), -12*(1<<(q)), 5*(1<<(q)), 9*(1<<(q)), 5*(1<<(q)), 4*(1<<(q)), 3*(1<<(q)), 12*(1<<(q)), 11*(1<<(q)), -13*(1<<(q)), 12*(1<<(q)), 4*(1<<(q)), 6*(1<<(q)), 12*(1<<(q)), 1*(1<<(q)), 1*(1<<(q)), 1*(1<<(q)), -13*(1<<(q)), \
	-13*(1<<(q)), 4*(1<<(q)), -2*(1<<(q)), -3*(1<<(q)), -2*(1<<(q)), 10*(1<<(q)), -9*(1<<(q)), -1*(1<<(q)), -2*(1<<(q)), -8*(1<<(q)), 5*(1<<(q)), 10*(1<<(q)), 5*(1<<(q)), 5*(1<<(q)), 11*(1<<(q)), -6*(1<<(q)), -12*(1<<(q)), 9*(1<<(q)), 4*(1<<(q)), -2*(1<<(q)), -2*(1<<(q)), -11*(1<<(q))

COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AX[256] = { _kBrief256Pattern31AX_(0) };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31AY[256] = { _kBrief256Pattern31AY_(0) };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BX[256] = { _kBrief256Pattern31BX_(0) };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() float kBrief256Pattern31BY[256] = { _kBrief256Pattern31BY_(0) };
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
// The partten values are mulb cosT and sinT with fxpq(cosT) = fxpq(sinT) = 15 (maxv=0x7fff)
// The equation is simple: [[ COMPV_FXPQ = (fxpq(cosT/sinT) + fxpq(pattern)) ]] => [[ fxpq(pattern) = COMPV_FXPQ - (fxpq(cosT/sinT)) ]]
// With ARM NEON we have COMPV_FXPQ=15 which means we should have fxpq(pattern) = 0
// With X86 we have COMPV_FXPQ=16 which means we should have fxpq(pattern) = 1
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31AXFxp[256] = { _kBrief256Pattern31AX_((COMPV_FXPQ - 15)) };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31AYFxp[256] = { _kBrief256Pattern31AY_((COMPV_FXPQ - 15)) };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31BXFxp[256] = { _kBrief256Pattern31BX_((COMPV_FXPQ - 15)) };
COMPV_EXTERNC COMPV_API const COMPV_ALIGN_DEFAULT() int16_t kBrief256Pattern31BYFxp[256] = { _kBrief256Pattern31BY_((COMPV_FXPQ - 15)) };
#endif

COMPV_NAMESPACE_BEGIN()

// Default values from the detector
extern int COMPV_FEATURE_DETE_ORB_FAST_THRESHOLD_DEFAULT;
extern int COMPV_FEATURE_DETE_ORB_FAST_N_DEFAULT;
extern bool COMPV_FEATURE_DETE_ORB_FAST_NON_MAXIMA_SUPP;
extern int COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS;
extern float COMPV_FEATURE_DETE_ORB_PYRAMID_SF;
extern int COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER;
extern int COMPV_FEATURE_DETE_ORB_PATCH_BITS;
extern COMPV_SCALE_TYPE COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE;

static const int COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE = 7;
static const float COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIGMA = 1.52f;

#define COMPV_FEATURE_DESC_ORB_DESCRIBE_MIN_SAMPLES_PER_THREAD	(500 >> 3) // number of interestPoints

static void Brief256_31_Float32_C(const uint8_t* img_center, compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(x) void* out);
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
static void Brief256_31_Fxp_C(const uint8_t* img_center, compv_scalar_t img_stride, const int16_t* cos1, const int16_t* sin1, COMPV_ALIGNED(x) void* out);
#endif

CompVFeatureDescORB::CompVFeatureDescORB()
    : CompVFeatureDesc(COMPV_ORB_ID)
    , m_nPatchDiameter(COMPV_FEATURE_DETE_ORB_PATCH_DIAMETER)
    , m_nPatchBits(COMPV_FEATURE_DETE_ORB_PATCH_BITS)
    , m_bMediaTypeVideo(false)
    , m_funBrief256_31_Float32(Brief256_31_Float32_C)
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
    , m_funBrief256_31_Fxp(Brief256_31_Fxp_C)
#endif
{

}

CompVFeatureDescORB::~CompVFeatureDescORB()
{
}

// override CompVSettable::set
COMPV_ERROR_CODE CompVFeatureDescORB::set(int id, const void* valuePtr, size_t valueSize)
{
    COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    switch (id) {
    case -1:
    default:
        return CompVSettable::set(id, valuePtr, valueSize);
    }
}

COMPV_ERROR_CODE CompVFeatureDescORB::convlt(CompVPtr<CompVImageScalePyramid * > pPyramid, int level)
{
    // apply gaussianblur filter on the pyramid
    CompVPtr<CompVImage*> imageAtLevelN;
    COMPV_CHECK_CODE_RETURN(pPyramid->getImage(level, &imageAtLevelN));
    // The out is the image itself to avoid allocating temp buffer. This means the images in the pyramid are modified
    // and any subsequent call must take care
    if (m_kern_fxp) {
        // Fixed-point
        COMPV_CHECK_CODE_RETURN(m_convlt->convlt1_fxp((uint8_t*)imageAtLevelN->getDataPtr(), imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), m_kern_fxp->ptr(), m_kern_fxp->ptr(), COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE, (uint8_t*)imageAtLevelN->getDataPtr()));
    }
    else {
        // Floating-point
        COMPV_CHECK_CODE_RETURN(m_convlt->convlt1((uint8_t*)imageAtLevelN->getDataPtr(), imageAtLevelN->getWidth(), imageAtLevelN->getStride(), imageAtLevelN->getHeight(), m_kern_float->ptr(), m_kern_float->ptr(), COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE, (uint8_t*)imageAtLevelN->getDataPtr()));
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVFeatureDescORB::describe(CompVPtr<CompVImageScalePyramid * > pPyramid, const CompVInterestPoint* begin, const CompVInterestPoint* end, uint8_t* desc)
{
    float fx, fy, angleInRad, sf, fcos, fsin;
    int32_t xi, yi;
    const CompVInterestPoint* point;
    CompVPtr<CompVImage*> imageAtLevelN;
    const int nFeaturesBytes = (m_nPatchBits >> 3);
    const int nPatchRadius = (m_nPatchDiameter >> 1);
    const uint8_t* img_center;
    int32_t stride;

    // Describe the points
    for (point = begin; point < end; ++point) {
        // Get image at level N
        COMPV_CHECK_CODE_RETURN(pPyramid->getImage(point->level, &imageAtLevelN));
        stride = imageAtLevelN->getStride();
        // Scale
        sf = pPyramid->getScaleFactor(point->level);
        fx = (point->x * sf);
        fy = (point->y * sf);
        // Convert the angle from degree to radian
        angleInRad = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(point->orient);
        // Get angle's cos and sin
        fcos = ::cos(angleInRad);
        fsin = ::sin(angleInRad);
        // Round the point
        xi = COMPV_MATH_ROUNDFU_2_INT(fx, int32_t);
        yi = COMPV_MATH_ROUNDFU_2_INT(fy, int32_t);
        // Compute description
        {
            // Check if the keypoint is too close to the border
            if ((xi - nPatchRadius) < 0 || (xi + nPatchRadius) >= imageAtLevelN->getWidth() || (yi - nPatchRadius) < 0 || (yi + nPatchRadius) >= imageAtLevelN->getHeight()) {
                // Must never happen....unless you are using keypoints from another implementation (e.g. OpenCV)
                COMPV_DEBUG_ERROR("Keypoint too close to the border");
                memset(desc, 0, nFeaturesBytes);
            }
            else {
                img_center = ((const uint8_t*)imageAtLevelN->getDataPtr()) + ((yi * stride) + xi); // Translate the image to have the keypoint at the center. This is required before applying the rotated patch.
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
                if (CompVEngine::isMathFixedPoint()) {
                    // cosT and sinT are within [-1, 1] which means we can just mulb 0x7fff
                    int16_t cosTQ15 = COMPV_MATH_ROUNDF_2_INT((fcos * 0x7fff), int16_t);
                    int16_t sinTQ15 = COMPV_MATH_ROUNDF_2_INT((fsin * 0x7fff), int16_t);
                    m_funBrief256_31_Fxp(img_center, imageAtLevelN->getStride(), &cosTQ15, &sinTQ15, desc);
                }
                else
#endif
                {
                    m_funBrief256_31_Float32(img_center, stride, &fcos, &fsin, desc);
                }
                desc += nFeaturesBytes;
            }
        }
    }
    return COMPV_ERROR_CODE_S_OK;
}

// override CompVFeatureDesc::process
COMPV_ERROR_CODE CompVFeatureDescORB::process(const CompVPtr<CompVImage*>& image, const CompVPtr<CompVBoxInterestPoint* >& interestPoints, CompVPtr<CompVArray<uint8_t>* >* descriptions)
{
    COMPV_CHECK_EXP_RETURN(*image == NULL || image->getDataPtr() == NULL || image->getPixelFormat() != COMPV_PIXEL_FORMAT_GRAYSCALE || !descriptions || !interestPoints,
                           COMPV_ERROR_CODE_E_INVALID_PARAMETER);

    // For now only Brief256_31 is supported
    COMPV_CHECK_EXP_RETURN(m_nPatchBits != 256 || m_nPatchDiameter != 31, COMPV_ERROR_CODE_E_INVALID_CALL);

    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    CompVPtr<CompVArray<uint8_t>* > _descriptions;
    CompVPtr<CompVImageScalePyramid * > _pyramid;
    CompVPtr<CompVImage*> imageAtLevelN;
    CompVPtr<CompVFeatureDete*> attachedDete = getAttachedDete();
    uint8_t* _descriptionsPtr = NULL;
    static const bool size_of_float_is4 = (sizeof(float) == 4); // ASM and INTRIN code require it
    CompVPtr<CompVThreadDispatcher* >threadDip = CompVEngine::getThreadDispatcher();
    int threadsCount = 1, levelsCount, threadStartIdx = 0;
    CompVPtr<CompVFeatureDescORB* >This = this;
    bool bLevelZeroBlurred = false;

    // return COMPV_ERROR_CODE_S_OK;

    const int nFeatures = (int)interestPoints->size();
    const int nFeaturesBits = m_nPatchBits;
    const int nFeaturesBytes = nFeaturesBits >> 3;
    COMPV_CHECK_CODE_RETURN(err_ = CompVArray<uint8_t>::newObj(&_descriptions, nFeatures, nFeaturesBytes)); // do not align nFeaturesBytes(32) which is already good for AVX, SSE and NEON
    _descriptionsPtr = (uint8_t*)_descriptions->ptr();
    if (nFeatures == 0) {
        return COMPV_ERROR_CODE_S_OK;
    }

    // Get the pyramid from the detector or use or own pyramid
    if ((attachedDete = getAttachedDete())) {
        switch (attachedDete->getId()) {
        case COMPV_ORB_ID: {
            const void* valuePtr = NULL;
            COMPV_CHECK_CODE_RETURN(err_ = attachedDete->get(COMPV_FEATURE_GET_PTR_PYRAMID, valuePtr, sizeof(CompVImageScalePyramid)));
            _pyramid = (CompVImageScalePyramid*)(valuePtr);
            break;
        }
        }
    }
    if (!_pyramid) {
        // This code is called when we fail to get a pyramid from the attached detector or when none is attached.
        // The pyramid should come from the detector. Attach a detector to this descriptor to give it access to the pyramid.
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();
        COMPV_CHECK_CODE_RETURN(err_ = m_pyramid->process(image)); // multithreaded function
        _pyramid = m_pyramid;
    }

    levelsCount = _pyramid->getLevels();

    // Check if we have the same image
    if (m_bMediaTypeVideo && m_image_blurred_prev) {
        COMPV_DEBUG_INFO_CODE_FOR_TESTING();
        // apply gaussianblur filter on the image at level 0
        COMPV_CHECK_CODE_RETURN(err_ = convlt(_pyramid, 0));
        // get blurred image
        COMPV_CHECK_CODE_RETURN(err_ = _pyramid->getImage(0, &imageAtLevelN));
        bLevelZeroBlurred = true; // to avoid apply gaussian blur again
        bool bSameImageAsPrev = false;
        COMPV_CHECK_CODE_RETURN(err_ = m_image_blurred_prev->isEquals(imageAtLevelN, bSameImageAsPrev, 15, m_image_blurred_prev->getHeight() - 15, 15, m_image_blurred_prev->getWidth() - 15));
        // TODO(dmi): Do not use equality but SAD with thredshold: "(abs(img1_ptr[x] - img2_ptr[x]) > t"
    }

    // Compute number of threads
    if (threadDip && threadDip->getThreadsCount() > 1 && !threadDip->isMotherOfTheCurrentThread()) {
        threadsCount = threadDip->getThreadsCount();
        threadStartIdx = threadDip->getThreadIdxForNextToCurrentCore(); // start execution on the next CPU core
    }

    // apply gaussianblur filter on the pyramid
    if (threadsCount > 1) {
        // levelStart is used to make sure we won't schedule more than "threadsCount"
        int levelStart, level, levelMax;
        for (levelStart = bLevelZeroBlurred ? 1 : 0, levelMax = threadsCount; levelStart < levelsCount; levelStart += threadsCount, levelMax += threadsCount) {
            for (level = levelStart; level < levelsCount && level < levelMax; ++level) {
                COMPV_CHECK_CODE_ASSERT(threadDip->execute((uint32_t)(threadStartIdx + level), COMPV_TOKENIDX0, CompVFeatureDescORB::convlt_AsynExec,
                                        COMPV_ASYNCTASK_SET_PARAM_ASISS(*This, *_pyramid, level),
                                        COMPV_ASYNCTASK_SET_PARAM_NULL()));
            }
            for (level = levelStart; level < levelsCount && level < levelMax; ++level) {
                COMPV_CHECK_CODE_ASSERT(threadDip->wait((uint32_t)(threadStartIdx + level), COMPV_TOKENIDX0));
            }
        }
    }
    else {
        for (int level = bLevelZeroBlurred ? 1 : 0; level < levelsCount; ++level) {
            COMPV_CHECK_CODE_RETURN(err_ = convlt(_pyramid, level));
        }
    }

    /* Init "m_funBrief256_31" using current CPU flags */
#if COMPV_FEATURE_DESC_ORB_FXP_DESC
    if (CompVEngine::isMathFixedPoint()) {
        // ARM = FXPQ15, X86 = FXPQ16
        if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
            COMPV_EXEC_IFDEF_INTRIN_X86(m_funBrief256_31_Fxp = Brief256_31_Fxpq16_Intrin_SSE2);
        }
    }
    else
#endif
    {
        if (size_of_float_is4) {
            if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE2)) {
                COMPV_EXEC_IFDEF_INTRIN_X86(m_funBrief256_31_Float32 = Brief256_31_Intrin_SSE2);
            }
            if (compv::CompVCpu::isEnabled(compv::kCpuFlagSSE41)) {
                COMPV_EXEC_IFDEF_ASM_X86(m_funBrief256_31_Float32 = Brief256_31_Asm_X86_SSE41);
                COMPV_EXEC_IFDEF_ASM_X64(m_funBrief256_31_Float32 = Brief256_31_Asm_X64_SSE41);
            }
            if (compv::CompVCpu::isEnabled(compv::kCpuFlagAVX2)) {
                COMPV_EXEC_IFDEF_INTRIN_X86(m_funBrief256_31_Float32 = Brief256_31_Intrin_AVX2);
                COMPV_EXEC_IFDEF_ASM_X86(m_funBrief256_31_Float32 = Brief256_31_Asm_X86_AVX2);
                COMPV_EXEC_IFDEF_ASM_X64(m_funBrief256_31_Float32 = Brief256_31_Asm_X64_AVX2);
                if (compv::CompVCpu::isEnabled(compv::kCpuFlagFMA3)) {
                    COMPV_EXEC_IFDEF_ASM_X86(m_funBrief256_31_Float32 = Brief256_31_Asm_X86_FMA3_AVX2);
                    COMPV_EXEC_IFDEF_ASM_X64(m_funBrief256_31_Float32 = Brief256_31_Asm_X64_FMA3_AVX2);
                }
            }
        }
    }

    // Describe the points
    int32_t threadsCountDescribe = 1;
    if (threadsCount > 1) {
        threadsCountDescribe = (int32_t)(interestPoints->size() / COMPV_FEATURE_DESC_ORB_DESCRIBE_MIN_SAMPLES_PER_THREAD);
        threadsCountDescribe = COMPV_MATH_MIN(threadsCountDescribe, threadsCount);
    }
    if (threadsCountDescribe > 1) {
        const CompVInterestPoint* begin = interestPoints->begin();
        int32_t total = (int32_t)interestPoints->size();
        int32_t count = total / threadsCountDescribe;
        uint8_t* desc = _descriptionsPtr;
        for (int32_t i = 0; count > 0 && i < threadsCountDescribe; ++i) {
            COMPV_CHECK_CODE_RETURN(err_ = threadDip->execute((uint32_t)(threadStartIdx + i), COMPV_TOKENIDX0, describe_AsynExec,
                                           COMPV_ASYNCTASK_SET_PARAM_ASISS(*This, *_pyramid, begin, begin + count, desc),
                                           COMPV_ASYNCTASK_SET_PARAM_NULL()));
            begin += count;
            desc += (count * nFeaturesBytes);
            total -= count;
            if (i == (threadsCountDescribe - 2)) {
                count = (total); // the remaining
            }
        }
        // wait() for threads execution later
    }
    else {
        COMPV_CHECK_CODE_RETURN(err_ = describe(_pyramid, interestPoints->begin(), interestPoints->end(), _descriptionsPtr));
    }

    // TODO(dmi): if MT, call wait() after image cloning
    if (m_bMediaTypeVideo) {
        COMPV_DEBUG_INFO_CODE_FOR_TESTING();
        COMPV_CHECK_CODE_RETURN(err_ = _pyramid->getImage(0, &imageAtLevelN));
        COMPV_CHECK_CODE_RETURN(err_ = imageAtLevelN->clone(&m_image_blurred_prev));
    }

    // Wait for the threads to finish the work
    if (threadsCountDescribe > 1) {
        for (int32_t i = 0; i < threadsCountDescribe; ++i) {
            COMPV_CHECK_CODE_RETURN(err_ = threadDip->wait((uint32_t)(threadStartIdx + i), COMPV_TOKENIDX0));
        }
    }

    *descriptions = _descriptions;

    return err_;
}

COMPV_ERROR_CODE CompVFeatureDescORB::newObj(CompVPtr<CompVFeatureDesc* >* orb)
{
    COMPV_CHECK_EXP_RETURN(orb == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVPtr<CompVImageScalePyramid * > pyramid_;
    CompVPtr<CompVArray<float>* > kern_float_;
    CompVPtr<CompVArray<uint16_t>* > kern_fxp_;
    CompVPtr<CompVConvlt<float>* > convlt_;

    // Create Gauss kernel values
#if COMPV_FEATURE_DESC_ORB_FXP_CONVLT
    if (CompVEngine::isMathFixedPoint()) {
        COMPV_CHECK_CODE_RETURN(CompVGaussKern<float>::buildKern1_fxp(&kern_fxp_, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIGMA));
    }
#endif /* COMPV_FEATURE_DESC_ORB_FXP_CONVLT */
    if (!kern_fxp_) {
        COMPV_CHECK_CODE_RETURN(CompVGaussKern<float>::buildKern1(&kern_float_, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIZE, COMPV_FEATURE_DESC_ORB_GAUSS_KERN_SIGMA));
    }
    // Create convolution context
    COMPV_CHECK_CODE_RETURN(CompVConvlt<float>::newObj(&convlt_));
    // Create the pyramid
    COMPV_CHECK_CODE_RETURN(CompVImageScalePyramid::newObj(COMPV_FEATURE_DETE_ORB_PYRAMID_SF, COMPV_FEATURE_DETE_ORB_PYRAMID_LEVELS, COMPV_FEATURE_DETE_ORB_PYRAMID_SCALE_TYPE, &pyramid_));

    CompVPtr<CompVFeatureDescORB* >_orb = new CompVFeatureDescORB();
    if (!_orb) {
        COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    }
    _orb->m_pyramid = pyramid_;
    _orb->m_kern_float = kern_float_;
    _orb->m_kern_fxp = kern_fxp_; // Fixed-Point is defined only if isMathFixedPoint() is true
    _orb->m_convlt = convlt_;

    *orb = *_orb;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVFeatureDescORB::convlt_AsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
    CompVPtr<CompVFeatureDescORB* >  This = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, CompVFeatureDescORB*);
    CompVPtr<CompVImageScalePyramid* > pyramid = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, CompVImageScalePyramid*);
    int level = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, int);
    return This->convlt(pyramid, level);
}

COMPV_ERROR_CODE CompVFeatureDescORB::describe_AsynExec(const struct compv_asynctoken_param_xs* pc_params)
{
    CompVPtr<CompVFeatureDescORB* >  This = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, CompVFeatureDescORB*);
    CompVPtr<CompVImageScalePyramid* > pyramid = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, CompVImageScalePyramid*);
    const CompVInterestPoint* begin = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, const CompVInterestPoint*);
    const CompVInterestPoint* end = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[3].pcParamPtr, const CompVInterestPoint*);
    uint8_t* desc = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[4].pcParamPtr, uint8_t*);
    return This->describe(pyramid, begin, end, desc);
}

static void Brief256_31_Float32_C(const uint8_t* img_center, compv_scalar_t img_stride, const float* cos1, const float* sin1, COMPV_ALIGNED(x) void* out)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

    static const uint64_t u64_1 = 1;
    uint64_t* _out = (uint64_t*)out;
    int i, j, x, y;
    uint8_t a, b;
    float xf, yf, cosT = *cos1, sinT = *sin1;

    // 256bits = 32Bytes = 4 uint64
    _out[0] = _out[1] = _out[2] = _out[3] = 0;

    // Applying rotation matrix to each (x, y) point in the patch gives us:
    // xr = x*cosT - y*sinT and yr = x*sinT + y*cosT
    for (i = 0, j = 0; i < 256; ++i) {
        xf = (kBrief256Pattern31AX[i] * cosT - kBrief256Pattern31AY[i] * sinT);
        yf = (kBrief256Pattern31AX[i] * sinT + kBrief256Pattern31AY[i] * cosT);
        x = COMPV_MATH_ROUNDF_2_INT(xf, int);
        y = COMPV_MATH_ROUNDF_2_INT(yf, int);
        a = img_center[(y * img_stride) + x];

        xf = (kBrief256Pattern31BX[i] * cosT - kBrief256Pattern31BY[i] * sinT);
        yf = (kBrief256Pattern31BX[i] * sinT + kBrief256Pattern31BY[i] * cosT);
        x = COMPV_MATH_ROUNDF_2_INT(xf, int);
        y = COMPV_MATH_ROUNDF_2_INT(yf, int);
        b = img_center[(y * img_stride) + x];

        _out[0] |= (a < b) ? (u64_1 << j) : 0;
        if (++j == 64) {
            ++_out;
            j = 0;
        }
    }
}

#if COMPV_FEATURE_DESC_ORB_FXP_DESC
static void Brief256_31_Fxp_C(const uint8_t* img_center, compv_scalar_t img_stride, const int16_t* cos1, const int16_t* sin1, COMPV_ALIGNED(x) void* out)
{
    COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED();

    static const uint64_t u64_1 = 1;
    uint64_t* _out = (uint64_t*)out;
    int i, j, x, y;
    uint8_t a, b;
    int cosT = *cos1;
    int sinT = *sin1;

    // 256bits = 32Bytes = 4 uint64
    _out[0] = _out[1] = _out[2] = _out[3] = 0;

    // Applying rotation matrix to each (x, y) point in the patch gives us:
    // xr = x*cosT - y*sinT and yr = x*sinT + y*cosT
    for (i = 0, j = 0; i < 256; ++i) {
        x = (kBrief256Pattern31AXFxp[i] * cosT - kBrief256Pattern31AYFxp[i] * sinT) >> COMPV_FXPQ;
        y = (kBrief256Pattern31AXFxp[i] * sinT + kBrief256Pattern31AYFxp[i] * cosT) >> COMPV_FXPQ;
        a = img_center[(y * img_stride) + x];

        x = (kBrief256Pattern31BXFxp[i] * cosT - kBrief256Pattern31BYFxp[i] * sinT) >> COMPV_FXPQ;
        y = (kBrief256Pattern31BXFxp[i] * sinT + kBrief256Pattern31BYFxp[i] * cosT) >> COMPV_FXPQ;
        b = img_center[(y * img_stride) + x];

        _out[0] |= (a < b) ? (u64_1 << j) : 0;
        if (++j == 64) {
            ++_out;
            j = 0;
        }
    }
}
#endif

COMPV_NAMESPACE_END()
