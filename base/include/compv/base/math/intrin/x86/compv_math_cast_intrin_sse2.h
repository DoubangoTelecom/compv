/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATH_CAST_INTRIN_SSE2_H_)
#define _COMPV_BASE_MATH_CAST_INTRIN_SSE2_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVMathCastProcess_static_64f32f_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const compv_float64_t* src,
	COMPV_ALIGNED(SSE) compv_float32_t* dst,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(SSE) const compv_uscalar_t stride
);

void CompVMathCastProcess_static_8u32f_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const uint8_t* src,
	COMPV_ALIGNED(SSE) compv_float32_t* dst,
	const compv_uscalar_t width,
	const compv_uscalar_t height,
	COMPV_ALIGNED(SSE) const compv_uscalar_t stride
);

void CompVMathCastProcess_static_pixel8_32f_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const compv_float32_t* src,
	COMPV_ALIGNED(SSE) uint8_t* dst,
	compv_uscalar_t width,
	compv_uscalar_t height,
	COMPV_ALIGNED(SSE) compv_uscalar_t stride
);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MATH_CAST_INTRIN_SSE2_H_ */
