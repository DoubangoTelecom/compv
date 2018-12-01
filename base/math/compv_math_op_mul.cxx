/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_op_mul.h"
#include "compv/base/compv_generic_invoke.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"


#include "compv/base/math/intrin/x86/compv_math_op_mul_intrin_sse2.h"
#include "compv/base/math/intrin/arm/compv_math_op_mul_intrin_neon.h"

#define COMPV_MATH_OP_MUL_ABt_SAMPLES_PER_THREAD (50 * 50)

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVMathOpMulMulABt_32f32f32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* Aptr, COMPV_ALIGNED(SSE) const compv_float32_t* Bptr, COMPV_ALIGNED(SSE) compv_float32_t* Rptr, const compv_uscalar_t Bcols, const compv_uscalar_t Arows, const compv_uscalar_t Brows, COMPV_ALIGNED(SSE) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride, COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVMathOpMulMulABt_32f32f32f_Asm_X64_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* Aptr, COMPV_ALIGNED(AVX) const compv_float32_t* Bptr, COMPV_ALIGNED(AVX) compv_float32_t* Rptr, const compv_uscalar_t Bcols, const compv_uscalar_t Arows, const compv_uscalar_t Brows, COMPV_ALIGNED(AVX) const compv_uscalar_t Astride, COMPV_ALIGNED(AVX) const compv_uscalar_t Bstride, COMPV_ALIGNED(AVX) const compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVMathOpMulMulABt_32f32f32f_Asm_X64_FMA3_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* Aptr, COMPV_ALIGNED(AVX) const compv_float32_t* Bptr, COMPV_ALIGNED(AVX) compv_float32_t* Rptr, const compv_uscalar_t Bcols, const compv_uscalar_t Arows, const compv_uscalar_t Brows, COMPV_ALIGNED(AVX) const compv_uscalar_t Astride, COMPV_ALIGNED(AVX) const compv_uscalar_t Bstride, COMPV_ALIGNED(AVX) const compv_uscalar_t Rstride);
#endif /* COMPV_ASM && COMPV_ARCH_X64 */

