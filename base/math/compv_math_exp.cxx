/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/math/compv_math_exp.h"
#include "compv/base/math/compv_math.h"
#include "compv/base/compv_generic_invoke.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"

#include "compv/base/math/intrin/x86/compv_math_exp_intrin_sse2.h"
#include "compv/base/math/intrin/x86/compv_math_exp_intrin_avx2.h"

#define COMPV_THIS_CLASSNAME	"CompVMathExp"

// Part of the code is based on https://github.com/herumi/fmath

COMPV_NAMESPACE_BEGIN()

#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVMathExpExp_minpack4_64f64f_Asm_X64_AVX2(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const uint64_t* lut64u, const uint64_t* var64u, const compv_float64_t* var64f);
#endif /* #if COMPV_ASM && COMPV_ARCH_X64 */

bool CompVMathExp::s_bInitialized = false;
COMPV_ALIGN_DEFAULT() uint64_t CompVMathExp::s_arrayVars64u[2] = { //!\\ MUST NOT CHANGE THE INDEXES (add at the bottom but never remove or swap)
	2047ULL, /* [0]: mask(c.sbit = 11) */
	2095104ULL /* [1]: c.adj = (1UL << (sbit + 10)) - (1UL << sbit) */
};
COMPV_ALIGN_DEFAULT() compv_float64_t CompVMathExp::s_arrayVars6f4[8] = { //!\\ MUST NOT CHANGE THE INDEXES (add at the bottom but never remove or swap)
	6755399441055744.0, /* [0]: 3ULL << 51 */
	2954.6394437405970, /* [1]: c.a */
	0.00033845077175778578, /* [2]: c.ra */
	1.0000000000000000, /* [3]: c.C1[0] */
	0.16666666685227835, /* [4]: c.C2[0] */
	3.0000000027955394, /* [5]: c.C3[0] */
	-708.39641853226408, /* [6]: ExpMin */
	709.78271289338397 /* [7]: ExpMax */
};
static COMPV_ALIGN_DEFAULT() compv_float64_t s_arrayVars6f4[2];
COMPV_ALIGN_DEFAULT() uint64_t CompVMathExp::s_arrayLut64u[2048 /* 1UL << sbit[11] */] = { 0ULL };

template<typename T>
static void CompVMathExpExp_C(const T* ptrIn, T* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found");
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			ptrOut[i] = std::exp(ptrIn[i]);
		}
		ptrIn += stride;
		ptrOut += stride;
	}
}

// Source code based on https://github.com/herumi/fmath
static void CompVMathExpExp_minpack1_64f64f_C(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const uint64_t* lut64u, const uint64_t* var64u, const compv_float64_t* var64f)
{
#if COMPV_ARCH_X86
	if (width > 3) {
#elif COMPV_ARCH_ARM
	if (width > 1) {
#endif
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found");
	}
	static const uint64_t mask = var64u[0]; /* mask(c.sbit = 11) */
	static const uint64_t cadj = var64u[1]; /* c.adj = (1UL << (sbit + 10)) - (1UL << sbit) */

	static const compv_float64_t b = var64f[0]; /* 3ULL << 51 */
	static const compv_float64_t ca = var64f[1]; /* c.a */
	static const compv_float64_t cra = var64f[2]; /* c.ra */
	static const compv_float64_t C10 = var64f[3]; /* c.C1[0] */
	static const compv_float64_t C20 = var64f[4]; /* c.C2[0] */
	static const compv_float64_t C30 = var64f[5]; /* c.C3[0] */
	static const compv_float64_t ExpMin = var64f[6]; /* ExpMin */
	static const compv_float64_t ExpMax = var64f[7]; /* ExpMax */
	union di {
		compv_float64_t d;
		uint64_t i;
	};
	for (compv_uscalar_t j = 0; j < height; ++j) {
		for (compv_uscalar_t i = 0; i < width; ++i) {
			compv_float64_t x = ptrIn[i];
			x = COMPV_MATH_CLIP3(ExpMin, ExpMax, x);
			di di;
			di.d = x * ca + b;

			const double t = (di.d - b) * cra - x;
			const uint64_t u = ((di.i + 2095104) >> 11) << 52; // 2095104 is c.adj
			const double y = (C30 - t) * (t * t) * C20 - t + C10;

			di.i = u | lut64u[di.i & mask];
			ptrOut[i] = y * di.d;
		}
		ptrIn += stride;
		ptrOut += stride;
	}
}

template<typename T>
static COMPV_ERROR_CODE CompVMathExpExp(const CompVMatPtr &in, CompVMatPtrPtr out)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");
	const size_t rows = in->rows();
	const size_t cols = in->cols();
	const size_t stride = in->stride();

	CompVMatPtr out_ = *out;
	if (out_ != in) { // This function allows "in == out"
		COMPV_CHECK_CODE_RETURN(CompVMat::newObj(&out_, in));
	}

	auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
		const T* ptrIn = in->ptr<const T>(start);
		T* ptrOut = out_->ptr<T>(start);
		if (std::is_same<T, compv_float64_t>::value) {
			int minpack;
			void(*CompVMathExpExp_64f64f)(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const uint64_t* lut64u, const uint64_t* var64u, const compv_float64_t* var64f)
				= nullptr;
			COMPV_CHECK_CODE_RETURN(CompVMathExp::hookExp_64f(&CompVMathExpExp_64f64f, &minpack));
			CompVMathExpExp_64f64f(
				reinterpret_cast<const compv_float64_t*>(ptrIn), reinterpret_cast<compv_float64_t*>(ptrOut),
				cols, (end - start), stride,
				CompVMathExp::lut64u(), CompVMathExp::vars64u(), CompVMathExp::vars64f()
			);
			const size_t colsp = cols & -minpack;
			const size_t remain = (cols - colsp);
			if (remain) {
				CompVMathExpExp_minpack1_64f64f_C(
					reinterpret_cast<const compv_float64_t*>(&ptrIn[colsp]), reinterpret_cast<compv_float64_t*>(&ptrOut[colsp]),
					remain, (end - start), stride,
					CompVMathExp::lut64u(), CompVMathExp::vars64u(), CompVMathExp::vars64f()
				);
			}
		}
		else {
			CompVMathExpExp_C(
				ptrIn, ptrOut,
				cols, (end - start), stride
			);
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		cols,
		rows,
		1
	));

	*out = out_;
	return COMPV_ERROR_CODE_S_OK;
}

