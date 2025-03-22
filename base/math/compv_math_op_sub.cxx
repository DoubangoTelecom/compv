/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_op_sub.h"
#include "compv/base/compv_generic_invoke.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/math/intrin/x86/compv_math_op_sub_intrin_sse2.h"
#include "compv/base/math/intrin/x86/compv_math_op_sub_intrin_avx.h"
#include "compv/base/math/intrin/arm/compv_math_op_sub_intrin_neon.h"

#define COMPV_MATH_OP_SUB_SAMPLES_PER_THREAD	(50 * 50)
#define COMPV_MATH_OP_SUBMUL_SAMPLES_PER_THREAD (20 * 20)

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVMathOpSubSub_32f32f32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* Aptr, COMPV_ALIGNED(SSE) const compv_float32_t* Bptr, COMPV_ALIGNED(SSE) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride, COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVMathOpSubSubMul_32f32f32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* Aptr, const compv_float32_t* subVal1, const compv_float32_t* mulVal1, COMPV_ALIGNED(SSE) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVMathOpSubSubVal_32f32f32f_Asm_X64_SSE2(COMPV_ALIGNED(SSE) const compv_float32_t* Aptr, const compv_float32_t* subVal1, COMPV_ALIGNED(SSE) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride);

COMPV_EXTERNC void CompVMathOpSubSub_32f32f32f_Asm_X64_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* Aptr, COMPV_ALIGNED(AVX) const compv_float32_t* Bptr, COMPV_ALIGNED(AVX) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(AVX) const compv_uscalar_t Astride, COMPV_ALIGNED(AVX) const compv_uscalar_t Bstride, COMPV_ALIGNED(AVX) const compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVMathOpSubSubMul_32f32f32f_Asm_X64_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* Aptr, const compv_float32_t* subVal1, const compv_float32_t* mulVal1, COMPV_ALIGNED(AVX) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(AVX) const compv_uscalar_t Astride, COMPV_ALIGNED(AVX) const compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVMathOpSubSubVal_32f32f32f_Asm_X64_AVX(COMPV_ALIGNED(AVX) const compv_float32_t* Aptr, const compv_float32_t* subVal1, COMPV_ALIGNED(AVX) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(AVX) const compv_uscalar_t Astride, COMPV_ALIGNED(AVX) const compv_uscalar_t Rstride);
#endif /* COMPV_ASM && COMPV_ARCH_X64 */

#if COMPV_ASM && COMPV_ARCH_ARM32
COMPV_EXTERNC void CompVMathOpSubSub_32f32f32f_Asm_NEON32(COMPV_ALIGNED(NEON) const compv_float32_t* Aptr, COMPV_ALIGNED(NEON) const compv_float32_t* Bptr, COMPV_ALIGNED(NEON) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride, COMPV_ALIGNED(NEON) const compv_uscalar_t Rstride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

#if COMPV_ASM && COMPV_ARCH_ARM64
COMPV_EXTERNC void CompVMathOpSubSub_32f32f32f_Asm_NEON64(COMPV_ALIGNED(NEON) const compv_float32_t* Aptr, COMPV_ALIGNED(NEON) const compv_float32_t* Bptr, COMPV_ALIGNED(NEON) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride, COMPV_ALIGNED(NEON) const compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVMathOpSubSubMul_32f32f32f_Asm_NEON64(COMPV_ALIGNED(NEON) const compv_float32_t* Aptr, const compv_float32_t* subVal1, const compv_float32_t* mulVal1, COMPV_ALIGNED(NEON) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t Astride, COMPV_ALIGNED(NEON) const compv_uscalar_t Rstride);
COMPV_EXTERNC void CompVMathOpSubSubVal_32f32f32f_Asm_NEON64(COMPV_ALIGNED(NEON) const compv_float32_t* Aptr, const compv_float32_t* subVal1, COMPV_ALIGNED(NEON) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(NEON) const compv_uscalar_t Astride, COMPV_ALIGNED(NEON) const compv_uscalar_t Rstride);
#endif /* COMPV_ASM && COMPV_ARCH_ARM64 */

template<typename T>
static void CompVMathOpSubSub(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtr& R);

template<typename T>
static void CompVMathOpSubSubVal(const CompVMatPtr& A, const double& subVal_, CompVMatPtr& R);

template<typename T>
static void CompVMathOpSubSubMul(const CompVMatPtr& A, const double& subVal, const double& mulVal, CompVMatPtr& R);

// R = A - B
COMPV_ERROR_CODE CompVMathOpSub::sub(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtrPtr R, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!A || !B || !R || A->subType() != B->subType() || A->cols() != B->cols() || A->rows() != B->rows(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr R_ = *R;
	if (R_ != A && R_ != B) { // This function allows having R = A/B
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&R_, A));
	}
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const CompVRectFloat32 roi = {
			0.f, // left
			static_cast<compv_float32_t>(ystart), // top
			static_cast<compv_float32_t>(A->cols() - 1), // right
			static_cast<compv_float32_t>(yend - 1) // bottom
		};
		CompVMatPtr Abind, Bbind, Rbind;
		COMPV_CHECK_CODE_RETURN(A->bind(&Abind, roi));
		COMPV_CHECK_CODE_RETURN(B->bind(&Bbind, roi));
		COMPV_CHECK_CODE_RETURN(R_->bind(&Rbind, roi));
		CompVGenericInvokeVoidRawType(Abind->subType(), CompVMathOpSubSub, Abind, Bbind, Rbind);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		A->cols(),
		A->rows(),
		enforceSingleThread ? SIZE_MAX : COMPV_MATH_OP_SUB_SAMPLES_PER_THREAD
	));
	*R = R_;
	return COMPV_ERROR_CODE_S_OK;
}

