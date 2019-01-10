/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_CCL_LSL_INTRIN_SSSE3_H_)
#define _COMPV_CORE_CCL_LSL_INTRIN_SSSE3_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#if COMPV_ARCH_X86 && COMPV_INTRINSIC

COMPV_NAMESPACE_BEGIN()

void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Intrin_SSSE3(
	COMPV_ALIGNED(SSE) const uint8_t* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi,
	int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
	const compv_uscalar_t width, const compv_uscalar_t height
);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_CORE_CCL_LSL_INTRIN_SSSE3_H_ */
