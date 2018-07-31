/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_bits.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_generic_invoke.h"

#include "compv/base/intrin/x86/compv_bits_intrin_sse2.h"
#include "compv/base/intrin/arm/compv_bits_intrin_neon.h"

#define COMPV_BITS_AND_SAMPLES_PER_THREAD		(100 * 100)
#define COMPV_BITS_NOT_AND_SAMPLES_PER_THREAD	(100 * 100)
#define COMPV_BITS_NOT_SAMPLES_PER_THREAD		(100 * 100)
#define COMPV_BITS_XORVT_SAMPLES_PER_THREAD		(100 * 100)

COMPV_BASE_API compv::compv_uscalar_t kPopcnt256[] = {
	0, 1, 1, 2, 1, 2, 2, 3, 1, 2, 2, 3, 2, 3, 3, 4, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3,
	4, 3, 4, 4, 5, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4
	, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 1, 2, 2, 3, 2, 3, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4,
	5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 1, 2, 2, 3, 2, 3
	, 3, 4, 2, 3, 3, 4, 3, 4, 4, 5, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6,
	2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5,
	6, 5, 6, 6, 7, 2, 3, 3, 4, 3, 4, 4, 5, 3, 4, 4, 5, 4, 5, 5, 6, 3, 4, 4, 5, 4, 5
	, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7, 3, 4, 4, 5, 4, 5, 5, 6, 4, 5, 5, 6, 5, 6, 6, 7,
	4, 5, 5, 6, 5, 6, 6, 7, 5, 6, 6, 7, 6, 7, 7, 8,
};

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVBitsLogicalAnd_8u_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* Aptr, COMPV_ALIGNED(SSE) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(SSE) compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t Astride, COMPV_ALIGNED(SSE) compv_uscalar_t Bstride, COMPV_ALIGNED(SSE) compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVBitsLogicalNotAnd_8u_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* Aptr, COMPV_ALIGNED(SSE) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(SSE) compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t Astride, COMPV_ALIGNED(SSE) compv_uscalar_t Bstride, COMPV_ALIGNED(SSE) compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVBitsLogicalNot_8u_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* Aptr, COMPV_ALIGNED(SSE) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t Astride, COMPV_ALIGNED(SSE) compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVBitsLogicalXorVt_8u_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const uint8_t* Aptr, COMPV_ALIGNED(SSE) const uint8_t* A_Minus1_ptr, COMPV_ALIGNED(SSE) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(SSE) compv_uscalar_t Astride, COMPV_ALIGNED(SSE) compv_uscalar_t Rstride);
#endif /* COMPV_ARCH_X64 */

#if COMPV_ASM && COMPV_ARCH_ARM32
COMPV_EXTERNC void CompVBitsLogicalAnd_8u_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(NEON) compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Bstride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVBitsLogicalNotAnd_8u_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(NEON) compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Bstride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVBitsLogicalNot_8u_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVBitsLogicalXorVt_8u_Asm_NEON32(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(SSE) const uint8_t* A_Minus1_ptr, COMPV_ALIGNED(NEON) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);
#endif /* COMPV_ARCH_ARM32 */

#if COMPV_ASM && COMPV_ARCH_ARM64
COMPV_EXTERNC void CompVBitsLogicalAnd_8u_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(NEON) compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Bstride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVBitsLogicalNotAnd_8u_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, COMPV_ALIGNED(NEON) compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Bstride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVBitsLogicalNot_8u_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(NEON) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVBitsLogicalXorVt_8u_Asm_NEON64(COMPV_ALIGNED(NEON) const uint8_t* Aptr, COMPV_ALIGNED(SSE) const uint8_t* A_Minus1_ptr, COMPV_ALIGNED(NEON) uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, COMPV_ALIGNED(NEON) compv_uscalar_t Astride, COMPV_ALIGNED(NEON) compv_uscalar_t Rstride);
#endif /* COMPV_ARCH_ARM64 */

