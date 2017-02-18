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
#include "compv/base/parallel/compv_parallel.h"

#include "compv/base/math/intrin/x86/compv_math_distance_intrin_sse42.h"
#include "compv/base/math/intrin/x86/compv_math_distance_intrin_avx2.h"
#include "compv/base/math/intrin/arm/compv_math_distance_intrin_neon.h"

#define COMPV_HAMMING32_MIN_SAMPLES_PER_THREAD					1 // use max threads
#define COMPV_HAMMING32_MIN_SAMPLES_PER_THREAD_POPCNT_AVX2		120 // for hamming distance (Fast Popcnt using Mula's formula)

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM
#	if COMPV_ARCH_X86
	COMPV_EXTERNC void CompVMathDistanceHamming_Asm_X86_POPCNT_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr);
	COMPV_EXTERNC void CompVMathDistanceHamming32_Asm_X86_POPCNT_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr);
#	endif /* COMPV_ARCH_X86 */
#	if COMPV_ARCH_X64
	COMPV_EXTERNC void CompVMathDistanceHamming_Asm_X64_POPCNT_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr);
	COMPV_EXTERNC void CompVMathDistanceHamming32_Asm_X64_POPCNT_SSE42(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr);
	COMPV_EXTERNC void CompVMathDistanceHamming32_Asm_X64_POPCNT(COMPV_ALIGNED(SSE) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t stride, COMPV_ALIGNED(SSE) const uint8_t* patch1xnPtr, int32_t* distPtr);
	COMPV_EXTERNC void CompVMathDistanceHamming32_Asm_X64_POPCNT_AVX2(COMPV_ALIGNED(AVX) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(AVX) compv_uscalar_t stride, COMPV_ALIGNED(AVX) const uint8_t* patch1xnPtr, COMPV_ALIGNED(AVX) int32_t* distPtr);
#	endif /* COMPV_ARCH_X64 */
#	if COMPV_ARCH_ARM32
    COMPV_EXTERNC void CompVMathDistanceHamming_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr);
    COMPV_EXTERNC void CompVMathDistanceHamming32_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr);
#	endif /* COMPV_ARCH_ARM32 */
#	if COMPV_ARCH_ARM64
    COMPV_EXTERNC void CompVMathDistanceHamming_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr);
    COMPV_EXTERNC void CompVMathDistanceHamming32_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr);
#	endif /* COMPV_ARCH_ARM32 */
#endif /* COMPV_ASM */


static void CompVHammingDistance_C(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr);
#if COMPV_ARCH_X86
static void CompVHammingDistance_POPCNT_C(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr);
#endif

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
	void(*HammingDistance32)(const uint8_t* dataPtr, compv_uscalar_t height, compv_uscalar_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr) = NULL;

	size_t minSamplesPerThread = COMPV_HAMMING32_MIN_SAMPLES_PER_THREAD;

#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagPOPCNT)) {
		HammingDistance = CompVHammingDistance_POPCNT_C;
		if (width > 15 && CompVCpu::isEnabled(kCpuFlagSSE42) && COMPV_IS_ALIGNED_SSE(dataPtr) && COMPV_IS_ALIGNED_SSE(patch1xnPtr) && COMPV_IS_ALIGNED_SSE(stride)) {
			COMPV_EXEC_IFDEF_INTRIN_X86(HammingDistance = CompVMathDistanceHamming_Intrin_POPCNT_SSE42);
			COMPV_EXEC_IFDEF_ASM_X86(HammingDistance = CompVMathDistanceHamming_Asm_X86_POPCNT_SSE42);
			COMPV_EXEC_IFDEF_ASM_X64(HammingDistance = CompVMathDistanceHamming_Asm_X64_POPCNT_SSE42);
		}
		// Width == 32 -> Very common (Brief256_31)
		if (width == 32) { 
			COMPV_EXEC_IFDEF_ASM_X64(HammingDistance32 = CompVMathDistanceHamming32_Asm_X64_POPCNT); // Pure asm code is Faster than the SSE (tested using core i7)
			if (CompVCpu::isEnabled(kCpuFlagSSE42) && COMPV_IS_ALIGNED_SSE(dataPtr) && COMPV_IS_ALIGNED_SSE(patch1xnPtr) && COMPV_IS_ALIGNED_SSE(stride)) {
				COMPV_EXEC_IFDEF_ASM_X86(HammingDistance32 = CompVMathDistanceHamming32_Asm_X86_POPCNT_SSE42);
				COMPV_EXEC_IFDEF_ASM_X64(HammingDistance32 = CompVMathDistanceHamming32_Asm_X64_POPCNT_SSE42);
			}
			if (CompVCpu::isEnabled(kCpuFlagAVX2) && COMPV_IS_ALIGNED_AVX2(dataPtr) && COMPV_IS_ALIGNED_AVX2(patch1xnPtr) && COMPV_IS_ALIGNED_AVX2(stride)) {
				COMPV_EXEC_IFDEF_INTRIN_X86((HammingDistance32 = CompVMathDistanceHamming32_Intrin_POPCNT_AVX2, minSamplesPerThread = COMPV_HAMMING32_MIN_SAMPLES_PER_THREAD_POPCNT_AVX2)); // Mula's algorithm
				COMPV_EXEC_IFDEF_ASM_X64((HammingDistance32 = CompVMathDistanceHamming32_Asm_X64_POPCNT_AVX2, minSamplesPerThread = COMPV_HAMMING32_MIN_SAMPLES_PER_THREAD_POPCNT_AVX2)); // Mula's algorithm
				// TODO(dmi): add "CompVMathDistanceHamming32_Intrin_POPCNT_SSE42" using mula's algorithm
			}
		}
	}
