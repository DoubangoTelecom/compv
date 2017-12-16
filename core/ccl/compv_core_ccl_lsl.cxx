/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

/* @description
This class implement LSL (Light Speed Labeling) algorithm.
Some literature about LSL:
- Light Speed Labeling: https://www.lri.fr/~lacas/Publications/JRTIP10.pdf
- Parallel Light Speed Labeling: https://hal.archives-ouvertes.fr/hal-01361188/document
*/

#include "compv/core/ccl/compv_core_ccl_lsl.h"
#include "compv/core/ccl/compv_core_ccl_lsl_result.h"
#include "compv/core/compv_core.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_memz.h"

#include "compv/core/ccl/intrin/x86/compv_core_ccl_lsl_intrin_sse2.h"
#include "compv/core/ccl/intrin/x86/compv_core_ccl_lsl_intrin_ssse3.h"
#include "compv/core/ccl/intrin/x86/compv_core_ccl_lsl_intrin_avx2.h"

#include <numeric> /* std::iota */

#define COMPV_CCL_LSL_STEP1_MIN_SAMPLES_PER_THREAD		(20*20)
#define COMPV_CCL_LSL_STEP20_MIN_SAMPLES_PER_THREAD		(30*30)
#define COMPV_CCL_LSL_BUILD_LEA_MIN_SAMPLES_PER_THREAD	(40*40)

#define COMPV_THIS_CLASSNAME	"CompVConnectedComponentLabelingLSL"

COMPV_NAMESPACE_BEGIN()

// X64
#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_X64_SSSE3(
	COMPV_ALIGNED(SSE) const uint8_t* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi, const compv_uscalar_t ERi_stride,
	int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
	const compv_uscalar_t width, const compv_uscalar_t height
);
COMPV_EXTERNC void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_X64_AVX2(
	COMPV_ALIGNED(SSE) const uint8_t* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi, const compv_uscalar_t ERi_stride,
	int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
	const compv_uscalar_t width, const compv_uscalar_t height
);
COMPV_EXTERNC void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_X64_SSE2(
	const uint8_t* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi, const compv_uscalar_t ERi_stride,
	int16_t* RLCi, const compv_uscalar_t RLCi_stride,
	const compv_uscalar_t width, const compv_uscalar_t height
);
COMPV_EXTERNC void CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_CMOV(
	const int16_t* RLCi, const compv_uscalar_t RLCi_stride,
	int32_t* ERAi, const compv_uscalar_t ERAi_stride,
	const int16_t* ERi, const compv_uscalar_t ERi_stride,
	const int16_t* ner,
	const compv_uscalar_t width, const compv_uscalar_t height
);
#endif /* COMPV_ASM && COMPV_ARCH_X64 */

CompVConnectedComponentLabelingLSL::CompVConnectedComponentLabelingLSL()
	:CompVConnectedComponentLabeling(static_cast<int32_t>(COMPV_PLSL_ID))
{

}

CompVConnectedComponentLabelingLSL::~CompVConnectedComponentLabelingLSL()
{

}

COMPV_ERROR_CODE CompVConnectedComponentLabelingLSL::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_PLSL_SET_INT_TYPE: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		//int type = *reinterpret_cast<const int*>(valuePtr);

		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Set with id %d not implemented", id);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
}

template<typename T>
static void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_C(
	const T* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi, const compv_uscalar_t ERi_stride,
	int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
	const compv_uscalar_t width, const compv_uscalar_t height)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	int16_t i, er;
	const int16_t width1 = static_cast<int16_t>(width);
	int16_t ner_max = 0;
	int32_t ner_sum = 0;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		er = (Xi[0] & 1);
		ERi[0] = er;
		for (i = 1; i < width1; ++i) {
			er += ((Xi[i - 1] ^ Xi[i]) & 1);
			ERi[i] = er;
		}
		er += (Xi[width1 - 1] & 1);
		ner[j] = er;
		ner_sum += er;
		if (ner_max < er) { // TODO(dmi): asm use cmovgt
			ner_max = er;
		}
		/* next */
		Xi += Xi_stride;
		ERi += ERi_stride;
	}
	
	*ner_max1 = ner_max;
	*ner_sum1 = ner_sum;
}

