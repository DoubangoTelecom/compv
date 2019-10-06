/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
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

// TODO(dmi) ASM code slightly faster on both ARM32 and ARM64 (tested on MediaPad2)
void CompVMemUnpack3_Intrin_NEON(
	COMPV_ALIGNED(NEON) uint8_t* dstPt0, COMPV_ALIGNED(NEON) uint8_t* dstPt1, COMPV_ALIGNED(NEON) uint8_t* dstPt2,
	COMPV_ALIGNED(NEON) const compv_uint8x3_t* srcPtr,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	COMPV_DEBUG_INFO_CHECK_NEON();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) {
			const uint8x16x3_t& v3 = vld3q_u8(reinterpret_cast<const uint8_t*>(&srcPtr[i]));
			vst1q_u8(&dstPt0[i], v3.val[0]);
			vst1q_u8(&dstPt1[i], v3.val[1]);
			vst1q_u8(&dstPt2[i], v3.val[2]);
		}
		dstPt0 += stride;
		dstPt1 += stride;
		dstPt2 += stride;
		srcPtr += stride;
	}
}

// TODO(dmi) ASM code slightly faster on both ARM32 and ARM64 (tested on MediaPad2)
// TODO(dmi): On MediaPad2 ARM64, 1282x720, 1 thread, "Unpack+Pack" -> Intrin: 504ms, ASM: 429ms
// TODO(dmi): On MediaPad2 ARM32, 1282x720, 1 thread, "Unpack+Pack" -> Intrin: 502ms, ASM: 443ms
void CompVMemPack3_Intrin_NEON(
	COMPV_ALIGNED(NEON) compv_uint8x3_t* dstPtr,
	COMPV_ALIGNED(NEON) const uint8_t* srcPt0, COMPV_ALIGNED(NEON) const uint8_t* srcPt1, COMPV_ALIGNED(NEON) const uint8_t* srcPt2,
	compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("ASM code faster");
	COMPV_DEBUG_INFO_CHECK_NEON();
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; i += 16) { // strided/SSE-aligned -> can write beyond width
			const uint8x16x3_t vecSrc = { { vld1q_u8(&srcPt0[i]), vld1q_u8(&srcPt1[i]), vld1q_u8(&srcPt2[i]) } };
			vst3q_u8(reinterpret_cast<uint8_t*>(&dstPtr[i]), vecSrc);
		}
		dstPtr += stride;
		srcPt0 += stride;
		srcPt1 += stride;
		srcPt2 += stride;
	}
}

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 && COMPV_INTRINSIC */
