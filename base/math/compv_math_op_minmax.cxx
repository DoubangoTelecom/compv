/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_op_minmax.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_generic_invoke.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/math/intrin/x86/compv_math_op_minmax_intrin_sse2.h"
#include "compv/base/math/intrin/arm/compv_math_op_minmax_intrin_neon.h"

#define COMPV_MATH_OP_MINMAX_SAMPLES_PER_THREAD (50 * 50)

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM && COMPV_ARCH_X64
#endif /* COMPV_ASM && COMPV_ARCH_X64 */

#if COMPV_ASM && COMPV_ARCH_ARM32
#endif /* COMPV_ASM && COMPV_ARCH_ARM32 */

#if COMPV_ASM && COMPV_ARCH_ARM64
#endif /* COMPV_ASM && COMPV_ARCH_ARM64 */

template<typename T>
static void OpMinMax_C(const T* APtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, T* min1, T* max1);
template<typename T>
static void OpMin_C(const T* APtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, T* min1);
template<typename T>
static void OpMax_C(const T* APtr, compv_uscalar_t Awidth, compv_uscalar_t Aheight, compv_uscalar_t Astride, const T* b1, T* RPtr);
template<typename T>
static void OpMinMax(const CompVMatPtr& A, double& minn, double& maxx);
template<typename T>
static void OpMin(const CompVMatPtr& A, double& minn);
template<typename T>
static void OpMax(const CompVMatPtr& A, const double& b, CompVMatPtr& R);

COMPV_ERROR_CODE CompVMathOpMinMax::minMax(const CompVMatPtr &A, double& minn, double& maxx)
{
	COMPV_CHECK_EXP_RETURN(!A, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const size_t rows = A->rows();
	const size_t threadsCount = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(1, rows, 1);
	minn = std::numeric_limits<double>::max();
	maxx = std::numeric_limits<double>::lowest();
	std::vector<double> mt_maxes_all(threadsCount - 1, maxx);
	std::vector<double> mt_mins_all(threadsCount - 1, minn);
	auto funcPtr = [&](const size_t ystart, const size_t yend, const size_t threadIdx) -> COMPV_ERROR_CODE {
		COMPV_ASSERT(threadIdx < threadsCount);
		double& mt_max = threadIdx ? mt_maxes_all[threadIdx - 1] : maxx;
		double& mt_min = threadIdx ? mt_mins_all[threadIdx - 1] : minn;
		const CompVRectFloat32 roi = {
			0.f, // left
			static_cast<compv_float32_t>(ystart), // top
			static_cast<compv_float32_t>(A->cols() - 1), // right
			static_cast<compv_float32_t>(yend - 1) // bottom
		};
		CompVMatPtr Abind, Bbind, Rbind;
		COMPV_CHECK_CODE_RETURN(A->bind(&Abind, roi));
		CompVGenericInvokeVoidRawType(Abind->subType(), OpMinMax, Abind, mt_min, mt_max);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		rows,
		threadsCount
	));

	const size_t mt_count = mt_maxes_all.size();
	for (size_t i = 0; i < mt_count; ++i) {
		maxx = COMPV_MATH_MAX(maxx, mt_maxes_all[i]);
		minn = COMPV_MATH_MIN(minn, mt_mins_all[i]);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathOpMinMax::minn(const CompVMatPtr &A, double& minn)
{
	COMPV_CHECK_EXP_RETURN(!A, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const size_t rows = A->rows();
	const size_t threadsCount = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(1, rows, 1);
	minn = std::numeric_limits<double>::max();
	std::vector<double> mt_mins_all(threadsCount - 1, minn);
	auto funcPtr = [&](const size_t ystart, const size_t yend, const size_t threadIdx) -> COMPV_ERROR_CODE {
		COMPV_ASSERT(threadIdx < threadsCount);
		double& mt_min = threadIdx ? mt_mins_all[threadIdx - 1] : minn;
		const CompVRectFloat32 roi = {
			0.f, // left
			static_cast<compv_float32_t>(ystart), // top
			static_cast<compv_float32_t>(A->cols() - 1), // right
			static_cast<compv_float32_t>(yend - 1) // bottom
		};
		CompVMatPtr Abind;
		COMPV_CHECK_CODE_RETURN(A->bind(&Abind, roi));
		CompVGenericInvokeVoidRawType(Abind->subType(), OpMin, Abind, mt_min);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		rows,
		threadsCount
	));

	for (auto it : mt_mins_all) {
		minn = COMPV_MATH_MIN(minn, it);
	}
	return COMPV_ERROR_CODE_S_OK;
}

// R = max(A, b)
COMPV_ERROR_CODE CompVMathOpMinMax::maxx(const CompVMatPtr &A, const double& b, CompVMatPtrPtr R)
{
	COMPV_CHECK_EXP_RETURN(!A || !R, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	const size_t rows = A->rows();
	const size_t threadsCount = CompVThreadDispatcher::guessNumThreadsDividingAcrossY(1, rows, 1);
	if (*R != A) {
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(R, A));
	}
	auto funcPtr = [&](const size_t ystart, const size_t yend, const size_t threadIdx) -> COMPV_ERROR_CODE {
		COMPV_ASSERT(threadIdx < threadsCount);
		const CompVRectFloat32 roi = {
			0.f, // left
			static_cast<compv_float32_t>(ystart), // top
			static_cast<compv_float32_t>(A->cols() - 1), // right
			static_cast<compv_float32_t>(yend - 1) // bottom
		};
		CompVMatPtr Abind, Rbind;
		COMPV_CHECK_CODE_RETURN(A->bind(&Abind, roi));
		COMPV_CHECK_CODE_RETURN((*R)->bind(&Rbind, roi));
		CompVGenericInvokeVoidRawType(Abind->subType(), OpMax, Abind, b, Rbind);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		rows,
		threadsCount
	));
	return COMPV_ERROR_CODE_S_OK;
}

template<typename T>
static void OpMinMax_C(const T* APtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, T* min1, T* max1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	T& minn = *min1;
	T& maxx = *max1;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			const T& vv = APtr[i];
			minn = COMPV_MATH_MIN(minn, vv);
			maxx = COMPV_MATH_MAX(maxx, vv);
		}
		APtr += stride;
	}
}
static void OpMinMax_32f_C(const compv_float32_t* APtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, compv_float32_t* min1, compv_float32_t* max1) {
	OpMinMax_C<compv_float32_t>(APtr, width, height, stride, min1, max1);
}