template<typename T>
static void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_C(
	const T* Xi, const compv_uscalar_t Xi_stride,
	int16_t* ERi, const compv_uscalar_t ERi_stride,
	int16_t* RLCi, const compv_uscalar_t RLCi_stride,
	const compv_uscalar_t width, const compv_uscalar_t height
)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Unroll loop");
	const int16_t width1 = static_cast<int16_t>(width);
	int16_t er, i;
	for (compv_uscalar_t j = 0; j < height; ++j) {
		er = (Xi[0] & 1);
		RLCi[0] = 0;
		for (i = 1; i < width1; ++i) {
			if (ERi[i - 1] != ERi[i]) {
				RLCi[er++] = i;
			}
		}
		RLCi[er] = width1 - ((Xi[width1 - 1] & 1) ^ 1);

		/* next */
		Xi += Xi_stride;
		RLCi += RLCi_stride;
		ERi += ERi_stride;
	}
}

// Relative segment labeling: step#1
template<typename T>
static void step1_algo13_segment_STDZ(const CompVMatPtr& X, CompVMatPtr ptr16sER, CompVMatPtr ptr16sRLC, CompVMatPtr ptr16sNer, int16_t* ner_max1, int32_t* ner_sum1, const size_t start, const size_t end)
{
	const T* Xi = X->ptr<T>(start);
	int16_t* ERi = ptr16sER->ptr<int16_t>(start);
	int16_t* RLCi = ptr16sRLC->ptr<int16_t>(start);
	int16_t* ner0 = ptr16sNer->ptr<int16_t>(0, start);
	const compv_uscalar_t ERi_stride = static_cast<compv_uscalar_t>(ptr16sER->stride());
	const compv_uscalar_t RLCi_stride = static_cast<compv_uscalar_t>(ptr16sRLC->stride());
	const compv_uscalar_t X_stride = static_cast<compv_uscalar_t>(X->stride());
	const compv_uscalar_t width = static_cast<compv_uscalar_t>(X->cols());
	const compv_uscalar_t height = static_cast<compv_uscalar_t>(end - start);

	/* Hook to processing functions */
	typedef void(*FunERiPtr)(const T* Xi, const compv_uscalar_t Xi_stride,
		int16_t* ERi, const compv_uscalar_t ERi_stride,
		int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
		const compv_uscalar_t width, const compv_uscalar_t height);
	typedef void(*FunRLCiPtr)(const T* Xi, const compv_uscalar_t Xi_stride,
		int16_t* ERi, const compv_uscalar_t ERi_stride,
		int16_t* RLCi, const compv_uscalar_t RLCi_stride,
		const compv_uscalar_t width, const compv_uscalar_t height);

	FunERiPtr funPtrERi = [](const T* Xi, const compv_uscalar_t Xi_stride,
		int16_t* ERi, const compv_uscalar_t ERi_stride,
		int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
		const compv_uscalar_t width, const compv_uscalar_t height) {
		CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_C<T >(
			Xi, Xi_stride,
			ERi, ERi_stride,
			ner,
			ner_max1,
			ner_sum1,
			width, height
		);
	};
	FunRLCiPtr funPtrRLCi = [](const T* Xi, const compv_uscalar_t Xi_stride,
		int16_t* ERi, const compv_uscalar_t ERi_stride,
		int16_t* RLCi, const compv_uscalar_t RLCi_stride,
		const compv_uscalar_t width, const compv_uscalar_t height) {
		CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_C(
			Xi, Xi_stride,
			ERi, ERi_stride,
			RLCi, RLCi_stride,
			width, height
		);
	};


	if (std::is_same<T, uint8_t>::value) {
		/* ERi */
		void(*funPtrERi_8u16s32s)(const uint8_t* Xi, const compv_uscalar_t Xi_stride,
			int16_t* ERi, const compv_uscalar_t ERi_stride,
			int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
			const compv_uscalar_t width, const compv_uscalar_t height)
			= nullptr;
		// SIMD functions requires width > alignment (not ">=" but ">" because we start at 1 (asm code expect it), also c++ can handle short data without perf issues -thanks to unrolling-)
#if COMPV_ARCH_X86
		if (width > 16 && CompVCpu::isEnabled(kCpuFlagSSSE3) && X->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(funPtrERi_8u16s32s = CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Intrin_SSSE3);
			COMPV_EXEC_IFDEF_ASM_X64(funPtrERi_8u16s32s = CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_X64_SSSE3);
		}
		if (width > 32 && CompVCpu::isEnabled(kCpuFlagAVX2) && X->isAlignedAVX()) {
			COMPV_EXEC_IFDEF_ASM_X64(funPtrERi_8u16s32s = CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_X64_AVX2);
		}
#elif COMPV_ARCH_ARM
		//COMPV_EXEC_IFDEF_ASM_ARM32(funPtrERi_8u16s32s = nullptr);
		//COMPV_EXEC_IFDEF_ASM_ARM64(funPtrERi_8u16s32s = nullptr);
#endif

		if (funPtrERi_8u16s32s) {
			funPtrERi = reinterpret_cast<FunERiPtr>(funPtrERi_8u16s32s);
		}

		/* RLCi */
		void(*funPtrRLCi_8u16s)(const uint8_t* Xi, const compv_uscalar_t Xi_stride,
			int16_t* ERi, const compv_uscalar_t ERi_stride,
			int16_t* RLCi, const compv_uscalar_t RLCi_stride,
			const compv_uscalar_t width, const compv_uscalar_t height)
			= nullptr;
		// SIMD functions requires width > alignment (not ">=" but ">" because we start at 1 (asm code expect it), also c++ can handle short data without perf issues -thanks to unrolling-)
#if COMPV_ARCH_X86
		if (width > 16 && CompVCpu::isEnabled(kCpuFlagSSE2) && X->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(funPtrRLCi_8u16s = CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Intrin_SSE2);
			COMPV_EXEC_IFDEF_ASM_X64(funPtrRLCi_8u16s = CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_RLCi_8u16s_Asm_X64_SSE2);
		}
#elif COMPV_ARCH_ARM
		//COMPV_EXEC_IFDEF_ASM_ARM32(funPtrRLCi_8u16s = nullptr);
		//COMPV_EXEC_IFDEF_ASM_ARM64(funPtrRLCi_8u16s = nullptr);
#endif

		if (funPtrRLCi_8u16s) {
			funPtrRLCi = reinterpret_cast<FunRLCiPtr>(funPtrRLCi_8u16s);
		}
	}

	/* Compute ERi */
	funPtrERi(
		Xi, X_stride,
		ERi, ERi_stride,
		ner0, ner_max1, ner_sum1,
		width, height
	);

	/* COmpute RLCi */
	funPtrRLCi(
		Xi, X_stride,
		ERi, ERi_stride,
		RLCi, RLCi_stride,
		width, height
	);
}