static void CompVBitsLogicalAnd_8u_C(const uint8_t* Aptr, const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride);
static void CompVBitsLogicalNotAnd_8u_C(const uint8_t* Aptr, const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride);
static void CompVBitsLogicalNot_8u_C(const uint8_t* Aptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Rstride);
static void CompVBitsLogicalXorVt_8u_C(const uint8_t* Aptr, const uint8_t* A_Minus1_ptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Rstride);

// R = (A & B)
// Supports and type (float, double, uint8, uint16....)
COMPV_ERROR_CODE CompVBits::logical_and(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtrPtr R)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !R || A->cols() != B->cols() || A->rows() != B->rows() || A->planeCount() != B->planeCount(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr R_ = *R;
	if (!R_ || (R_ != A && R_ != B)) { // This function allows R to be equal to A or B
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&R_, A));
	}

	void(*CompVBitsLogicalAnd_8u)(const uint8_t* Aptr, const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride)
		= CompVBitsLogicalAnd_8u_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && A->isAlignedSSE() && B->isAlignedSSE() && R_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVBitsLogicalAnd_8u = CompVBitsLogicalAnd_8u_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVBitsLogicalAnd_8u = CompVBitsLogicalAnd_8u_Asm_X64_SSE2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && A->isAlignedNEON() && B->isAlignedNEON() && R_->isAlignedNEON()) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVBitsLogicalAnd_8u = CompVBitsLogicalAnd_8u_Intrin_NEON);
		COMPV_EXEC_IFDEF_ASM_ARM32(CompVBitsLogicalAnd_8u = CompVBitsLogicalAnd_8u_Asm_NEON32);
		COMPV_EXEC_IFDEF_ASM_ARM64(CompVBitsLogicalAnd_8u = CompVBitsLogicalAnd_8u_Asm_NEON64);
	}
#endif

	int planeId = 0;
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		CompVBitsLogicalAnd_8u(
			A->ptr<const uint8_t>(ystart, 0, planeId), B->ptr<const uint8_t>(ystart, 0, planeId), R_->ptr<uint8_t>(ystart, 0, planeId),
			static_cast<compv_uscalar_t>(A->cols(planeId)), static_cast<compv_uscalar_t>(yend - ystart),
			static_cast<compv_uscalar_t>(A->strideInBytes(planeId)), static_cast<compv_uscalar_t>(B->strideInBytes(planeId)), static_cast<compv_uscalar_t>(R_->strideInBytes(planeId))
		);
		return COMPV_ERROR_CODE_S_OK;
	};

	const int planesCount = static_cast<int>(A->planeCount());
	for (planeId = 0; planeId < planesCount; ++planeId) {
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			A->cols(planeId),
			A->rows(planeId),
			COMPV_BITS_AND_SAMPLES_PER_THREAD
		));
	}

	*R = R_;
	return COMPV_ERROR_CODE_S_OK;
}

// R = (~A & B)
// Supports and type (float, double, uint8, uint16....)
COMPV_ERROR_CODE CompVBits::logical_not_and(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtrPtr R)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !R || A->cols() != B->cols() || A->rows() != B->rows() || A->planeCount() != B->planeCount(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr R_ = *R;
	if (!R_ || (R_ != A && R_ != B)) { // This function allows R to be equal to A or B
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&R_, A));
	}

	void(*CompVBitsLogicalNotAnd_8u)(const uint8_t* Aptr, const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride)
		= CompVBitsLogicalNotAnd_8u_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && A->isAlignedSSE() && B->isAlignedSSE() && R_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVBitsLogicalNotAnd_8u = CompVBitsLogicalNotAnd_8u_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVBitsLogicalNotAnd_8u = CompVBitsLogicalNotAnd_8u_Asm_X64_SSE2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && A->isAlignedNEON() && B->isAlignedNEON() && R_->isAlignedNEON()) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVBitsLogicalNotAnd_8u = CompVBitsLogicalNotAnd_8u_Intrin_NEON);
		COMPV_EXEC_IFDEF_ASM_ARM32(CompVBitsLogicalNotAnd_8u = CompVBitsLogicalNotAnd_8u_Asm_NEON32);
		COMPV_EXEC_IFDEF_ASM_ARM64(CompVBitsLogicalNotAnd_8u = CompVBitsLogicalNotAnd_8u_Asm_NEON64);
	}