#if COMPV_ASM && COMPV_ARCH_ARM32
COMPV_EXTERNC void CompVMathOpMulMulABt_32f32f32f_Asm_NEON32(COMPV_ALIGNED(SSE) const compv_float32_t* Aptr, COMPV_ALIGNED(SSE) const compv_float32_t* Bptr, COMPV_ALIGNED(SSE) compv_float32_t* Rptr, const compv_uscalar_t Bcols, const compv_uscalar_t Arows, const compv_uscalar_t Brows, COMPV_ALIGNED(SSE) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride, COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVMathOpMulMulABt_32f32f32f_Asm_FMA_NEON32(COMPV_ALIGNED(SSE) const compv_float32_t* Aptr, COMPV_ALIGNED(SSE) const compv_float32_t* Bptr, COMPV_ALIGNED(SSE) compv_float32_t* Rptr, const compv_uscalar_t Bcols, const compv_uscalar_t Arows, const compv_uscalar_t Brows, COMPV_ALIGNED(SSE) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride, COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

#if COMPV_ASM && COMPV_ARCH_ARM64
COMPV_EXTERNC void CompVMathOpMulMulABt_32f32f32f_Asm_NEON64(COMPV_ALIGNED(SSE) const compv_float32_t* Aptr, COMPV_ALIGNED(SSE) const compv_float32_t* Bptr, COMPV_ALIGNED(SSE) compv_float32_t* Rptr, const compv_uscalar_t Bcols, const compv_uscalar_t Arows, const compv_uscalar_t Brows, COMPV_ALIGNED(SSE) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride, COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVMathOpMulMulABt_32f32f32f_Asm_FMA_NEON64(COMPV_ALIGNED(SSE) const compv_float32_t* Aptr, COMPV_ALIGNED(SSE) const compv_float32_t* Bptr, COMPV_ALIGNED(SSE) compv_float32_t* Rptr, const compv_uscalar_t Bcols, const compv_uscalar_t Arows, const compv_uscalar_t Brows, COMPV_ALIGNED(SSE) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride, COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

template<typename T>
static void CompVMathOpMulMulABt(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtr& R);

COMPV_ERROR_CODE CompVMathOpMul::mulAB(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathOpMul::mulABt(const CompVMatPtr &A, const CompVMatPtr &B, CompVMatPtrPtr R)
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !R || A->subType() != B->subType() || A->cols() != B->cols(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr R_ = (*R == A || *R == B) ? nullptr : *R;
	CompVGenericInvokeCodeRawType(A->subType(), CompVMat::newObjAligned, &R_, A->rows(), B->rows());
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const CompVRectFloat32 Aroi = {
			0.f, // left
			static_cast<compv_float32_t>(ystart), // top
			static_cast<compv_float32_t>(A->cols()), // right (should be "cols-1", but bind will clip the value)
			static_cast<compv_float32_t>(yend - 1) // bottom
		};
		const CompVRectFloat32 Rroi = {
			0.f, // left
			static_cast<compv_float32_t>(ystart), // top
			static_cast<compv_float32_t>(R_->cols()), // right (should be "cols-1", but bind will clip the value)
			static_cast<compv_float32_t>(yend - 1) // bottom
		};
		CompVMatPtr Abind, Rbind;
		COMPV_CHECK_CODE_RETURN(A->bind(&Abind, Aroi));
		COMPV_CHECK_CODE_RETURN(R_->bind(&Rbind, Rroi));
		CompVGenericInvokeVoidRawType(Abind->subType(), CompVMathOpMulMulABt, Abind, B, Rbind);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		B->cols(),
		A->rows(),
		COMPV_MATH_OP_MUL_ABt_SAMPLES_PER_THREAD
	));
	*R = R_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathOpMul::mulAtA(const CompVMatPtr &A, CompVMatPtrPtr R)
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

template<typename T>
static void CompVMathOpMulMulABt(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtr& R)
{
	// Private function, no needed to check or imputs
	COMPV_ASSERT(A->isRawTypeMatch<T>() && B->isRawTypeMatch<T>() && R->isRawTypeMatch<T>());
	const T* Aptr = A->ptr<const T>();
	const T* Bptr = B->ptr<const T>();
	T* Rptr = R->ptr<T>();
	const compv_uscalar_t Bcols = static_cast<compv_uscalar_t>(B->cols());
	const compv_uscalar_t Arows = static_cast<compv_uscalar_t>(A->rows());
	const compv_uscalar_t Brows = static_cast<compv_uscalar_t>(B->rows());
	const compv_uscalar_t Astride = static_cast<compv_uscalar_t>(A->stride());
	const compv_uscalar_t Bstride = static_cast<compv_uscalar_t>(B->stride());
	const compv_uscalar_t Rstride = static_cast<compv_uscalar_t>(R->stride());

	if (std::is_same<T, compv_float32_t>::value) {
		void (*CompVMathOpMulMulABt_32f32f32f)(const compv_float32_t* Aptr, const compv_float32_t* Bptr, compv_float32_t* Rptr, const compv_uscalar_t Bcols, const compv_uscalar_t Arows, const compv_uscalar_t Brows, const compv_uscalar_t Astride, const compv_uscalar_t Bstride, const compv_uscalar_t Rstride)
			= nullptr;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && A->isAlignedSSE() && B->isAlignedSSE() && R->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathOpMulMulABt_32f32f32f = CompVMathOpMulMulABt_32f32f32f_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathOpMulMulABt_32f32f32f = CompVMathOpMulMulABt_32f32f32f_Asm_X64_SSE2);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX) && A->isAlignedAVX() && B->isAlignedAVX() && R->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathOpMulMulABt_32f32f32f = CompVMathOpMulMulABt_32f32f32f_Asm_X64_AVX);
			if (CompVCpu::isEnabled(kCpuFlagFMA3)) {
				COMPV_EXEC_IFDEF_ASM_X64(CompVMathOpMulMulABt_32f32f32f = CompVMathOpMulMulABt_32f32f32f_Asm_X64_FMA3_AVX);
			}
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && A->isAlignedNEON() && B->isAlignedNEON() && R->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathOpMulMulABt_32f32f32f = CompVMathOpMulMulABt_32f32f32f_Intrin_NEON);
			COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathOpMulMulABt_32f32f32f = CompVMathOpMulMulABt_32f32f32f_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathOpMulMulABt_32f32f32f = CompVMathOpMulMulABt_32f32f32f_Asm_NEON64);
			if (CompVCpu::isEnabled(kCpuFlagARM_NEON_FMA)) {
				COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathOpMulMulABt_32f32f32f = CompVMathOpMulMulABt_32f32f32f_Asm_FMA_NEON32);
				COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathOpMulMulABt_32f32f32f = CompVMathOpMulMulABt_32f32f32f_Asm_FMA_NEON64);
			}
		}