static void CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_C(
	const int16_t* RLCi, const compv_uscalar_t RLCi_stride,
	int32_t* ERAi, const compv_uscalar_t ERAi_stride,
	const int16_t* ERiminus1, const compv_uscalar_t ERi_stride,
	const int16_t* ner,
	const compv_uscalar_t width, const compv_uscalar_t height)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You should ASM code which is faster");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found");

	const int16_t wminus1 = static_cast<int16_t>(width - 1);
	int16_t er, er0, er1, j0, j1, nerj;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		nerj = ner[j];
		for (er = 1; er < nerj; er += 2) {
			j0 = RLCi[er - 1];
			j1 = RLCi[er] - 1;
			// [check extension in case of 8-connect algorithm]
			j0 -= (j0 > 0);
			j1 += (j1 < wminus1);
			er0 = ERiminus1[j0];
			er1 = ERiminus1[j1];
			// [check label parity: segments are odd]
			er0 += ((er0 & 1) ^ 1);
			er1 -= ((er1 & 1) ^ 1);
			ERAi[er] = (er1 >= er0)
				? (er0 | er1 << 16)
				: 0;
		}
		ERiminus1 += ERi_stride;
		RLCi += RLCi_stride;
		ERAi += ERAi_stride;
	}
}

// 2.2 Equivalence construction: step#2.0 (MT friendly)
static void step20_algo14_equivalence_build(const CompVMatPtr& ptr16sER, const CompVMatPtr& ptr16sRLC, CompVMatPtr ptr16sNer, CompVMemZero32sPtr ptr32sERA, const size_t width, const size_t start, const size_t end)
{
	const size_t jstart = !start ? 1 : start;
	const int16_t* ERiminus1 = ptr16sER->ptr<const int16_t>(jstart - 1);
	const int16_t* RLCi = ptr16sRLC->ptr<const int16_t>(jstart);
	const int16_t* ner0 = ptr16sNer->ptr<const int16_t>(0, jstart);
	int32_t* ERAi = ptr32sERA->ptr(jstart);
	const compv_uscalar_t ERA_stride = static_cast<compv_uscalar_t>(ptr32sERA->stride());
	const compv_uscalar_t RLCi_stride = static_cast<compv_uscalar_t>(ptr16sRLC->stride());
	const compv_uscalar_t ERi_stride = static_cast<compv_uscalar_t>(ptr16sER->stride());

	void(*funPtr)(
		const int16_t* RLCi, const compv_uscalar_t RLCi_stride,
		int32_t* ERAi, const compv_uscalar_t ERAi_stride,
		const int16_t* ERiminus1, const compv_uscalar_t ERi_stride,
		const int16_t* ner,
		const compv_uscalar_t width, const compv_uscalar_t height
	) = 
	[](
		const int16_t* RLCi, const compv_uscalar_t RLCi_stride,
		int32_t* ERAi, const compv_uscalar_t ERAi_stride,
		const int16_t* ERiminus1, const compv_uscalar_t ERi_stride,
		const int16_t* ner,
		const compv_uscalar_t width, const compv_uscalar_t height
	) {
		CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_C(
			RLCi, RLCi_stride,
			ERAi, ERAi_stride,
			ERiminus1, ERi_stride,
			ner,
			width, height);
	};

#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagCMOV)) {
			COMPV_EXEC_IFDEF_ASM_X64(funPtr = CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_CMOV);
		}
