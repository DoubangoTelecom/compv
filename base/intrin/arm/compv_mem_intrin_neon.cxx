/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/intrin/arm/compv_mem_intrin_neon.h"

#if COMPV_ARCH_ARM && COMPV_INTRINSIC
#include "compv/base/compv_simd_globals.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()


void CompVMemCopy_Intrin_NEON(COMPV_ALIGNED(NEON) void* dataDstPtr, COMPV_ALIGNED(NEON) const void* dataSrcPtr, compv_uscalar_t size)
{
    COMPV_DEBUG_INFO_CHECK_NEON();
	const compv_uscalar_t sizeNEON = size & -64;
	compv_uscalar_t i;
	uint8_t* dataDstPtrAligned = reinterpret_cast<uint8_t*>(__compv_builtin_assume_aligned(dataDstPtr, 16));
	const uint8_t* dataSrcPtrAligned = reinterpret_cast<const uint8_t*>(__compv_builtin_assume_aligned(dataSrcPtr, 16));
	uint8x16_t vec0, vec1, vec2, vec3;

	__compv_builtin_prefetch_read(&dataSrcPtrAligned[COMPV_CACHE1_LINE_SIZE * 0]);
	__compv_builtin_prefetch_read(&dataSrcPtrAligned[COMPV_CACHE1_LINE_SIZE * 1]);
	__compv_builtin_prefetch_read(&dataSrcPtrAligned[COMPV_CACHE1_LINE_SIZE * 2]);
	__compv_builtin_prefetch_write(&dataDstPtrAligned[COMPV_CACHE1_LINE_SIZE * 0]);
	__compv_builtin_prefetch_write(&dataDstPtrAligned[COMPV_CACHE1_LINE_SIZE * 1]);
	__compv_builtin_prefetch_write(&dataDstPtrAligned[COMPV_CACHE1_LINE_SIZE * 2]);

	for (i = 0; i < sizeNEON; i += 64) {
		__compv_builtin_prefetch_read(&dataSrcPtrAligned[COMPV_CACHE1_LINE_SIZE * 3]);
		__compv_builtin_prefetch_write(&dataDstPtrAligned[COMPV_CACHE1_LINE_SIZE *3]);
		vec0 = vld1q_u8(&dataSrcPtrAligned[i]);
		vec1 = vld1q_u8(&dataSrcPtrAligned[i + 16]);
		vec2 = vld1q_u8(&dataSrcPtrAligned[i + 32]);
		vec3 = vld1q_u8(&dataSrcPtrAligned[i + 48]);
		vst1q_u8(&dataDstPtrAligned[i], vec0);
		vst1q_u8(&dataDstPtrAligned[i + 16], vec1);
		vst1q_u8(&dataDstPtrAligned[i + 32], vec2);
		vst1q_u8(&dataDstPtrAligned[i + 48], vec3);
	}
	if (size > sizeNEON) {
		memcpy(&dataDstPtrAligned[sizeNEON], &dataSrcPtrAligned[sizeNEON], (size - sizeNEON));
	}
}

void CompVMemZero_Intrin_NEON(COMPV_ALIGNED(NEON) void* dstPtr, compv_uscalar_t size)
{
	COMPV_DEBUG_INFO_CHECK_NEON();
	static uint8x16_t vecZero = vdupq_n_u8(0);
	const compv_uscalar_t sizeNEON = size & -64;
	compv_uscalar_t i;
	uint8_t*dstPtrAligned = reinterpret_cast<uint8_t*>(__compv_builtin_assume_aligned(dstPtr, 16));
	for (i = 0; i < sizeNEON; i += 64) {
		vst1q_u8(&dstPtrAligned[i], vecZero);
		vst1q_u8(&dstPtrAligned[i + 16], vecZero);
		vst1q_u8(&dstPtrAligned[i + 32], vecZero);
		vst1q_u8(&dstPtrAligned[i + 48], vecZero);
	}
	if (size > sizeNEON) {
		memset(&dstPtrAligned[sizeNEON], 0, (size - sizeNEON));
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