// out[i] = std::exp(in[i])
COMPV_ERROR_CODE CompVMathExp::exp(const CompVMatPtr &in, CompVMatPtrPtr out)
{
	COMPV_CHECK_EXP_RETURN(!in || !out || in->planeCount() != 1
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(CompVMathExp::init()); // Init LUT tables (nop if already done)
	CompVGenericFloatInvokeCodeRawType(in->subType(), CompVMathExpExp, in, out);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathExp::hookExp_64f(
	void(**CompVMathExpExp_64f64f)(const compv_float64_t* ptrIn, compv_float64_t* ptrOut, const compv_uscalar_t width, const compv_uscalar_t height, const compv_uscalar_t stride, const uint64_t* lut64u, const uint64_t* var64u, const compv_float64_t* var64f)
	, int* minpack COMPV_DEFAULT(nullptr)
)
{
	COMPV_CHECK_EXP_RETURN(!CompVMathExpExp_64f64f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	int minpack_ = 1;
	*CompVMathExpExp_64f64f = CompVMathExpExp_minpack1_64f64f_C, minpack_ = 1;
#if COMPV_ARCH_X86
	if (CompVCpu::isEnabled(kCpuFlagSSE2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((*CompVMathExpExp_64f64f = CompVMathExpExp_minpack2_64f64f_Intrin_SSE2, minpack_ = 2));
		//COMPV_EXEC_IFDEF_ASM_X64((*CompVMathExpExp_64f64f = CompVMathExpExp_minpack2_64f64f_Asm_X64_SSE2, minpack_ = 2));
	}
	if (CompVCpu::isEnabled(kCpuFlagAVX2)) {
		COMPV_EXEC_IFDEF_INTRIN_X86((*CompVMathExpExp_64f64f = CompVMathExpExp_minpack4_64f64f_Intrin_AVX2, minpack_ = 4));
		COMPV_EXEC_IFDEF_ASM_X64((*CompVMathExpExp_64f64f = CompVMathExpExp_minpack4_64f64f_Asm_X64_AVX2, minpack_ = 4));
	}
#elif COMPV_ARCH_ARM
#endif
	if (minpack) {
		*minpack = minpack_;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathExp::init()
{
	if (isInitialized()) {
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Init");

	// 64u LUT
	{
		const int s = static_cast<int>(sizeof(s_arrayLut64u) / sizeof(s_arrayLut64u[0]));
		const uint64_t mask = (1ULL << 52) - 1; // mask64(52)
		union di {
			compv_float64_t d;
			uint64_t i;
		};		
		for (int i = 0; i < s; i++) {
			di di;
			di.d = std::pow(2.0, i * (1.0 / s));
			s_arrayLut64u[i] = di.i & mask;
		}
	}

	s_bInitialized = true;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMathExp::deInit()
{
	// Nothing to do
	COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "DeInit");
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