#endif

	int planeId = 0;
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		CompVBitsLogicalNotAnd_8u(
			A->ptr<const uint8_t>(ystart, 0, planeId), B->ptr<const uint8_t>(ystart, 0, planeId), R_->ptr<uint8_t>(ystart, 0, planeId),
			static_cast<compv_uscalar_t>(A->cols(planeId)), static_cast<compv_uscalar_t>(yend - ystart),
			static_cast<compv_uscalar_t>(A->strideInBytes(planeId)), static_cast<compv_uscalar_t>(B->strideInBytes(planeId)), static_cast<compv_uscalar_t>(R_->strideInBytes(planeId))
		);
		return COMPV_ERROR_CODE_S_OK;
	};

	const int planesCount = static_cast<int>(A->planeCount());
	for (planeId = 0; planeId < planesCount; ++planeId) {
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			A->cols(planeId),
			A->rows(planeId),
			COMPV_BITS_AND_SAMPLES_PER_THREAD
		));
	}

	*R = R_;
	return COMPV_ERROR_CODE_S_OK;
}

// R = ~A
// Supports and type (float, double, uint8, uint16....)
COMPV_ERROR_CODE CompVBits::logical_not(const CompVMatPtr& A, CompVMatPtrPtr R)
{
	COMPV_CHECK_EXP_RETURN(!A || !R, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr R_ = (!A->isMemoryOwed() && A == *R) ? nullptr : *R;
	if (!R_ || R_ != A) { // This function allows R to be equal to A
		if (A->isMemoryOwed()) {
			COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&R_, A));
		}
		else { 
			// Memory not owed means A is a bind which means the "stride" is probably too large compared to "cols" -> do not waste memory
			// For example, calling binary not from ultimateText T-HOG classifier (text/nontext classification).
			CompVGenericInvokeCodeRawType(A->subType(), CompVMat::newObjAligned, &R_, A->rows(), A->cols());
		}
	}

	void(*CompVBitsLogicalNot_8u)(const uint8_t* Aptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Rstride)
		= CompVBitsLogicalNot_8u_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && A->isAlignedSSE() && R_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVBitsLogicalNot_8u = CompVBitsLogicalNot_8u_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVBitsLogicalNot_8u = CompVBitsLogicalNot_8u_Asm_X64_SSE2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && A->isAlignedNEON() && R_->isAlignedNEON()) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVBitsLogicalNot_8u = CompVBitsLogicalNot_8u_Intrin_NEON);
		COMPV_EXEC_IFDEF_ASM_ARM32(CompVBitsLogicalNot_8u = CompVBitsLogicalNot_8u_Asm_NEON32);
		COMPV_EXEC_IFDEF_ASM_ARM64(CompVBitsLogicalNot_8u = CompVBitsLogicalNot_8u_Asm_NEON64);
	}
#endif

	int planeId = 0;
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		CompVBitsLogicalNot_8u(
			A->ptr<const uint8_t>(ystart, 0, planeId), R_->ptr<uint8_t>(ystart, 0, planeId),
			static_cast<compv_uscalar_t>(A->cols(planeId)), static_cast<compv_uscalar_t>(yend - ystart),
			static_cast<compv_uscalar_t>(A->strideInBytes(planeId)), static_cast<compv_uscalar_t>(R_->strideInBytes(planeId))
		);
		return COMPV_ERROR_CODE_S_OK;
	};

	const int planesCount = static_cast<int>(A->planeCount());
	for (planeId = 0; planeId < planesCount; ++planeId) {
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			A->cols(planeId),
			A->rows(planeId),
			COMPV_BITS_AND_SAMPLES_PER_THREAD
		));
	}

	*R = R_;
	return COMPV_ERROR_CODE_S_OK;
}