#elif COMPV_ARCH_ARM
		//COMPV_EXEC_IFDEF_ASM_ARM32(funPtr = CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_ARM32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(funPtr = CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_ARM64);
#endif

	funPtr(
		RLCi, RLCi_stride,
		ERAi, ERA_stride,
		ERiminus1, ERi_stride,
		ner0,
		static_cast<compv_uscalar_t>(width),
		static_cast<compv_uscalar_t>(end - jstart)
	);
}

// 2.2 Equivalence construction: step#2.1 (not MT friendly)
// Algorithm 14: LSL equivalence construction
static void step21_algo14_equivalence_build(const CompVMatPtr& ptr16sNer, CompVMemZero32sPtr ptr32sERA, CompVMatPtr ptr32sEQ, int32_t* nea1)
{
	const size_t rows = ptr32sERA->rows();

	const int16_t* ner = ptr16sNer->ptr<const int16_t>(0, 0);
	const int16_t ner00 = ner[0];
	int32_t* ERA0 = ptr32sERA->ptr(0);
	int32_t nea = 0;
	for (int16_t er = 1; er < ner00; er += 2) {
		ERA0[er] = ++nea; // [new label]
	}
	
	int32_t* EQ = ptr32sEQ->ptr<int32_t>(0, 0);
	int32_t* ERAi = ptr32sERA->ptr(1);
	const int32_t* ERAiminus1 = ptr32sERA->ptr(0);
	const size_t ERA_stride = ptr32sERA->stride();
	
	for (size_t j = 1; j < rows; ++j) {
		const int16_t ner0j = ner[j];
		for (int16_t er = 1; er < ner0j; er += 2) {
			if (ERAi[er]) {
				const int32_t er0 = ERAi[er] & 0xffff;
				const int32_t er1 = (ERAi[er] >> 16) & 0xffff;
				int32_t ea = ERAiminus1[er0];
				int32_t a = EQ[ea];
				for (int32_t erk = er0 + 2; erk <= er1; erk += 2) {
					const int32_t eak = ERAiminus1[erk];
					const int32_t ak = EQ[eak];
					// [min extraction and propagation]
					if (a < ak) {
						EQ[eak] = a;
					}
					else {
						a = ak;
						EQ[ea] = a;
						ea = eak;
					}
				}
				ERAi[er] = a; // [global min]
			}
			else {
				ERAi[er] = ++nea; // [new label]
			}
		}
		ERAiminus1 = ERAi;
		ERAi += ERA_stride;
	}

	*nea1 = nea;
}

