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

#include "compv/base/math/intrin/x86/compv_math_distance_intrin_sse2.h"
#include "compv/base/math/intrin/x86/compv_math_distance_intrin_sse42.h"
#include "compv/base/math/intrin/x86/compv_math_distance_intrin_avx.h"
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
	COMPV_EXTERNC void CompVMathDistanceLine_32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* xPtr, COMPV_ALIGNED(SSE) const compv_float32_t* yPtr, const compv_float32_t* Ascaled1, const compv_float32_t* Bscaled1, const compv_float32_t* Cscaled1, COMPV_ALIGNED(SSE) compv_float32_t* distPtr, const compv_uscalar_t count);
	COMPV_EXTERNC void CompVMathDistanceLine_32f_Asm_X64_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* xPtr, COMPV_ALIGNED(AVX) const compv_float32_t* yPtr, const compv_float32_t* Ascaled1, const compv_float32_t* Bscaled1, const compv_float32_t* Cscaled1, COMPV_ALIGNED(AVX) compv_float32_t* distPtr, const compv_uscalar_t count);
	COMPV_EXTERNC void CompVMathDistanceLine_32f_Asm_X64_FMA3_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* xPtr, COMPV_ALIGNED(AVX) const compv_float32_t* yPtr, const compv_float32_t* Ascaled1, const compv_float32_t* Bscaled1, const compv_float32_t* Cscaled1, COMPV_ALIGNED(AVX) compv_float32_t* distPtr, const compv_uscalar_t count);
	COMPV_EXTERNC void CompVMathDistanceParabola_32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* xPtr, COMPV_ALIGNED(SSE) const compv_float32_t* yPtr, const compv_float32_t* A1, const compv_float32_t* B1, const compv_float32_t* C1, COMPV_ALIGNED(SSE) compv_float32_t* distPtr, const compv_uscalar_t count);
	COMPV_EXTERNC void CompVMathDistanceParabola_32f_Asm_X64_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* xPtr, COMPV_ALIGNED(AVX) const compv_float32_t* yPtr, const compv_float32_t* A1, const compv_float32_t* B1, const compv_float32_t* C1, COMPV_ALIGNED(AVX) compv_float32_t* distPtr, const compv_uscalar_t count);
	COMPV_EXTERNC void CompVMathDistanceParabola_32f_Asm_X64_FMA3_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* xPtr, COMPV_ALIGNED(AVX) const compv_float32_t* yPtr, const compv_float32_t* A1, const compv_float32_t* B1, const compv_float32_t* C1, COMPV_ALIGNED(AVX) compv_float32_t* distPtr, const compv_uscalar_t count);
#	endif /* COMPV_ARCH_X64 */
#	if COMPV_ARCH_ARM32
    COMPV_EXTERNC void CompVMathDistanceHamming_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr);
    COMPV_EXTERNC void CompVMathDistanceHamming32_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr);
	COMPV_EXTERNC void CompVMathDistanceLine_32f_Asm_NEON32(COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, const compv_float32_t* Ascaled1, const compv_float32_t* Bscaled1, const compv_float32_t* Cscaled1, COMPV_ALIGNED(NEON) compv_float32_t* distPtr, const compv_uscalar_t count);
	COMPV_EXTERNC void CompVMathDistanceLine_32f_Asm_FMA_NEON32(COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, const compv_float32_t* Ascaled1, const compv_float32_t* Bscaled1, const compv_float32_t* Cscaled1, COMPV_ALIGNED(NEON) compv_float32_t* distPtr, const compv_uscalar_t count);
	COMPV_EXTERNC void CompVMathDistanceParabola_32f_Asm_NEON32(COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, const compv_float32_t* A1, const compv_float32_t* B1, const compv_float32_t* C1, COMPV_ALIGNED(NEON) compv_float32_t* distPtr, const compv_uscalar_t count);
	COMPV_EXTERNC void CompVMathDistanceParabola_32f_Asm_FMA_NEON32(COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, const compv_float32_t* A1, const compv_float32_t* B1, const compv_float32_t* C1, COMPV_ALIGNED(NEON) compv_float32_t* distPtr, const compv_uscalar_t count);