template<typename T>
static void OpMinMax(const CompVMatPtr& A, double& minn, double& maxx)
{
	// Private function, no needed to check or imputs
	COMPV_ASSERT(A->isRawTypeMatch<T>() || (A->subType() == COMPV_SUBTYPE_PIXELS_Y && A->elmtInBytes() == sizeof(T)));

	const T* Aptr = A->ptr<const T>();
	const compv_uscalar_t width = static_cast<compv_uscalar_t>(A->cols());
	const compv_uscalar_t height = static_cast<compv_uscalar_t>(A->rows());
	const compv_uscalar_t Astride = static_cast<compv_uscalar_t>(A->stride());

	T minn_ = std::numeric_limits<T>::max();
	T maxx_ = std::numeric_limits<T>::lowest();

	if (std::is_same<T, compv_float32_t>::value) {
		void(*OpMinMax_32f)(const compv_float32_t* APtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, compv_float32_t* min1, compv_float32_t* max1)
			= OpMinMax_32f_C;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && A->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86((OpMinMax_32f = CompVMathOpMinMax_32f_Intrin_SSE2));
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && A->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM((OpMinMax_32f = CompVMathOpMinMax_32f_Intrin_NEON));
		}
#endif
		OpMinMax_32f(reinterpret_cast<const compv_float32_t*>(Aptr), width, height, Astride, reinterpret_cast<compv_float32_t*>(&minn_), reinterpret_cast<compv_float32_t*>(&maxx_));
	}
	else {
		OpMinMax_C<T>(Aptr, width, height, Astride, &minn_, &maxx_);
	}

	minn = static_cast<double>(minn_);
	maxx = static_cast<double>(maxx_);
}

