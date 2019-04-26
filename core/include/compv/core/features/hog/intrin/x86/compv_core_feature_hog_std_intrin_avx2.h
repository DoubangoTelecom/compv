/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURE_HOG_STD_INTRIN_AVX2_H_)
#define _COMPV_CORE_FEATURE_HOG_STD_INTRIN_AVX2_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVHogStdBuildMapHistForSingleCellBilinear_32f32s_Intrin_AVX2(
	COMPV_ALIGNED(AVX) const compv_float32_t* magPtr,
	COMPV_ALIGNED(AVX) const compv_float32_t* dirPtr,
	COMPV_ALIGNED(AVX) compv_float32_t* mapHistPtr,
	const compv_float32_t* thetaMax1,
	const compv_float32_t* scaleBinWidth1,
	const int32_t* binWidth1,
	const int32_t* binIdxMax1,
	COMPV_ALIGNED(AVX) const compv_uscalar_t cellWidth,
	const compv_uscalar_t cellHeight,
	COMPV_ALIGNED(AVX) const compv_uscalar_t magStride,
	COMPV_ALIGNED(AVX) const compv_uscalar_t dirStride,
	const void* bilinearFastLUT COMPV_DEFAULT(nullptr)
);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_CORE_FEATURE_HOG_STD_INTRIN_AVX2_H_ */
