/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_distance.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/compv_bits.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/math/intrin/x86/compv_math_distance_intrin_sse42.h"

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM
#	if COMPV_ARCH_X86
	COMPV_EXTERNC void CompVMathDistanceHamming_Asm_X86_POPCNT_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr);
#	endif /* COMPV_ARCH_X86 */
#endif /* COMPV_ASM */



static void CompVHammingDistance_C(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr);
static void CompVHammingDistance_POPCNT_C(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr);

/*
dataPtr: The pointer to the data for which we want to compute the hamming distance. Hamming distance will be compute for each width-bytes.
width: The number of bytes to use to compute each hamming distance value.
stride: The stride value.
height: The number of rows.
patch1xnPtr: The pointer to the patch. The patch is used as sliding window over the rows to compute the hamming distances. It must be a (1 x width) array.
distPtr: The results for each row. It must be a (1 x height) array.
Algorithm:
for (row = 0; row < height; ++row)
distPtr[row] = hamming(dataPtr[row], key1xnPtr);
*/
COMPV_ERROR_CODE CompVMathDistance::hamming(const uint8_t* dataPtr, size_t width, size_t height, size_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr)
{
	COMPV_CHECK_EXP_RETURN(!dataPtr || !width || width > stride || !height || !patch1xnPtr || !distPtr, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	void(*HammingDistance)(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr) = CompVHammingDistance_C;
	void(*Hamming256Distance)(const uint8_t* dataPtr, compv_uscalar_t height, const uint8_t* patch1xnPtr, int32_t* distPtr) = NULL;

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagPOPCNT)) {
		HammingDistance = CompVHammingDistance_POPCNT_C;
		if (width > 15 && CompVCpu::isEnabled(kCpuFlagSSE42) && COMPV_IS_ALIGNED_SSE(dataPtr) && COMPV_IS_ALIGNED_SSE(patch1xnPtr) && COMPV_IS_ALIGNED_SSE(stride)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(HammingDistance = CompVMathDistanceHamming_Intrin_POPCNT_SSE42);
			COMPV_EXEC_IFDEF_ASM_X86(HammingDistance = CompVMathDistanceHamming_Asm_X86_POPCNT_SSE42);
		}
	}
#endif
#if 0
	if (CompVCpu::isEnabled(kCpuFlagPOPCNT)) {
		HammingDistance = CompVHammingDistance_POPCNT_C;
		if (CompVCpu::isEnabled(kCpuFlagSSE42) && COMPV_IS_ALIGNED_SSE(dataPtr) && COMPV_IS_ALIGNED_SSE(patch1xnPtr)) {
			COMPV_EXEC_IFDEF_INTRIN_X86((HammingDistance = CompVHammingDistance_Intrin_POPCNT_SSE42));
			COMPV_EXEC_IFDEF_ASM_X86((HammingDistance = CompVHammingDistance_Asm_POPCNT_X86_SSE42));
			COMPV_EXEC_IFDEF_ASM_X64((HammingDistance = CompVHammingDistance_Asm_POPCNT_X64_SSE42));
			if (stride == 32 && width == 32) {
				// Hamming distance is frequently used with Brief256, "Hamming256Distance" is a very fast function
				COMPV_EXEC_IFDEF_INTRIN_X86((Hamming256Distance = HammingDistance256_Intrin_POPCNT_SSE42));
				COMPV_EXEC_IFDEF_ASM_X86((Hamming256Distance = CompVHammingDistance256_Asm_POPCNT_X86_SSE42));
				COMPV_EXEC_IFDEF_ASM_X64((Hamming256Distance = CompVHammingDistance256_Asm_POPCNT_X64_SSE42));
			}
		}
		// There is no math operations (except the xor) in hamming function but a lot of loads and this is why SSE42 version is
		// slightly faster than AVX
#if 0
		if (CompVCpu::isEnabled(kCpuFlagAVX2) && COMPV_IS_ALIGNED_AVX2(dataPtr) && COMPV_IS_ALIGNED_AVX2(patch1xnPtr)) {
			COMPV_EXEC_IFDEF_ASM_X86((HammingDistance = CompVHammingDistance_Asm_POPCNT_X86_AVX2));
			COMPV_EXEC_IFDEF_ASM_X64((HammingDistance = CompVHammingDistance_Asm_POPCNT_X64_AVX2));
		}
#endif
	}
#endif

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("Fast hamming, do not require stride to be 32");

	if (Hamming256Distance) { // TODO(dmi): Why it's named hamming256?
		Hamming256Distance(dataPtr, height, patch1xnPtr, distPtr);
	}
	else {
		if (stride == 32 && width == 32) {
			COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found for fast hamming distance (32 x 32)");
		}
		HammingDistance(dataPtr, width, height, stride, patch1xnPtr, distPtr);
	}

	return COMPV_ERROR_CODE_S_OK;
}

// Private function, up to the caller to check input parameters
static void CompVHammingDistance_C(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	compv_uscalar_t i, j, cnt;
	uint8_t pop;

	for (j = 0; j < height; ++j) {
		cnt = 0;
		for (i = 0; i < width; ++i) {
			pop = dataPtr[i] ^ patch1xnPtr[i];
			cnt += kPopcnt256[pop];
		}
		dataPtr += stride;
		distPtr[j] = static_cast<int32_t>(cnt);
	}
}

// Private function, up to the caller to check input parameters
static void CompVHammingDistance_POPCNT_C(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr)
{
	// POPCNT is available with SSE4.2, there is no reason to fallback to this function. Use ASM_SSE42 instead.
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	compv_uscalar_t i, j;
	uint64_t cnt;
	uint64_t pop;

	for (j = 0; j < height; ++j) {
		cnt = 0;
		i = 0;
#if COMPV_ARCH_X64
		for (; i <= width - 8; i += 8) {
			pop = *((uint64_t*)&dataPtr[i]) ^ *((uint64_t*)&patch1xnPtr[i]);
			cnt += compv_popcnt64(pop);
		}
#endif
		for (; i <= width - 4; i += 4) {
			pop = *((uint32_t*)&dataPtr[i]) ^ *((uint32_t*)&patch1xnPtr[i]);
			cnt += compv_popcnt32((uint32_t)pop);
		}
		if (i <= width - 2) {
			pop = *((uint16_t*)&dataPtr[i]) ^ *((uint16_t*)&patch1xnPtr[i]);
			cnt += compv_popcnt16((uint16_t)pop);
			i += 2;
		}
		if (i <= width - 1) {
			pop = *((uint8_t*)&dataPtr[i]) ^ *((uint8_t*)&patch1xnPtr[i]);
			cnt += compv_popcnt16((uint16_t)pop);
			++i;
		}
		dataPtr += stride;
		distPtr[j] = (int32_t)(cnt);
	}
}

COMPV_NAMESPACE_END()