#	endif /* COMPV_ARCH_ARM32 */
#	if COMPV_ARCH_ARM64
    COMPV_EXTERNC void CompVMathDistanceHamming_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr);
    COMPV_EXTERNC void CompVMathDistanceHamming32_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* dataPtr, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t stride, COMPV_ALIGNED(NEON) const uint8_t* patch1xnPtr, int32_t* distPtr);
	COMPV_EXTERNC void CompVMathDistanceLine_32f_Asm_NEON64(COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, const compv_float32_t* Ascaled1, const compv_float32_t* Bscaled1, const compv_float32_t* Cscaled1, COMPV_ALIGNED(NEON) compv_float32_t* distPtr, const compv_uscalar_t count);
	COMPV_EXTERNC void CompVMathDistanceLine_32f_Asm_FMA_NEON64(COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, const compv_float32_t* Ascaled1, const compv_float32_t* Bscaled1, const compv_float32_t* Cscaled1, COMPV_ALIGNED(NEON) compv_float32_t* distPtr, const compv_uscalar_t count);
	COMPV_EXTERNC void CompVMathDistanceParabola_32f_Asm_NEON64(COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, const compv_float32_t* A1, const compv_float32_t* B1, const compv_float32_t* C1, COMPV_ALIGNED(NEON) compv_float32_t* distPtr, const compv_uscalar_t count);
	COMPV_EXTERNC void CompVMathDistanceParabola_32f_Asm_FMA_NEON64(COMPV_ALIGNED(NEON) const compv_float32_t* xPtr, COMPV_ALIGNED(NEON) const compv_float32_t* yPtr, const compv_float32_t* A1, const compv_float32_t* B1, const compv_float32_t* C1, COMPV_ALIGNED(NEON) compv_float32_t* distPtr, const compv_uscalar_t count);
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
			if (CompVCpu::isEnabled(kCpuFlagSSE42) && COMPV_IS_ALIGNED_SSE(dataPtr) && COMPV_IS_ALIGNED_SSE(patch1xnPtr) && COMPV_IS_ALIGNED_SSE(stride)) {
				COMPV_EXEC_IFDEF_ASM_X86(HammingDistance32 = CompVMathDistanceHamming32_Asm_X86_POPCNT_SSE42);
				COMPV_EXEC_IFDEF_ASM_X64(HammingDistance32 = CompVMathDistanceHamming32_Asm_X64_POPCNT_SSE42);
			}
			COMPV_EXEC_IFDEF_ASM_X64(HammingDistance32 = CompVMathDistanceHamming32_Asm_X64_POPCNT); // Pure asm code is Faster than the SSE (tested using core i7)
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
            COMPV_EXEC_IFDEF_ASM_ARM64(HammingDistance32 = CompVMathDistanceHamming32_Asm_NEON64);
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
	const size_t maxThreads = (threadDisp && !threadDisp->isMotherOfTheCurrentThread()) ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	const size_t threadsCount = COMPV_MATH_CLIP3(1, maxThreads, (width*height) / minSamplesPerThread);
	
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

template <typename FloatType>
static void CompVMathDistanceLine_C(const FloatType* xPtr, const FloatType* yPtr, const FloatType* Ascaled1, const FloatType* Bscaled1, const FloatType* Cscaled1, FloatType* distPtr, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	const FloatType& Ascaled = *Ascaled1;
	const FloatType& Bscaled = *Bscaled1;
	const FloatType& Cscaled = *Cscaled1;
	for (compv_uscalar_t i = 0; i < count; ++i) {
		// (Ax + By + C) and (Ax + C + By)  produce slithly different results but we keep using the 2nd 
		// one to make sure this code will produce same MD5 hash when tested against SIMD (SSE, AVX or NEON)
		distPtr[i] = std::abs((Ascaled * xPtr[i]) + Cscaled + (Bscaled * yPtr[i]));
	}
}