// R = A - subVal
COMPV_ERROR_CODE CompVMathOpSub::sub(const CompVMatPtr& A, const double& subVal, CompVMatPtrPtr R, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!A || !R, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr R_ = *R;
	if (R_ != A) { // This function allows having R = A
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&R_, A));
	}
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const CompVRectFloat32 roi = {
			0.f, // left
			static_cast<compv_float32_t>(ystart), // top
			static_cast<compv_float32_t>(A->cols() - 1), // right
			static_cast<compv_float32_t>(yend - 1) // bottom
		};
		CompVMatPtr Abind, Bbind, Rbind;
		COMPV_CHECK_CODE_RETURN(A->bind(&Abind, roi));
		COMPV_CHECK_CODE_RETURN(R_->bind(&Rbind, roi));
		CompVGenericInvokeVoidRawType(Abind->subType(), CompVMathOpSubSubVal, Abind, subVal, Rbind);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		A->cols(),
		A->rows(),
		enforceSingleThread ? SIZE_MAX : COMPV_MATH_OP_SUB_SAMPLES_PER_THREAD
	));
	*R = R_;
	return COMPV_ERROR_CODE_S_OK;
}

// R = (A-subVal) * mulVal
COMPV_ERROR_CODE CompVMathOpSub::subMul(const CompVMatPtr& A, const double& subVal, const double& mulVal, CompVMatPtrPtr R, const bool enforceSingleThread COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!A || !R, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVMatPtr R_ = *R;
	if (R_ != A) { // This function allows having R = A/B
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&R_, A));
	}
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const CompVRectFloat32 roi = {
			0.f, // left
			static_cast<compv_float32_t>(ystart), // top
			static_cast<compv_float32_t>(A->cols() - 1), // right
			static_cast<compv_float32_t>(yend - 1) // bottom
		};
		CompVMatPtr Abind, Bbind, Rbind;
		COMPV_CHECK_CODE_RETURN(A->bind(&Abind, roi));
		COMPV_CHECK_CODE_RETURN(R_->bind(&Rbind, roi));
		CompVGenericInvokeVoidRawType(Abind->subType(), CompVMathOpSubSubMul, Abind, subVal, mulVal, Rbind);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		A->cols(),
		A->rows(),
		enforceSingleThread ? SIZE_MAX : COMPV_MATH_OP_SUBMUL_SAMPLES_PER_THREAD
	));
	*R = R_;
	return COMPV_ERROR_CODE_S_OK;
}

