/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/intrin/x86/compv_mem_intrin_ssse3.h"

#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/intrin/x86/compv_intrin_sse.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

void CompVMemCopy3_Intrin_SSSE3(
	COMPV_ALIGNED(SSE) uint8_t* dstPt0, COMPV_ALIGNED(SSE) uint8_t* dstPt1, COMPV_ALIGNED(SSE) uint8_t* dstPt2,
	COMPV_ALIGNED(SSE) const compv_uint8x3_t* srcPtr,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride)
{
    COMPV_DEBUG_INFO_CHECK_SSSE3();
	__m128i vecLane0, vecLane1, vecLane2, vectmp0, vectmp1;
    
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			COMPV_VLD3_U8_SSSE3(&srcPtr[i], vecLane0, vecLane1, vecLane2, vectmp0, vectmp1);
			_mm_store_si128(reinterpret_cast<__m128i*>(&dstPt0[i]), vecLane0);
			_mm_store_si128(reinterpret_cast<__m128i*>(&dstPt1[i]), vecLane1);
			_mm_store_si128(reinterpret_cast<__m128i*>(&dstPt2[i]), vecLane2);
		}
		dstPt0 += stride;
		dstPt1 += stride;
		dstPt2 += stride;
		srcPtr += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