// Distance = abs(Ax + By + C)/(1/sqrt(A^2 + B^2)) with A = lineEq[0], B = lineEq[1] and C = lineEq[2]
// https://en.wikipedia.org/wiki/Distance_from_a_point_to_a_line#Line_defined_by_an_equation
COMPV_ERROR_CODE CompVMathDistance::line(const CompVMatPtr& points, const double(&lineEq)[3], CompVMatPtrPtr distances)
{
	COMPV_CHECK_EXP_RETURN(
		!points || points->cols() < 2 || !distances || points == *distances
		|| (points->subType() != COMPV_SUBTYPE_RAW_FLOAT32 && points->subType() != COMPV_SUBTYPE_RAW_FLOAT64)
		|| (!lineEq[0] && !lineEq[1]) // A and B cannot be equal to zero at the same time
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVMatPtr distances_ = *distances;
	const size_t count = points->cols();

	// d = abs(Ax + By + C) / (1/sqrt(A^2 + B^2))
	//	-> d = abs(Ax + By + C) * S, with S = (1/sqrt(A^2 + B^2))
	//	-> d = abs((A*S)x + (B*S)y + (C*S))
	//	-> d = abs((A')x + (B')y + (C'))
	const double scale = 1.0 / std::sqrt((lineEq[0] * lineEq[0]) + (lineEq[1] * lineEq[1])); // A and B cannot be equal to zero at the same time
	const double lineEqScaled[3] = {
		lineEq[0] * scale,
		lineEq[1] * scale,
		lineEq[2] * scale,
	};

	switch (points->subType()) {
	case COMPV_SUBTYPE_RAW_FLOAT32: {
		// Create distance only if type mismatch
		if (!distances_ || distances_->subType() != COMPV_SUBTYPE_RAW_FLOAT32 || distances_->cols() != count || distances_->rows() != 1) {
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&distances_, 1, count));
		}
		const compv_float32_t Ascaled = static_cast<compv_float32_t>(lineEqScaled[0]);
		const compv_float32_t Bscaled = static_cast<compv_float32_t>(lineEqScaled[1]);
		const compv_float32_t Cscaled = static_cast<compv_float32_t>(lineEqScaled[2]);
		void(*CompVMathDistanceLine_32f)(const compv_float32_t* xPtr, const compv_float32_t* yPtr, const compv_float32_t* A1scaled, const compv_float32_t* B1scaled, const compv_float32_t* C1scaled, compv_float32_t* distPtr, const compv_uscalar_t count)
			= [](const compv_float32_t* xPtr, const compv_float32_t* yPtr, const compv_float32_t* A1scaled, const compv_float32_t* B1scaled, const compv_float32_t* C1scaled, compv_float32_t* distPtr, const compv_uscalar_t count) {
			CompVMathDistanceLine_C<compv_float32_t>(xPtr, yPtr, A1scaled, B1scaled, C1scaled, distPtr, count);
		};
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && points->isAlignedSSE() && distances_->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathDistanceLine_32f = CompVMathDistanceLine_32f_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathDistanceLine_32f = CompVMathDistanceLine_32f_Asm_X64_SSE2);
		} 
		if (CompVCpu::isEnabled(kCpuFlagAVX) && points->isAlignedAVX() && distances_->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathDistanceLine_32f = CompVMathDistanceLine_32f_Intrin_AVX);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathDistanceLine_32f = CompVMathDistanceLine_32f_Asm_X64_AVX);
			if (CompVCpu::isEnabled(kCpuFlagFMA3)) {
				COMPV_EXEC_IFDEF_ASM_X64(CompVMathDistanceLine_32f = CompVMathDistanceLine_32f_Asm_X64_FMA3_AVX);
			}
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && points->isAlignedNEON() && distances_->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathDistanceLine_32f = CompVMathDistanceLine_32f_Intrin_NEON);
			COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathDistanceLine_32f = CompVMathDistanceLine_32f_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathDistanceLine_32f = CompVMathDistanceLine_32f_Asm_NEON64);
			if (CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) {
				COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathDistanceLine_32f = CompVMathDistanceLine_32f_Asm_FMA_NEON32);
				//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathDistanceLine_32f = CompVMathDistanceLine_32f_Asm_FMA_NEON64);
			}
		}