#endif
		if (CompVMathOpMulMulABt_32f32f32f) {
			CompVMathOpMulMulABt_32f32f32f(reinterpret_cast<const compv_float32_t*>(Aptr), reinterpret_cast<const compv_float32_t*>(Bptr), reinterpret_cast<compv_float32_t*>(Rptr), Bcols, Arows, Brows, Astride, Bstride, Rstride);
			return;
		}
	}

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	const compv_uscalar_t Bcols16 = Bcols & -16;
	const compv_uscalar_t Bcols4 = Bcols & -4;
	compv_uscalar_t k;
	for (compv_uscalar_t i = 0; i < Arows; ++i) {
		const T* B0ptr = Bptr;
		for (compv_uscalar_t j = 0; j < Brows; ++j) {
			T vec16[16] = { 0 }; // SIMD-way to make sure we'll end with same MD5 as SSE/AVX/NEON code (unit-test module)
			for (k = 0; k < Bcols16; k += 16) {
				vec16[0] += (Aptr[k + 0] * B0ptr[k + 0]);
				vec16[1] += (Aptr[k + 1] * B0ptr[k + 1]);
				vec16[2] += (Aptr[k + 2] * B0ptr[k + 2]);
				vec16[3] += (Aptr[k + 3] * B0ptr[k + 3]);
				vec16[4] += (Aptr[k + 4] * B0ptr[k + 4]);
				vec16[5] += (Aptr[k + 5] * B0ptr[k + 5]);
				vec16[6] += (Aptr[k + 6] * B0ptr[k + 6]);
				vec16[7] += (Aptr[k + 7] * B0ptr[k + 7]);
				vec16[8] += (Aptr[k + 8] * B0ptr[k + 8]);
				vec16[9] += (Aptr[k + 9] * B0ptr[k + 9]);
				vec16[10] += (Aptr[k + 10] * B0ptr[k + 10]);
				vec16[11] += (Aptr[k + 11] * B0ptr[k + 11]);
				vec16[12] += (Aptr[k + 12] * B0ptr[k + 12]);
				vec16[13] += (Aptr[k + 13] * B0ptr[k + 13]);
				vec16[14] += (Aptr[k + 14] * B0ptr[k + 14]);
				vec16[15] += (Aptr[k + 15] * B0ptr[k + 15]);
			}
			for (; k < Bcols4; k += 4) {
				vec16[0] += (Aptr[k + 0] * B0ptr[k + 0]);
				vec16[1] += (Aptr[k + 1] * B0ptr[k + 1]);
				vec16[2] += (Aptr[k + 2] * B0ptr[k + 2]);
				vec16[3] += (Aptr[k + 3] * B0ptr[k + 3]);
			}
			vec16[0] += vec16[4];
			vec16[1] += vec16[5];
			vec16[2] += vec16[6];
			vec16[3] += vec16[7];
			vec16[8] += vec16[12];
			vec16[9] += vec16[13];
			vec16[10] += vec16[14];
			vec16[11] += vec16[15];
			vec16[0] += vec16[8];
			vec16[1] += vec16[9];
			vec16[2] += vec16[10];
			vec16[3] += vec16[11];
			vec16[0] += vec16[2];
			vec16[1] += vec16[3];
			vec16[0] += vec16[1];
			for (; k < Bcols; k += 1) {
				vec16[0] += Aptr[k] * B0ptr[k];
			}
			Rptr[j] = vec16[0];
			B0ptr += Bstride;
		}
		Aptr += Astride;
		Rptr += Rstride;
	}
}

COMPV_NAMESPACE_END()
