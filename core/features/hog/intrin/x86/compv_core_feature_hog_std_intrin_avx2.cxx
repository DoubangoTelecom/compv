/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hog/intrin/x86/compv_core_feature_hog_std_intrin_avx2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

#if defined(__INTEL_COMPILER)
#	pragma intel optimization_parameter target_arch=avx2
#endif
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
	COMPV_ALIGNED(AVX) const compv_uscalar_t dirStride
)
{
	COMPV_DEBUG_INFO_CHECK_AVX2();
#if 0 // TODO(dmi): Add ASM code
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM code (ASM code should use )");
#endif
	_mm256_zeroupper();
	const __m256 vecMask_sign = _mm256_castsi256_ps(_mm256_set1_epi32(0x7fffffff)); // used to compute _mm256_abs_ps, not needed for ARM NEON
	const __m256 vecthetaMax = _mm256_set1_ps(*thetaMax1);
	const __m256 vecScaleBinWidth = _mm256_set1_ps(*scaleBinWidth1);
	const __m256 vecZero = _mm256_setzero_ps();
	const __m256 vecHalf = _mm256_set1_ps(0.5f);
	const __m256i vecOne_plus = _mm256_set1_epi32(1);
	const __m256i vecOne_minus = _mm256_set1_epi32(-1);
	const __m256i vecBinIdxMax = _mm256_set1_epi32(*binIdxMax1);
	const __m256 vecBinWidth = _mm256_cvtepi32_ps(_mm256_set1_epi32(*binWidth1));
	COMPV_ALIGN_AVX() int32_t indices[16];
	COMPV_ALIGN_AVX() compv_float32_t values[16];
	for (compv_uscalar_t j = 0; j < cellHeight; ++j) {
		for (compv_uscalar_t i = 0; i < cellWidth; i += 8) {
			__m256 vecTheta = _mm256_load_ps(&dirPtr[i]);
			const __m256 vecMask = _mm256_cmp_ps(vecTheta, vecthetaMax, _CMP_GT_OQ);
			vecTheta = _mm256_sub_ps(vecTheta, _mm256_and_ps(vecthetaMax, vecMask));
			const __m256i vecBinIdx = _mm256_cvttps_epi32(_mm256_sub_ps(_mm256_mul_ps(vecScaleBinWidth, vecTheta), vecHalf));
			const __m256 vecDiff = _mm256_sub_ps(_mm256_mul_ps(_mm256_sub_ps(vecTheta, _mm256_mul_ps(_mm256_cvtepi32_ps(vecBinIdx), vecBinWidth)), vecScaleBinWidth), vecHalf); // '_mm256_mullo_epi32' is SSE4.1
			__m256 vecMagPtr = _mm256_load_ps(&magPtr[i]);
			const __m256 vecAVV = _mm256_and_ps(vecMask_sign, _mm256_mul_ps(vecMagPtr, vecDiff));
			__m256i vecBinIdxNext = _mm256_castps_si256(_mm256_cmp_ps(vecDiff, vecZero, _CMP_GE_OQ));
			vecBinIdxNext = _mm256_or_si256(_mm256_and_si256(vecBinIdxNext, vecOne_plus), _mm256_andnot_si256(vecBinIdxNext, vecOne_minus));
			vecBinIdxNext = _mm256_add_epi32(vecBinIdxNext, vecBinIdx);
			const __m256i vecMaski0 = _mm256_cmpgt_epi32(_mm256_castps_si256(vecZero), vecBinIdxNext);
			const __m256i vecMaski1 = _mm256_cmpgt_epi32(vecBinIdxNext, vecBinIdxMax);
			vecBinIdxNext = _mm256_or_si256(
				_mm256_and_si256(vecMaski0, vecBinIdxMax),
				_mm256_andnot_si256(vecMaski0, _mm256_andnot_si256(vecMaski1, vecBinIdxNext))
			);
			// TODO(dmi): AVX2 -> use gather[binIdxNext] and gather[binIdx]:
			//		mapHistPtr[vecBinIdxNext.m256i_i32[k]] += vecAVV.m256_f32[k];
			//		mapHistPtr[vecBinIdx.m256i_i32[k]] += vecMagPtr.m256_f32[k];
			_mm256_store_si256(reinterpret_cast<__m256i*>(&indices[0]), vecBinIdxNext);
			_mm256_store_si256(reinterpret_cast<__m256i*>(&indices[8]), vecBinIdx);
			_mm256_store_ps(&values[0], vecAVV);
			_mm256_store_ps(&values[8], _mm256_sub_ps(vecMagPtr, vecAVV));
			// Addition order is important if we want to have the same MD5 as serial code
			mapHistPtr[indices[0]] += values[0];
			mapHistPtr[indices[8]] += values[8];
			mapHistPtr[indices[1]] += values[1];
			mapHistPtr[indices[9]] += values[9];
			mapHistPtr[indices[2]] += values[2];
			mapHistPtr[indices[10]] += values[10];
			mapHistPtr[indices[3]] += values[3];
			mapHistPtr[indices[11]] += values[11];
			mapHistPtr[indices[4]] += values[4];
			mapHistPtr[indices[12]] += values[12];
			mapHistPtr[indices[5]] += values[5];
			mapHistPtr[indices[13]] += values[13];
			mapHistPtr[indices[6]] += values[6];
			mapHistPtr[indices[14]] += values[14];
			mapHistPtr[indices[7]] += values[7];
			mapHistPtr[indices[15]] += values[15];
		}
		magPtr += magStride;
		dirPtr += dirStride;
	}
	_mm256_zeroupper();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