#elif COMPV_ARCH_ARM
	if (width > 15 && CompVCpu::isEnabled(kCpuFlagARM_NEON) && COMPV_IS_ALIGNED_NEON(dataPtr) && COMPV_IS_ALIGNED_NEON(patch1xnPtr) && COMPV_IS_ALIGNED_NEON(stride)) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(HammingDistance = CompVMathDistanceHamming_Intrin_NEON);
        COMPV_EXEC_IFDEF_ASM_ARM32(HammingDistance = CompVMathDistanceHamming_Asm_NEON32);
        COMPV_EXEC_IFDEF_ASM_ARM64(HammingDistance = CompVMathDistanceHamming_Asm_NEON64);
		// Width == 32 -> Very common (Brief256_31)
		if (width == 32) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(HammingDistance32 = CompVMathDistanceHamming32_Intrin_NEON);
            COMPV_EXEC_IFDEF_ASM_ARM32(HammingDistance32 = CompVMathDistanceHamming32_Asm_NEON32);
		}
	}
#endif

	// TODO(dmi): for large sizes (not 32 version) use Harley-Seal or Mula AVX2 algorithm : https://arxiv.org/pdf/1611.07612.pdf

	if (width == 32 && !HammingDistance32) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found for fast hamming distance (32 x n)");
	}

#define COMPV_HAMMING_EXEC(dataPtr__, width__, height__, stride__, patch1xnPtr__, distPtr__) \
	if (HammingDistance32) HammingDistance32(dataPtr__, height__, stride__, patch1xnPtr__, distPtr__); \
	else  HammingDistance(dataPtr__, width__, height__, stride__, patch1xnPtr__, distPtr__)

	// Compute number of threads
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	size_t threadsCount = COMPV_MATH_CLIP3(1, maxThreads, height / minSamplesPerThread);
	
	if (threadsCount > 1) {
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCount);
		size_t counts = static_cast<size_t>(height / threadsCount);
		size_t lastCount = height - ((threadsCount - 1) * counts);
		size_t countsTimesStride = (counts * stride);
		auto funcPtr = [&](const uint8_t* dataPtr_, compv_uscalar_t width_, compv_uscalar_t height_, compv_uscalar_t stride_, const uint8_t* patch1xnPtr_, int32_t* distPtr_) -> void {
			COMPV_HAMMING_EXEC(dataPtr_, width_, height_, stride_, patch1xnPtr_, distPtr_);
		};
		for (size_t i = 0; i < threadsCount; ++i) {
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtr, &dataPtr[countsTimesStride * i], width, (i == (threadsCount - 1)) ? lastCount : counts, stride, patch1xnPtr, &distPtr[counts * i]), taskIds));
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds));
	}
	else {
		COMPV_HAMMING_EXEC(dataPtr, width, height, stride, patch1xnPtr, distPtr);
	}

#undef COMPV_HAMMING_EXEC

	return COMPV_ERROR_CODE_S_OK;
}

// Private function, up to the caller to check input parameters
static void CompVHammingDistance_C(const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, const uint8_t* patch1xnPtr, int32_t* distPtr)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found");

	compv_uscalar_t i, j, cnt;
#if COMPV_ARCH_X64
	uint64_t pop;
#else
	uint8_t pop;
#endif

	for (j = 0; j < height; ++j) {
		cnt = 0;
		for (i = 0; i < width; ++i) {
#if COMPV_ARCH_X64  /*|| COMPV_ARCH_ARM64 */ // Next code is horribly slooow on ARM64
			// http://graphics.stanford.edu/~seander/bithacks.html#CountBitsSetParallel
			pop = dataPtr[i] ^ patch1xnPtr[i];
			pop = pop - ((pop >> 0x1) & 0x5555555555555555);
			pop = (pop & 0x3333333333333333) + ((pop >> 0x2) & 0x3333333333333333);
			pop = (pop + (pop >> 0x4)) & 0xf0f0f0f0f0f0f0f;
			cnt += (pop * 0x101010101010101) >> 0x38;
#else
			pop = dataPtr[i] ^ patch1xnPtr[i];
			cnt += kPopcnt256[pop];
#endif
		}
		dataPtr += stride;
		distPtr[j] = static_cast<int32_t>(cnt);
	}
}

#if COMPV_ARCH_X86
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
			pop = *reinterpret_cast<const uint64_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint64_t*>(&patch1xnPtr[i]);
			cnt += compv_popcnt64(pop);
		}
#endif
		for (; i <= width - 4; i += 4) {
			pop = *reinterpret_cast<const uint32_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint32_t*>(&patch1xnPtr[i]);
			cnt += compv_popcnt32(static_cast<uint32_t>(pop));
		}
		if (i <= width - 2) {
			pop = *reinterpret_cast<const uint16_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint16_t*>(&patch1xnPtr[i]);
			cnt += compv_popcnt16(static_cast<uint16_t>(pop));
			i += 2;
		}
		if (i <= width - 1) {
			pop = *reinterpret_cast<const uint8_t*>(&dataPtr[i]) ^ *reinterpret_cast<const uint8_t*>(&patch1xnPtr[i]);
			cnt += compv_popcnt16(static_cast<uint16_t>(pop));
			++i;
		}
		dataPtr += stride;
		distPtr[j] = static_cast<int32_t>(cnt);
	}
}
#endif

COMPV_NAMESPACE_END()