// 2.4 Equivalence resolution: step#4
static void step4_algo6_eq_resolv(const CompVMatPtr& ptr32sEQ, CompVMatPtr ptr32sA, int32_t* na1)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM implementation found (cmov)");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No unroll implementation found");
	const int32_t nea = static_cast<int32_t>(ptr32sA->cols());
	int32_t* A = ptr32sA->ptr<int32_t>(0, 0);
	const int32_t* EQ = ptr32sEQ->ptr<const int32_t>(0, 0);
	int32_t na = 0;

	for (int32_t ea = 1; ea < nea; ++ea) {
		const int32_t a = EQ[ea];
		COMPV_ASSERT(a <= nea); // FIXME(dmi): remove
		A[ea] = (a != ea)
			? A[a]
			: ++na;
	}
	*na1 = na;
}

static COMPV_ERROR_CODE build_EQ(const size_t ner_sum, CompVMatPtrPtr ptr32sEQ)
{
	/* Create EQ and init with 0...n */
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int32_t>(ptr32sEQ, 1, ner_sum));
	const int32_t n = static_cast<int32_t>(ner_sum);
	int32_t* EQ = (*ptr32sEQ)->ptr<int32_t>();
	for (int32_t i = 0; i < n; ++i) {
		EQ[i] = i;
	}
	return COMPV_ERROR_CODE_S_OK;
}

// MT-friendly
static COMPV_ERROR_CODE build_LEA(const CompVMatPtr& ptr16sNer, const CompVMemZero32sPtr& ptr32sERA, const CompVMatPtr& ptr16sRLC, const CompVMatPtr& ptr32sA, compv_ccl_lea_n_t& vecLEA, const size_t start, const size_t end)
{	
	const int16_t* ner = ptr16sNer->ptr<const int16_t>(0, 0);
	int32_t* ERAi = ptr32sERA->ptr(start);
	const int16_t* RLC = ptr16sRLC->ptr<const int16_t>(start);
	const int32_t* A = ptr32sA->ptr<const int32_t>(0, 0);
	const size_t ERA_stride = ptr32sERA->stride();
	const size_t RLC_stride = ptr16sRLC->stride();

	int16_t er;
	compv_ccl_lea_1_t::iterator it_lea;
	for (size_t j = start; j < end; ++j) {
		const int16_t ner0j = ner[j];
		compv_ccl_lea_1_t& lea = vecLEA[j];
		lea.resize(ner0j >> 1);
		for (er = 1, it_lea = lea.begin(); er < ner0j; er += 2, ++it_lea) {
			it_lea->a = A[ERAi[er]];
			it_lea->start = RLC[er - 1];
			it_lea->end = RLC[er];
		}
		ERAi += ERA_stride;
		RLC += RLC_stride;
	}
	return COMPV_ERROR_CODE_S_OK;
}

