/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hog/intrin/x86/compv_core_feature_hog_std_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVHogStdBuildMapHistForSingleCellBilinear_32f32s_Intrin_SSE2(
	COMPV_ALIGNED(SSE) const compv_float32_t* magPtr,
	COMPV_ALIGNED(SSE) const compv_float32_t* dirPtr,
	COMPV_ALIGNED(SSE) compv_float32_t* mapHistPtr,
	const compv_float32_t* thetaMax1,
	const compv_float32_t* scaleBinWidth1,
	const int32_t* binWidth1,
	const int32_t* binIdxMax1,
	COMPV_ALIGNED(SSE) const compv_uscalar_t cellWidth, // should be small (<= 16)
	const compv_uscalar_t cellHeight,
	COMPV_ALIGNED(SSE) const compv_uscalar_t magStride,
	COMPV_ALIGNED(SSE) const compv_uscalar_t dirStride,
	const void* bilinearFastLUT COMPV_DEFAULT(nullptr)
)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
#if 0 // TODO(dmi): not urgent
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code");
#endif
	const __m128 vecMask_sign = _mm_castsi128_ps(_mm_set1_epi32(0x7fffffff)); // used to compute _mm_abs_ps, not needed for ARM NEON
	const __m128 vecthetaMax = _mm_set1_ps(*thetaMax1);
	const __m128 vecScaleBinWidth = _mm_set1_ps(*scaleBinWidth1);
	const __m128 vecZero = _mm_setzero_ps();
	const __m128 vecHalf = _mm_set1_ps(0.5f);
	const __m128i vecOne_plus = _mm_set1_epi32(1);
	const __m128i vecOne_minus = _mm_set1_epi32(-1);
	const __m128i vecBinIdxMax = _mm_set1_epi32(*binIdxMax1);
	const __m128 vecBinWidth = _mm_cvtepi32_ps(_mm_set1_epi32(*binWidth1));
	COMPV_ALIGN_SSE() int32_t indices[8];
	COMPV_ALIGN_SSE() compv_float32_t values[8];
	for (compv_uscalar_t j = 0; j < cellHeight; ++j) {
		for (compv_uscalar_t i = 0; i < cellWidth; i += 4) {
			__m128 vecTheta = _mm_load_ps(&dirPtr[i]);
			const __m128 vecMask = _mm_cmpgt_ps(vecTheta, vecthetaMax);
			vecTheta = _mm_sub_ps(vecTheta, _mm_and_ps(vecthetaMax, vecMask));
			const __m128i vecBinIdx = _mm_cvttps_epi32(_mm_sub_ps(_mm_mul_ps(vecScaleBinWidth, vecTheta), vecHalf));
			const __m128 vecDiff = _mm_sub_ps(_mm_mul_ps(_mm_sub_ps(vecTheta, _mm_mul_ps(_mm_cvtepi32_ps(vecBinIdx), vecBinWidth)), vecScaleBinWidth), vecHalf); // '_mm_mullo_epi32' is SSE4.1
			__m128 vecMagPtr = _mm_load_ps(&magPtr[i]);
			const __m128 vecAVV = _mm_and_ps(vecMask_sign, _mm_mul_ps(vecMagPtr, vecDiff));
			__m128i vecBinIdxNext = _mm_castps_si128(_mm_cmpge_ps(vecDiff, vecZero));
			vecBinIdxNext = _mm_or_si128(_mm_and_si128(vecBinIdxNext, vecOne_plus), _mm_andnot_si128(vecBinIdxNext, vecOne_minus));
			vecBinIdxNext = _mm_add_epi32(vecBinIdxNext, vecBinIdx);
			const __m128i vecMaski0 = _mm_cmplt_epi32(vecBinIdxNext, _mm_castps_si128(vecZero));
			const __m128i vecMaski1 = _mm_cmpgt_epi32(vecBinIdxNext, vecBinIdxMax);
			vecBinIdxNext = _mm_or_si128(
				_mm_and_si128(vecMaski0, vecBinIdxMax),
				_mm_andnot_si128(vecMaski0, _mm_andnot_si128(vecMaski1, vecBinIdxNext))
			);
			vecMagPtr = _mm_sub_ps(vecMagPtr, vecAVV);
			// TODO(dmi): AVX2 -> use gather[binIdxNext] and gather[binIdx]:
			//		mapHistPtr[vecBinIdxNext.m128i_i32[k]] += vecAVV.m128_f32[k];
			//		mapHistPtr[vecBinIdx.m128i_i32[k]] += vecMagPtr.m128_f32[k];
			_mm_store_si128(reinterpret_cast<__m128i*>(&indices[0]), vecBinIdxNext);
			_mm_store_si128(reinterpret_cast<__m128i*>(&indices[4]), vecBinIdx);
			_mm_store_ps(&values[0], vecAVV);
			_mm_store_ps(&values[4], vecMagPtr);
			// Addition order is important if we want to have the same MD5 as serial code
			mapHistPtr[indices[0]] += values[0];
			mapHistPtr[indices[4]] += values[4];
			mapHistPtr[indices[1]] += values[1];
			mapHistPtr[indices[5]] += values[5];
			mapHistPtr[indices[2]] += values[2];
			mapHistPtr[indices[6]] += values[6];
			mapHistPtr[indices[3]] += values[3];
			mapHistPtr[indices[7]] += values[7];
		}
		magPtr += magStride;
		dirPtr += dirStride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