// Vertical xor
// Line[(n)*stride] ^= Line[(n-1)*stride]
COMPV_ERROR_CODE CompVBits::logical_xorvt(const CompVMatPtr& A, CompVMatPtrPtr R)
{
	COMPV_CHECK_EXP_RETURN(!A || !R, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVMatPtr R_ = (A == *R) ? nullptr : *R; // This function doesn't allow R to be equal to A
	COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&R_, A));

	void(*CompVBitsLogicalXorVt_8u)(const uint8_t* Aptr, const uint8_t* A_Minus1_ptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Rstride)
		= CompVBitsLogicalXorVt_8u_C;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2) && A->isAlignedSSE() && R_->isAlignedSSE()) {
		COMPV_EXEC_IFDEF_INTRIN_X86(CompVBitsLogicalXorVt_8u = CompVBitsLogicalXorVt_8u_Intrin_SSE2);
		COMPV_EXEC_IFDEF_ASM_X64(CompVBitsLogicalXorVt_8u = CompVBitsLogicalXorVt_8u_Asm_X64_SSE2);
	}
#elif COMPV_ARCH_ARM
	if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && A->isAlignedNEON() && R_->isAlignedNEON()) {
		COMPV_EXEC_IFDEF_INTRIN_ARM(CompVBitsLogicalXorVt_8u = CompVBitsLogicalXorVt_8u_Intrin_NEON);
		COMPV_EXEC_IFDEF_ASM_ARM32(CompVBitsLogicalXorVt_8u = CompVBitsLogicalXorVt_8u_Asm_NEON32);
		COMPV_EXEC_IFDEF_ASM_ARM64(CompVBitsLogicalXorVt_8u = CompVBitsLogicalXorVt_8u_Asm_NEON64);
	}
#endif
	
	int planeId = 0;
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		size_t ystart_ = ystart;
		if (!ystart_) {
			COMPV_CHECK_CODE_RETURN(CompVMem::copy(R_->ptr<void>(0, 0, planeId), A->ptr<const void>(ystart_, 0, planeId), A->rowInBytes(planeId)));
			++ystart_;
		}
		CompVBitsLogicalXorVt_8u(
			A->ptr<const uint8_t>(ystart_, 0, planeId), A->ptr<const uint8_t>(ystart_ - 1, 0, planeId), R_->ptr<uint8_t>(ystart_, 0, planeId),
			static_cast<compv_uscalar_t>(A->cols(planeId)), static_cast<compv_uscalar_t>(yend - ystart_),
			static_cast<compv_uscalar_t>(A->strideInBytes(planeId)), static_cast<compv_uscalar_t>(R_->strideInBytes(planeId))
		);
		return COMPV_ERROR_CODE_S_OK;
	};

	const int planesCount = static_cast<int>(A->planeCount());
	for (planeId = 0; planeId < planesCount; ++planeId) {
#if 1
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			A->cols(planeId),
			A->rows(planeId),
			COMPV_BITS_XORVT_SAMPLES_PER_THREAD
		));
#else
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");
		COMPV_CHECK_CODE_RETURN(funcPtr(0, A->rows(planeId)));
#endif
	}

	*R = R_;
	return COMPV_ERROR_CODE_S_OK;
}

static void CompVBitsLogicalAnd_8u_C(const uint8_t* Aptr, const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			Rptr[i] = (Aptr[i] & Bptr[i]);
		}
		Rptr += Rstride;
		Aptr += Astride;
		Bptr += Bstride;
	}
}

static void CompVBitsLogicalNotAnd_8u_C(const uint8_t* Aptr, const uint8_t* Bptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Bstride, compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			Rptr[i] = (~Aptr[i] & Bptr[i]);
		}
		Rptr += Rstride;
		Aptr += Astride;
		Bptr += Bstride;
	}
}

static void CompVBitsLogicalNot_8u_C(const uint8_t* Aptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			Rptr[i] = ~Aptr[i];
		}
		Rptr += Rstride;
		Aptr += Astride;
	}
}

static void CompVBitsLogicalXorVt_8u_C(const uint8_t* Aptr, const uint8_t* A_Minus1_ptr, uint8_t* Rptr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t Astride, compv_uscalar_t Rstride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			Rptr[i] = Aptr[i] ^ A_Minus1_ptr[i];
		}
		Rptr += Rstride;
		Aptr += Astride;
		A_Minus1_ptr += Astride;
	}
}

COMPV_NAMESPACE_END()