// min1 = reduce_min(APtr)
template<typename T>
static void OpMin_C(const T* APtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, T* min1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	T& minn = *min1;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			const T& vv = APtr[i];
			minn = COMPV_MATH_MIN(minn, vv);
		}
		APtr += stride;
	}
}

template<typename T>
static void OpMin(const CompVMatPtr& A, double& minn)
{
	// Private function, no needed to check or imputs
	COMPV_ASSERT(A->isRawTypeMatch<T>() || (A->subType() == COMPV_SUBTYPE_PIXELS_Y && A->elmtInBytes() == sizeof(T)));

	const T* Aptr = A->ptr<const T>();
	const compv_uscalar_t width = static_cast<compv_uscalar_t>(A->cols());
	const compv_uscalar_t height = static_cast<compv_uscalar_t>(A->rows());
	const compv_uscalar_t Astride = static_cast<compv_uscalar_t>(A->stride());

	T minn_ = std::numeric_limits<T>::max();

	if (std::is_same<T, uint8_t>::value) {
		void(*OpMin_8u)(const uint8_t* APtr, compv_uscalar_t width, compv_uscalar_t height, compv_uscalar_t stride, uint8_t* min1)
			= OpMin_C<uint8_t>;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && A->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86((OpMin_8u = CompVMathOpMin_8u_Intrin_SSE2));
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && A->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM((OpMin_8u = CompVMathOpMin_8u_Intrin_NEON));
		}
#endif
		OpMin_8u(reinterpret_cast<const uint8_t*>(Aptr), width, height, Astride, reinterpret_cast<uint8_t*>(&minn_));
	}
	else {
		OpMin_C<T>(Aptr, width, height, Astride, &minn_);
	}

	minn = static_cast<double>(minn_);
}

// R = max(A, b)
template<typename T>
static void OpMax_C(const T* APtr, compv_uscalar_t Awidth, compv_uscalar_t Aheight, compv_uscalar_t Astride, const T* b1, T* RPtr)
{
	const T& b = *b1;
	for (compv_uscalar_t j = 0; j < Aheight; ++j) {
		for (compv_uscalar_t i = 0; i < Awidth; ++i) {
			const T& vv = APtr[i];
			RPtr[i] = COMPV_MATH_MAX(b, vv);
		}
		APtr += Astride;
		RPtr += Astride;
	}
}

// R = max(A, b)
template<typename T>
static void OpMax(const CompVMatPtr& A, const double& b, CompVMatPtr& R)
{
	const compv_uscalar_t width = static_cast<compv_uscalar_t>(A->cols());
	const compv_uscalar_t height = static_cast<compv_uscalar_t>(A->rows());
	const compv_uscalar_t stride = static_cast<compv_uscalar_t>(A->stride());

	if (std::is_same<T, compv_float32_t>::value) {
		void(*OpMax_32f)(const compv_float32_t* APtr, compv_uscalar_t Awidth, compv_uscalar_t Aheight, compv_uscalar_t Astride, const compv_float32_t* b1, compv_float32_t* RPtr)
			= OpMax_C<float>;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagSSE2) && A->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86((OpMax_32f = CompVMathOpMax_32f_Intrin_SSE2));
		}
#elif COMPV_ARCH_ARM
		if (CompVCpu::isEnabled(kCpuFlagARM_NEON) && A->isAlignedNEON()) {
			COMPV_EXEC_IFDEF_INTRIN_ARM((OpMax_32f = CompVMathOpMax_8u_Intrin_NEON));
		}
#endif
		const compv_float32_t b1 = static_cast<compv_float32_t>(b);
		OpMax_32f(A->ptr<const compv_float32_t>(), width, height, stride, &b1, R->ptr<compv_float32_t>());
	}
	else {
		const T b1 = static_cast<T>(b);
		OpMax_C<T>(A->ptr<const T>(), width, height, stride, &b1, R->ptr<T>());
	}
}

COMPV_NAMESPACE_END()
