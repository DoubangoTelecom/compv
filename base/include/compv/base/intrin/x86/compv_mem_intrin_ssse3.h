/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MEM_INTRIN_SSSE3_H_)
#define _COMPV_BASE_MEM_INTRIN_SSSE3_H_

#include "compv/base/compv_config.h"
#if COMPV_ARCH_X86 && COMPV_INTRINSIC
#include "compv/base/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void CompVMemUnpack4_Intrin_SSSE3(
	COMPV_ALIGNED(SSE) uint8_t* dstPt0, COMPV_ALIGNED(SSE) uint8_t* dstPt1, COMPV_ALIGNED(SSE) uint8_t* dstPt2, COMPV_ALIGNED(SSE) uint8_t* dstPt3,
	COMPV_ALIGNED(SSE) const compv_uint8x4_t* srcPtr,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

void CompVMemUnpack3_Intrin_SSSE3(
	COMPV_ALIGNED(SSE) uint8_t* dstPt0, COMPV_ALIGNED(SSE) uint8_t* dstPt1, COMPV_ALIGNED(SSE) uint8_t* dstPt2, 
	COMPV_ALIGNED(SSE) const compv_uint8x3_t* srcPtr, 
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

void CompVMemUnpack3_SrcPtrNotAligned_Intrin_SSSE3(
	COMPV_ALIGNED(SSE) uint8_t* dstPt0, COMPV_ALIGNED(SSE) uint8_t* dstPt1, COMPV_ALIGNED(SSE) uint8_t* dstPt2,
	const compv_uint8x3_t* srcPtr,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

void CompVMemUnpack2_Intrin_SSSE3(
	COMPV_ALIGNED(SSE) uint8_t* dstPt0, COMPV_ALIGNED(SSE) uint8_t* dstPt1, COMPV_ALIGNED(SSE) const compv_uint8x2_t* srcPtr,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

void CompVMemPack3_Intrin_SSSE3(
	COMPV_ALIGNED(SSE) compv_uint8x3_t* dstPtr,
	COMPV_ALIGNED(SSE) const uint8_t* srcPt0, COMPV_ALIGNED(SSE) const uint8_t* srcPt1, COMPV_ALIGNED(SSE) const uint8_t* srcPt2,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */

#endif /* _COMPV_BASE_MEM_INTRIN_SSSE3_H_ */