#endif
		CompVMathDistanceLine_32f(points->ptr<const compv_float32_t>(0), points->ptr<const compv_float32_t>(1), &Ascaled, &Bscaled, &Cscaled, distances_->ptr<compv_float32_t>(), static_cast<compv_uscalar_t>(count));
		break;
	}
	case COMPV_SUBTYPE_RAW_FLOAT64: {
		// Create distance only if type mismatch
		if (!distances_ || distances_->subType() != COMPV_SUBTYPE_RAW_FLOAT64 || distances_->cols() != count || distances_->rows() != 1) {
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&distances_, 1, count));
		}
		const compv_float64_t Ascaled = lineEqScaled[0];
		const compv_float64_t Bscaled = lineEqScaled[1];
		const compv_float64_t Cscaled = lineEqScaled[2];
		void(*CompVMathDistanceLine_64f)(const compv_float64_t* xPtr, const compv_float64_t* yPtr, const compv_float64_t* A1scaled, const compv_float64_t* B1scaled, const compv_float64_t* C1scaled, compv_float64_t* distPtr, const compv_uscalar_t count)
			= [](const compv_float64_t* xPtr, const compv_float64_t* yPtr, const compv_float64_t* A1scaled, const compv_float64_t* B1scaled, const compv_float64_t* C1scaled, compv_float64_t* distPtr, const compv_uscalar_t count) {
			CompVMathDistanceLine_C<compv_float64_t>(xPtr, yPtr, A1scaled, B1scaled, C1scaled, distPtr, count);
		};
		CompVMathDistanceLine_64f(points->ptr<const compv_float64_t>(0), points->ptr<const compv_float64_t>(1), &Ascaled, &Bscaled, &Cscaled, distances_->ptr<compv_float64_t>(), static_cast<compv_uscalar_t>(count));
		break;
	}
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_SUBTYPE, "Points must be float32 or float64");
		break;
	}

	*distances = distances_;
	return COMPV_ERROR_CODE_S_OK;
}

template <typename FloatType>
static void CompVMathDistanceParabola_C(const FloatType* xPtr, const FloatType* yPtr, const FloatType* A1, const FloatType* B1, const FloatType* C1, FloatType* distPtr, const compv_uscalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	const FloatType& A = *A1;
	const FloatType& B = *B1;
	const FloatType& C = *C1;
	for (compv_uscalar_t i = 0; i < count; ++i) {
		// (Ax^2 + Bx + C) and (Ax^2 + C + Bx)  produce slithly different results but we keep using the 2nd 
		// one to make sure this code will produce same MD5 hash when tested against SIMD (SSE, AVX or NEON)
		distPtr[i] = std::abs(((A * (xPtr[i] * xPtr[i])) + C + (B * xPtr[i])) - yPtr[i]);
	}
}


