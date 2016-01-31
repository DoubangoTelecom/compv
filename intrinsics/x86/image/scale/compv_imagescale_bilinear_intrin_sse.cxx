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
#include "compv/intrinsics/x86/image/scale/compv_imagescale_bilinear_intrin_sse.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/compv_simd_globals.h"
#include "compv/compv_math.h"

COMPV_NAMESPACE_BEGIN()

// inPtr doesn't need to be aligned
// outPtr must be aligned
// outStride must be aligned
// image width and height must be <= SHRT_MAX
void scaleBilinearKernel11_Aligned_SSSE3(const uint8_t* inPtr, COMPV_ALIGNED(SSE) uint8_t* outPtr, compv_scalar_t inHeight, compv_scalar_t inWidth, compv_scalar_t inStride, compv_scalar_t outHeight, compv_scalar_t outWidth, compv_scalar_t outStride, compv_scalar_t sf_x, compv_scalar_t sf_y)
{
    COMPV_ASSERT(false);
}

COMPV_NAMESPACE_END()

#endif /* defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC) */