template<typename T>
static void CompVMathOpSubSub(const CompVMatPtr& A, const CompVMatPtr& B, CompVMatPtr& R)
{
	// Private function, no needed to check or imputs
	COMPV_ASSERT(A->isRawTypeMatch<T>() && B->isRawTypeMatch<T>() && R->isRawTypeMatch<T>());
	const T* Aptr = A->ptr<const T>(); 
	const T* Bptr = B->ptr<const T>(); 
	T* Rptr = R->ptr<T>();
	const compv_uscalar_t width = static_cast<compv_uscalar_t>(A->cols());
	const compv_uscalar_t height = static_cast<compv_uscalar_t>(A->rows());
	const compv_uscalar_t Astride = static_cast<compv_uscalar_t>(A->stride());
	const compv_uscalar_t Bstride = static_cast<compv_uscalar_t>(B->stride());
	const compv_uscalar_t Rstride = static_cast<compv_uscalar_t>(R->stride());

	if (std::is_same<T, compv_float32_t>::value) {
		void(*CompVMathOpSubSub_32f32f32f)(COMPV_ALIGNED(SSE) const compv_float32_t* Aptr, COMPV_ALIGNED(SSE) const compv_float32_t* Bptr, COMPV_ALIGNED(SSE) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Bstride, COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride)
			= nullptr;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && A->isAlignedSSE() && B->isAlignedSSE() && R->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathOpSubSub_32f32f32f = CompVMathOpSubSub_32f32f32f_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathOpSubSub_32f32f32f = CompVMathOpSubSub_32f32f32f_Asm_X64_SSE2);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX) && A->isAlignedAVX() && B->isAlignedAVX() && R->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathOpSubSub_32f32f32f = CompVMathOpSubSub_32f32f32f_Asm_X64_AVX);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && A->isAlignedNEON() && B->isAlignedNEON() && R->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathOpSubSub_32f32f32f = CompVMathOpSubSub_32f32f32f_Intrin_NEON);
			COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathOpSubSub_32f32f32f = CompVMathOpSubSub_32f32f32f_Asm_NEON32);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathOpSubSub_32f32f32f = CompVMathOpSubSub_32f32f32f_Asm_NEON64);
		}
#endif
		if (CompVMathOpSubSub_32f32f32f) {
			CompVMathOpSubSub_32f32f32f(reinterpret_cast<const compv_float32_t*>(Aptr), reinterpret_cast<const compv_float32_t*>(Bptr), reinterpret_cast<compv_float32_t*>(Rptr), width, height, Astride, Bstride, Rstride);
			return;
		}
	}

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			Rptr[i] = Aptr[i] - Bptr[i];
		}
		Aptr += Astride;
		Bptr += Bstride;
		Rptr += Rstride;
	}
}

template<typename T>
static void CompVMathOpSubSubVal(const CompVMatPtr& A, const double& subVal_, CompVMatPtr& R)
{
	// Private function, no needed to check or imputs
	COMPV_ASSERT(A->isRawTypeMatch<T>() && R->isRawTypeMatch<T>());
	const T* Aptr = A->ptr<const T>();
	T* Rptr = R->ptr<T>();
	const compv_uscalar_t width = static_cast<compv_uscalar_t>(A->cols());
	const compv_uscalar_t height = static_cast<compv_uscalar_t>(A->rows());
	const compv_uscalar_t Astride = static_cast<compv_uscalar_t>(A->stride());
	const compv_uscalar_t Rstride = static_cast<compv_uscalar_t>(R->stride());
	const T subVal = static_cast<T>(subVal_);

	if (std::is_same<T, compv_float32_t>::value) {
		void(*CompVMathOpSubSubVal_32f32f32f)(COMPV_ALIGNED(SSE) const compv_float32_t* Aptr, const compv_float32_t* subVal1, COMPV_ALIGNED(SSE) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride)
			= nullptr;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && A->isAlignedSSE() && R->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathOpSubSubVal_32f32f32f = CompVMathOpSubSubVal_32f32f32f_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathOpSubSubVal_32f32f32f = CompVMathOpSubSubVal_32f32f32f_Asm_X64_SSE2);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX) && A->isAlignedAVX() && R->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathOpSubSubVal_32f32f32f = CompVMathOpSubSubVal_32f32f32f_Asm_X64_AVX);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && A->isAlignedNEON() && R->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathOpSubSubVal_32f32f32f = CompVMathOpSubSubVal_32f32f32f_Intrin_NEON);
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathOpSubSubVal_32f32f32f = CompVMathOpSubSubVal_32f32f32f_Asm_NEON64);
		}