// Distance = abs(((A * x^2) + (B * x) + C) - y) with A = lineEq[0], B = lineEq[1] and C = lineEq[2]
// If the parabola result R = ((A * x^2) + (B * x) + C) need to be scaled/normalized before substracting y then,
// normalize A, B and C before calling this function instead of R.
COMPV_ERROR_CODE CompVMathDistance::parabola(const CompVMatPtr& points, const double(&parabolaEq)[3], CompVMatPtrPtr distances, const COMPV_MATH_PARABOLA_TYPE type COMPV_DEFAULT(COMPV_MATH_PARABOLA_TYPE_REGULAR))
{
	COMPV_CHECK_EXP_RETURN(
		!points || points->cols() < 2 || !distances || points == *distances
		|| (points->subType() != COMPV_SUBTYPE_RAW_FLOAT32 && points->subType() != COMPV_SUBTYPE_RAW_FLOAT64)
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVMatPtr distances_ = *distances;
	const size_t count = points->cols();
	const bool sideways = (type == COMPV_MATH_PARABOLA_TYPE_SIDEWAYS);
	const size_t xIndex = (sideways ? 1 : 0);
	const size_t yIndex = (sideways ? 0 : 1);

	switch (points->subType()) {
	case COMPV_SUBTYPE_RAW_FLOAT32: {
		// Create distance only if type mismatch
		if (!distances_ || distances_->subType() != COMPV_SUBTYPE_RAW_FLOAT32 || distances_->cols() != count || distances_->rows() != 1) {
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&distances_, 1, count));
		}
		const compv_float32_t A = static_cast<compv_float32_t>(parabolaEq[0]);
		const compv_float32_t B = static_cast<compv_float32_t>(parabolaEq[1]);
		const compv_float32_t C = static_cast<compv_float32_t>(parabolaEq[2]);
		void(*CompVMathDistanceParabola_32f)(const compv_float32_t* xPtr, const compv_float32_t* yPtr, const compv_float32_t* A1, const compv_float32_t* B1, const compv_float32_t* C1, compv_float32_t* distPtr, const compv_uscalar_t count)
			= [](const compv_float32_t* xPtr, const compv_float32_t* yPtr, const compv_float32_t* A1, const compv_float32_t* B1, const compv_float32_t* C1, compv_float32_t* distPtr, const compv_uscalar_t count) {
			CompVMathDistanceParabola_C<compv_float32_t>(xPtr, yPtr, A1, B1, C1, distPtr, count);
		};
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && points->isAlignedSSE() && distances_->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathDistanceParabola_32f = CompVMathDistanceParabola_32f_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathDistanceParabola_32f = CompVMathDistanceParabola_32f_Asm_X64_SSE2);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX) && points->isAlignedAVX() && distances_->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathDistanceParabola_32f = CompVMathDistanceParabola_32f_Intrin_AVX);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathDistanceParabola_32f = CompVMathDistanceParabola_32f_Asm_X64_AVX);
			if (CompVCpu::isEnabled(kCpuFlagFMA3)) {
				COMPV_EXEC_IFDEF_ASM_X64(CompVMathDistanceParabola_32f = CompVMathDistanceParabola_32f_Asm_X64_FMA3_AVX);
			}
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && points->isAlignedNEON() && distances_->isAlignedNEON()) {
			//COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathDistanceParabola_32f = CompVMathDistanceParabola_32f_Intrin_NEON);
			//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathDistanceParabola_32f = CompVMathDistanceParabola_32f_Asm_NEON32);
			//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathDistanceParabola_32f = CompVMathDistanceParabola_32f_Asm_NEON64);
			if (CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) {
				//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathDistanceParabola_32f = CompVMathDistanceParabola_32f_Asm_FMA_NEON32);
				//COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathDistanceParabola_32f = CompVMathDistanceParabola_32f_Asm_FMA_NEON64);
			}
		}
#endif
		CompVMathDistanceParabola_32f(points->ptr<const compv_float32_t>(xIndex), points->ptr<const compv_float32_t>(yIndex), &A, &B, &C, distances_->ptr<compv_float32_t>(), static_cast<compv_uscalar_t>(count));
		break;
	}
	case COMPV_SUBTYPE_RAW_FLOAT64: {
		// Create distance only if type mismatch
		if (!distances_ || distances_->subType() != COMPV_SUBTYPE_RAW_FLOAT64 || distances_->cols() != count || distances_->rows() != 1) {
			COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float64_t>(&distances_, 1, count));
		}
		const compv_float64_t Ascaled = parabolaEq[0];
		const compv_float64_t Bscaled = parabolaEq[1];
		const compv_float64_t Cscaled = parabolaEq[2];
		void(*CompVMathDistanceLine_64f)(const compv_float64_t* xPtr, const compv_float64_t* yPtr, const compv_float64_t* A1, const compv_float64_t* B1, const compv_float64_t* C1, compv_float64_t* distPtr, const compv_uscalar_t count)
			= [](const compv_float64_t* xPtr, const compv_float64_t* yPtr, const compv_float64_t* A1, const compv_float64_t* B1, const compv_float64_t* C1, compv_float64_t* distPtr, const compv_uscalar_t count) {
			CompVMathDistanceParabola_C<compv_float64_t>(xPtr, yPtr, A1, B1, C1, distPtr, count);
		};
		CompVMathDistanceLine_64f(points->ptr<const compv_float64_t>(xIndex), points->ptr<const compv_float64_t>(yIndex), &Ascaled, &Bscaled, &Cscaled, distances_->ptr<compv_float64_t>(), static_cast<compv_uscalar_t>(count));
		break;
	}
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_SUBTYPE, "Points must be float32 or float64");
		break;
	}

	*distances = distances_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
