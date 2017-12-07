/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURE_HOUGHKHT_INTRIN_NEON_H_)
#define _COMPV_CORE_FEATURE_HOUGHKHT_INTRIN_NEON_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#include "compv/core/features/hough/compv_core_feature_houghkht.h" /* CompVHoughKhtVotes */

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_ARM && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVHoughKhtPeaks_Section3_4_VotesCount_4mpd_Intrin_NEON(const int32_t *pcount, const size_t pcount_stride, const size_t theta_index, const size_t rho_count, const int32_t nThreshold, CompVHoughKhtVotes& votes);

#if COMPV_ARCH_ARM64

void CompVHoughKhtKernelHeight_2mpq_Intrin_NEON64(
	COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_r0, COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_0, COMPV_ALIGNED(NEON) const compv_float64_t* M_Eq14_2, COMPV_ALIGNED(NEON) const compv_float64_t* n_scale,
	COMPV_ALIGNED(NEON) compv_float64_t* sigma_rho_square, COMPV_ALIGNED(NEON) compv_float64_t* sigma_rho_times_theta, COMPV_ALIGNED(NEON) compv_float64_t* m2, COMPV_ALIGNED(NEON) compv_float64_t* sigma_theta_square,
	COMPV_ALIGNED(NEON) compv_float64_t* height, COMPV_ALIGNED(NEON) compv_float64_t* heightMax1, COMPV_ALIGNED(NEON) compv_uscalar_t count
);
#endif

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_NEON && COMPV_INTRINSIC */

#endif /* _COMPV_CORE_FEATURE_HOUGHKHT_INTRIN_NEON_H_ */
