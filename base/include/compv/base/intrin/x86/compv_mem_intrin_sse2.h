/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MEM_INTRIN_SSE2_H_)
#define _COMPV_BASE_MEM_INTRIN_SSE2_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVMemCopy_Intrin_Aligned_SSE2(COMPV_ALIGNED(SSE) void* dataDstPtr, COMPV_ALIGNED(SSE) const void* dataSrcPtr, compv_uscalar_t size);
void CompVMemCopyNTA_Intrin_Aligned_SSE2(COMPV_ALIGNED(SSE) void* dataDstPtr, COMPV_ALIGNED(SSE) const void* dataSrcPtr, compv_uscalar_t size);
void CompVMemZeroNTA_Intrin_Aligned_SSE2(COMPV_ALIGNED(SSE) void* dstPtr, compv_uscalar_t size);
void CompVMemPack2_Intrin_SSE2(
	COMPV_ALIGNED(SSE) compv_uint8x2_t* dstPtr, COMPV_ALIGNED(SSE) const uint8_t* srcPt0, COMPV_ALIGNED(SSE) const uint8_t* srcPt1,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MEM_INTRIN_SSE2_H_ */
