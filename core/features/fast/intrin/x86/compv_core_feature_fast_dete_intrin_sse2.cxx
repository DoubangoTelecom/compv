/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/fast/intrin/x86/compv_core_feature_fast_dete_intrin_sse2.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// No need to check for 'width'. The caller ('CompVFastProcessRange' function) already checked and prepared it for SSE.
void CompVFastDataRow16_Intrin_SSE2(const uint8_t* IP, COMPV_ALIGNED(SSE) compv_scalar_t width, COMPV_ALIGNED(DEFAULT) const compv_scalar_t *pixels16, compv_scalar_t N, compv_scalar_t threshold, COMPV_ALIGNED(DEFAULT) uint8_t* strengths)
{
	COMPV_DEBUG_INFO_CHECK_SSE2();
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
