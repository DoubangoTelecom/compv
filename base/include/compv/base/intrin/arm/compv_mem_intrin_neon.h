/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MEM_INTRIN_NEON_H_)
#define _COMPV_BASE_MEM_INTRIN_NEON_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVMemCopy_Intrin_NEON(COMPV_ALIGNED(NEON) void* dataDstPtr, COMPV_ALIGNED(NEON) const void* dataSrcPtr, compv_uscalar_t size);
void CompVMemZero_Intrin_NEON(COMPV_ALIGNED(NEON) void* dstPtr, compv_uscalar_t size);
void CompVMemCopy3_Intrin_NEON(
	COMPV_ALIGNED(NEON) uint8_t* dstPt0, COMPV_ALIGNED(NEON) uint8_t* dstPt1, COMPV_ALIGNED(NEON) uint8_t* dstPt2,
	COMPV_ALIGNED(NEON) const compv_uint8x3_t* srcPtr,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_ARM && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MEM_INTRIN_NEON_H_ */