// VERY important: binar must be binary image (allowed values: 0x01, 0xff, 0x00)
COMPV_ERROR_CODE CompVConnectedComponentLabelingLSL::process(const CompVMatPtr& binar, CompVConnectedComponentLabelingResultPtrPtr result) /*Overrides(CompVConnectedComponentLabeling)*/
{
	COMPV_CHECK_EXP_RETURN(!binar || binar->isEmpty() || binar->planeCount() != 1 || binar->elmtInBytes() != sizeof(uint8_t) || !result
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	/* Create result */
	CompVConnectedComponentLabelingResultLSLImplPtr result_;
	if (*result && (*result)->id() == id()) {
		result_ = dynamic_cast<CompVConnectedComponentLabelingResultLSLImpl*>(**result);
	}
	COMPV_CHECK_CODE_RETURN(CompVConnectedComponentLabelingResultLSLImpl::newObj(&result_));
	COMPV_CHECK_CODE_RETURN(result_->reset());
	CompVSizeSz& szInputSize = result_->szInput();

	szInputSize.width = binar->cols();
	szInputSize.height = binar->rows();
	const size_t __ner_max = ((szInputSize.width + 1) >> 1); // full dashed row
	int16_t ner_max;
	int32_t ner_sum;
	
	/* Multi-threading dispatcher */
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	const size_t threadsCountStep1 = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(szInputSize.width, szInputSize.height, maxThreads, COMPV_CCL_LSL_STEP1_MIN_SAMPLES_PER_THREAD)
		: 1;
	const size_t minSamplePerThreadStep20 = std::max(
		static_cast<size_t>(COMPV_CCL_LSL_STEP20_MIN_SAMPLES_PER_THREAD),
		(szInputSize.width << 1) // At least #2 rows (because of ERiminus1)
	);

	CompVMatPtr ptr16sNer; // the number of segments of ERi - black + white -
	CompVMatPtr ptr16sER; //  an associative table of size w holding the relative labels er associated to Xi
	CompVMatPtr ptr16sRLC; // a table holding the run length coding of segments of the line Xi, RLCi-1 is the similar memorization of the previous line.
	CompVMemZero32sPtr ptr32sERA; // an associative table holding the association between er and ea: ea = ERAi[er]
	compv_ccl_lea_n_t& vecLEA = result_->vecLEA();
	CompVMatPtr ptr32sEQ; // the table holding the equivalence classes, before transitive closure
	CompVMatPtr ptr32sA; // the associative table of ancestors
	int32_t nea1 = 0; // the current number of absolute labels, update of EQ and ERAi
	int32_t& na1 = result_->na1(); // final number of absolute labels

	/* Create some local variables */
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int16_t>(&ptr16sER, szInputSize.height, szInputSize.width));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int16_t>(&ptr16sRLC, szInputSize.height, __ner_max));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int16_t>(&ptr16sNer, 1, szInputSize.height));

	/* Relative segment labeling: step#1 */
	auto funcPtrStep1 = [&](const int32_t mt_start, const int32_t mt_end, int16_t* mt_ner_max, int32_t* mt_ner_sum) -> COMPV_ERROR_CODE {
		step1_algo13_segment_STDZ<uint8_t>(
			binar,
			ptr16sER,
			ptr16sRLC,
			ptr16sNer,
			mt_ner_max,
			mt_ner_sum,
			mt_start,
			mt_end
		);
		return COMPV_ERROR_CODE_S_OK;
	};
	if (threadsCountStep1 > 1) {
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCountStep1);
		std::vector<int16_t> mt_ner_max(threadsCountStep1);
		std::vector<int32_t> mt_ner_sum(threadsCountStep1);
		const size_t heights = (szInputSize.height / threadsCountStep1);
		size_t YStart = 0, YEnd;
		for (size_t threadIdx = 0; threadIdx < threadsCountStep1; ++threadIdx) {
			YEnd = (threadIdx == (threadsCountStep1 - 1)) ? szInputSize.height : (YStart + heights);
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrStep1, static_cast<int32_t>(YStart), static_cast<int32_t>(YEnd), &mt_ner_max[threadIdx], &mt_ner_sum[threadIdx]),
				taskIds), "Dispatching task failed");
			YStart += heights;
		}
		COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[0]));
		ner_max = mt_ner_max[0];
		ner_sum = mt_ner_sum[0];
		for (size_t threadIdx = 1; threadIdx < threadsCountStep1; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[threadIdx]));
			if (ner_max < mt_ner_max[threadIdx]) {
				ner_max = mt_ner_max[threadIdx];
			}
			ner_sum += mt_ner_sum[threadIdx];
		}
	}
	else {
		COMPV_CHECK_CODE_RETURN(funcPtrStep1(0, static_cast<int32_t>(szInputSize.height), &ner_max, &ner_sum));
	}

	/* Create ERA and init with zeros */
	COMPV_CHECK_CODE_RETURN(CompVMemZero32s::newObj(&ptr32sERA, szInputSize.height, ner_max));

	/* Equivalence construction: step#2.0 (MT-friendly) */
	auto funcPtrStep20 = [&](const size_t mt_start, const size_t mt_end) -> COMPV_ERROR_CODE {
		step20_algo14_equivalence_build(
			ptr16sER,
			ptr16sRLC,
			ptr16sNer,
			ptr32sERA,
			static_cast<int32_t>(szInputSize.width),
			static_cast<int32_t>(mt_start),
			static_cast<int32_t>(mt_end)
		);
		if (!mt_start) {
			/* Build EQ */
			COMPV_CHECK_CODE_RETURN(build_EQ(ner_sum, &ptr32sEQ));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrStep20,
		szInputSize.width,
		szInputSize.height,
		minSamplePerThreadStep20
	));
	
	/* Equivalence construction: step#2.1 (MT-unfriendly) */
	step21_algo14_equivalence_build(
		ptr16sNer,
		ptr32sERA,
		ptr32sEQ,
		&nea1
	);

	/* Equivalence resolution: step#4 (MT-unfriendly) */
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int32_t>(&ptr32sA, 1, (nea1 + 1))); // +1 for background which is "0"
	*ptr32sA->ptr<int32_t>(0, 0) = 0; // other values will be initialzed in step4_algo6_eq_resolv
	step4_algo6_eq_resolv(
		ptr32sEQ,
		ptr32sA,
		&na1
	);

	/* Build LEA (MT friendly) */
	vecLEA.resize(szInputSize.height);
	auto funcPtrBuildLEA = [&](const size_t mt_start, const size_t mt_end) -> COMPV_ERROR_CODE {
		COMPV_CHECK_CODE_RETURN(build_LEA(
			ptr16sNer,
			ptr32sERA,
			ptr16sRLC,
			ptr32sA,
			vecLEA,
			mt_start,
			mt_end
		));
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrBuildLEA,
		szInputSize.width,
		szInputSize.height,
		COMPV_CCL_LSL_BUILD_LEA_MIN_SAMPLES_PER_THREAD
	));
	
	*result = *result_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingLSL::newObj(CompVConnectedComponentLabelingPtrPtr ccl)
{
	COMPV_CHECK_CODE_RETURN(CompVCore::init());
	COMPV_CHECK_EXP_RETURN(!ccl, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVConnectedComponentLabelingPtr _ccl = new CompVConnectedComponentLabelingLSL();
	COMPV_CHECK_EXP_RETURN(!_ccl, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*ccl = *_ccl;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()