#endif
		if (CompVMathOpSubSubVal_32f32f32f) {
			CompVMathOpSubSubVal_32f32f32f(
				reinterpret_cast<const compv_float32_t*>(Aptr), 
				reinterpret_cast<const compv_float32_t*>(&subVal), 
				reinterpret_cast<compv_float32_t*>(Rptr), 
				width, height, Astride, Rstride
			);
			return;
		}
	}

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			Rptr[i] = Aptr[i] - subVal;
		}
		Aptr += Astride;
		Rptr += Rstride;
	}
}

template<typename T>
static void CompVMathOpSubSubMul(const CompVMatPtr& A, const double& subVal_, const double& mulVal_, CompVMatPtr& R)
{
	// Private function, no needed to check or imputs
	COMPV_ASSERT(A->isRawTypeMatch<T>() && R->isRawTypeMatch<T>());
	const T* Aptr = A->ptr<const T>();
	T* Rptr = R->ptr<T>();
	const compv_uscalar_t width = static_cast<compv_uscalar_t>(A->cols());
	const compv_uscalar_t height = static_cast<compv_uscalar_t>(A->rows());
	const compv_uscalar_t Astride = static_cast<compv_uscalar_t>(A->stride());
	const compv_uscalar_t Rstride = static_cast<compv_uscalar_t>(R->stride());
	const T subVal = static_cast<T>(subVal_);
	const T mulVal = static_cast<T>(mulVal_);

	if (std::is_same<T, compv_float32_t>::value) {
		void(*CompVMathOpSubSubMul_32f32f32f)(COMPV_ALIGNED(SSE) const compv_float32_t* Aptr, const compv_float32_t* subVal1, const compv_float32_t* mulVal1, COMPV_ALIGNED(SSE) compv_float32_t* Rptr, const compv_uscalar_t width, const compv_uscalar_t height, COMPV_ALIGNED(SSE) const compv_uscalar_t Astride, COMPV_ALIGNED(SSE) const compv_uscalar_t Rstride)
			= nullptr;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && A->isAlignedSSE() && R->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathOpSubSubMul_32f32f32f = CompVMathOpSubSubMul_32f32f32f_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathOpSubSubMul_32f32f32f = CompVMathOpSubSubMul_32f32f32f_Asm_X64_SSE2);
		}
		if (CompVCpu::isEnabled(kCpuFlagAVX) && A->isAlignedAVX() && R->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(CompVMathOpSubSubMul_32f32f32f = CompVMathOpSubSubMul_32f32f32f_Intrin_AVX);
			COMPV_EXEC_IFDEF_ASM_X64(CompVMathOpSubSubMul_32f32f32f = CompVMathOpSubSubMul_32f32f32f_Asm_X64_AVX);
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && A->isAlignedNEON() && R->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM(CompVMathOpSubSubMul_32f32f32f = CompVMathOpSubSubMul_32f32f32f_Intrin_NEON);
			//COMPV_EXEC_IFDEF_ASM_ARM32(CompVMathOpSubSubMul_32f32f32f = CompVMathOpSubSubMul_32f32f32f_Asm_NEON32); // Not used on ARM32 (Tensorflow)
			COMPV_EXEC_IFDEF_ASM_ARM64(CompVMathOpSubSubMul_32f32f32f = CompVMathOpSubSubMul_32f32f32f_Asm_NEON64); // used on Jetson to norm input data for NVIDIA TensorRT
		}
#endif
		if (CompVMathOpSubSubMul_32f32f32f) {
			CompVMathOpSubSubMul_32f32f32f(
				reinterpret_cast<const compv_float32_t*>(Aptr), 
				reinterpret_cast<const compv_float32_t*>(&subVal), 
				reinterpret_cast<const compv_float32_t*>(&mulVal),
				reinterpret_cast<compv_float32_t*>(Rptr),
				width, height, Astride, Rstride
			);
			return;
		}
	}

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			Rptr[i] = (Aptr[i] - subVal) * mulVal;
		}
		Aptr += Astride;
		Rptr += Rstride;
	}
}

COMPV_NAMESPACE_END()
