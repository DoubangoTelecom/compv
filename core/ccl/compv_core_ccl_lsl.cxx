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
#include "compv/core/compv_core.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_memz.h"

#include "compv/core/ccl/intrin/x86/compv_core_ccl_lsl_intrin_sse2.h"
#include "compv/core/ccl/intrin/x86/compv_core_ccl_lsl_intrin_ssse3.h"
#include "compv/core/ccl/intrin/x86/compv_core_ccl_lsl_intrin_avx2.h"

#define COMPV_CCL_LSL_STEP1_MIN_SAMPLES_PER_THREAD	(20*20)
#define COMPV_CCL_LSL_STEP20_MIN_SAMPLES_PER_THREAD	(30*30)

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

static const compv_ccl_indice_t kCompVConnectedComponentLabelingLSLBachgroundLabel = 0; // Must be zero because of calloc()
typedef CompVMemZero<compv_ccl_indice_t > CompVMemZeroCclIndice;
typedef CompVPtr<CompVMemZeroCclIndice *> CompVMemZeroCclIndicePtr;

CompVConnectedComponentLabelingLSL::CompVConnectedComponentLabelingLSL()
	:CompVConnectedComponentLabeling(static_cast<compv_ccl_indice_t>(COMPV_PLSL_ID))
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
	int16_t* ner, int16_t* ner_max1, compv_ccl_indice_t* ner_sum1,
	const compv_uscalar_t width, const compv_uscalar_t height)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation could be found");
	COMPV_DEBUG_INFO_CODE_TODO("Unroll loop");
	int16_t i, er;
	const int16_t width1 = static_cast<int16_t>(width);
	int16_t ner_max = 0;
	compv_ccl_indice_t ner_sum = 0;

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
// Algorithm 12: LSL segment detection STD
// Xi: a binary line of width w (allowed values: 0x01, 0xff, 0x00)
// ERi, an associative table of size w holding the relative labels er associated to Xi
// RLCi, a table holding the run length coding of segments of the line Xi
// ner, the number of segments of ERi – black + white
template<typename T>
static void step1_algo13_segment_STDZ(const CompVMatPtr& X, int16_t* ER, const size_t ER_stride, int16_t* RLC, const size_t RLC_stride, int16_t* ner, int16_t* ner_max1, compv_ccl_indice_t* ner_sum1, const compv_ccl_indice_t w, const compv_ccl_indice_t start, const compv_ccl_indice_t end)
{
	const T* Xi = X->ptr<T>(start);
	int16_t* ERi = ER + (ER_stride * start);
	int16_t* RLCi = RLC + (RLC_stride * start);
	int16_t* ner0 = ner + start;

	const size_t X_stride = X->stride();

	/* Hook to processing functions */
	typedef void(*FunERiPtr)(const T* Xi, const compv_uscalar_t Xi_stride,
		int16_t* ERi, const compv_uscalar_t ERi_stride,
		int16_t* ner, int16_t* ner_max1, compv_ccl_indice_t* ner_sum1,
		const compv_uscalar_t width, const compv_uscalar_t height);
	typedef void(*FunRLCiPtr)(const T* Xi, const compv_uscalar_t Xi_stride,
		int16_t* ERi, const compv_uscalar_t ERi_stride,
		int16_t* RLCi, const compv_uscalar_t RLCi_stride,
		const compv_uscalar_t width, const compv_uscalar_t height);

	FunERiPtr funPtrERi = [](const T* Xi, const compv_uscalar_t Xi_stride,
		int16_t* ERi, const compv_uscalar_t ERi_stride,
		int16_t* ner, int16_t* ner_max1, compv_ccl_indice_t* ner_sum1,
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


	if (std::is_same<T, uint8_t>::value && std::is_same<int32_t, compv_ccl_indice_t>::value) {
		/* ERi */
		void(*funPtrERi_8u16s32s)(const uint8_t* Xi, const compv_uscalar_t Xi_stride,
			int16_t* ERi, const compv_uscalar_t ERi_stride,
			int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
			const compv_uscalar_t width, const compv_uscalar_t height)
			= nullptr;
		// SIMD functions requires width > alignment (not ">=" but ">" because we start at 1 (asm code expect it), also c++ can handle short data without perf issues -thanks to unrolling-)
#if COMPV_ARCH_X86
		if (w > 16 && CompVCpu::isEnabled(kCpuFlagSSSE3) && X->isAlignedSSE()) {
			COMPV_EXEC_IFDEF_INTRIN_X86(funPtrERi_8u16s32s = CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Intrin_SSSE3);
			COMPV_EXEC_IFDEF_ASM_X64(funPtrERi_8u16s32s = CompVConnectedComponentLabelingLSL_Step1Algo13SegmentSTDZ_ERi_8u16s32s_Asm_X64_SSSE3);
		}
		if (w > 32 && CompVCpu::isEnabled(kCpuFlagAVX2) && X->isAlignedAVX()) {
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
		if (w > 16 && CompVCpu::isEnabled(kCpuFlagSSE2) && X->isAlignedSSE()) {
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
		ERi, ER_stride,
		ner0, ner_max1, ner_sum1,
		static_cast<compv_uscalar_t>(w), static_cast<compv_uscalar_t>(end - start)
	);

	/* COmpute RLCi */
	funPtrRLCi(
		Xi, X_stride,
		ERi, ER_stride,
		RLCi, RLC_stride,
		static_cast<compv_uscalar_t>(w), static_cast<compv_uscalar_t>(end - start)
	);
}

static void CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_C(
	const int16_t* RLCi, const compv_uscalar_t RLCi_stride,
	compv_ccl_indice_t* ERAi, const compv_uscalar_t ERAi_stride,
	const int16_t* ERiminus1, const compv_uscalar_t ERi_stride,
	const int16_t* ner,
	const compv_uscalar_t width, const compv_uscalar_t height)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You should ASM code which is faster");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD implementation could be found");

	const int16_t wminus1 = static_cast<int16_t>(width - 1);
	int16_t er, er0, er1, j0, j1, nerj;

	COMPV_DEBUG_INFO_CODE_TODO("ASM code not aligned: change single line: setting j1=RLCi[er] - 1");

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
// Algorithm 14: LSL equivalence construction
// ERi, an associative table of size w holding the relative labels er associated to Xi
// RLCi, a table holding the run length coding of segments of the line Xi
// ERAi, an associative table holding the association between er and ea: ea = ERAi[er]
static void step20_algo14_equivalence_build(const int16_t* ER, const size_t ER_stride, const int16_t* RLC, const size_t RLC_stride, const int16_t* ner, CompVMemZeroCclIndicePtr ERA, const compv_ccl_indice_t w, const compv_ccl_indice_t start, const compv_ccl_indice_t end)
{
	const compv_ccl_indice_t jstart = !start ? 1 : start;
	const int16_t* ERiminus1 = ER + (ER_stride * (jstart - 1));
	const int16_t* RLCi = RLC + (RLC_stride * jstart);
	const int16_t* ner0 = ner + (jstart);
	compv_ccl_indice_t* ERAi = ERA->ptr(jstart);
	const size_t ERA_stride = ERA->stride();

	typedef void(*FunPtr)(const int16_t* RLCi, const compv_uscalar_t RLCi_stride,
		compv_ccl_indice_t* ERAi, const compv_uscalar_t ERAi_stride,
		const int16_t* ERiminus1, const compv_uscalar_t ERi_stride,
		const int16_t* ner,
		const compv_uscalar_t width, const compv_uscalar_t height);
	FunPtr funPtr = [](const int16_t* RLCi, const compv_uscalar_t RLCi_stride,
		compv_ccl_indice_t* ERAi, const compv_uscalar_t ERAi_stride,
		const int16_t* ERiminus1, const compv_uscalar_t ERi_stride,
		const int16_t* ner,
		const compv_uscalar_t width, const compv_uscalar_t height) {
		CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_C(
			RLCi, RLCi_stride,
			ERAi, ERAi_stride,
			ERiminus1, ERi_stride,
			ner,
			width, height);
	};

	if (std::is_same<int32_t, compv_ccl_indice_t>::value) {
		void(*funPtr_16s32s)(const int16_t* RLCi, const compv_uscalar_t RLCi_stride,
			int32_t* ERAi, const compv_uscalar_t ERAi_stride,
			const int16_t* ERi, const compv_uscalar_t ERi_stride,
			const int16_t* ner,
			const compv_uscalar_t width, const compv_uscalar_t height)
			= nullptr;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagCMOV)) {
			COMPV_EXEC_IFDEF_ASM_X64(funPtr_16s32s = CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_CMOV);
		}
#elif COMPV_ARCH_ARM
		//COMPV_EXEC_IFDEF_ASM_ARM32(funPtr_16s32s = CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_ARM32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(funPtr_16s32s = CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_ARM64);
#endif

		if (funPtr_16s32s) {
			funPtr = reinterpret_cast<FunPtr>(funPtr_16s32s);
		}
	}

	funPtr(
		RLCi, RLC_stride,
		ERAi, ERA_stride,
		ERiminus1, ER_stride,
		ner0,
		static_cast<compv_uscalar_t>(w), static_cast<compv_uscalar_t>(end - jstart)
	);
}

// 2.2 Equivalence construction: step#2.1 (not MT friendly)
// Algorithm 14: LSL equivalence construction
// EQ,  the table holding the equivalence classes, before transitive closure
// ERAi, an associative table holding the association between er and ea: ea = ERAi[er]
// ner, the number of segments of ERi - black + white
// nea the current number of absolute labels, update of EQ and ERAi
static void step21_algo14_equivalence_build(const int16_t* ner, CompVMemZeroCclIndicePtr ERA, compv_ccl_indice_t* EQ, compv_ccl_indice_t* nea1)
{
	const int16_t ner00 = *ner;
	compv_ccl_indice_t* ERA0 = ERA->ptr(0);
	compv_ccl_indice_t nea = 0;
	for (int16_t er = 1; er < ner00; er += 2) {
		ERA0[er] = ++nea; // [new label]
	}

	const size_t rows = ERA->rows();
	compv_ccl_indice_t* ERAi = ERA->ptr(1);
	const compv_ccl_indice_t* ERAiminus1 = ERA->ptr(0);
	const size_t ERA_stride = ERA->stride();

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM implementation found");
	for (size_t j = 1; j < rows; ++j) {
		const int16_t ner0j = ner[j];
		for (int16_t er = 1; er < ner0j; er += 2) {
			if (ERAi[er]) {
				const compv_ccl_indice_t er0 = ERAi[er] & 0xffff;
				const compv_ccl_indice_t er1 = (ERAi[er] >> 16) & 0xffff;
				compv_ccl_indice_t ea = ERAiminus1[er0];
				compv_ccl_indice_t a = EQ[ea];
				for (compv_ccl_indice_t erk = er0 + 2; erk <= er1; erk += 2) {
					const compv_ccl_indice_t eak = ERAiminus1[erk];
					const compv_ccl_indice_t ak = EQ[eak];
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
// EQ, the table holding the equivalence classes, before transitive closure
// A, the associative table of ancestors
// nea, the current number of absolute labels
// na, final number of absolute labels (background not counted)
static void step4_algo6_eq_resolv(const compv_ccl_indice_t* EQ, const compv_ccl_indice_t nea, compv_ccl_indice_t* A, compv_ccl_indice_t& na)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM implementation found (cmov)");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No unroll implementation found");
	na = 0;

	for (compv_ccl_indice_t ea = 1; ea <= nea; ++ea) {
		const compv_ccl_indice_t a = EQ[ea];
		COMPV_ASSERT(a <= nea); // FIXME(dmi): remove
		A[ea] = (a != ea)
			? A[a]
			: ++na;
	}
}

static COMPV_ERROR_CODE build_EQ(const size_t ner_sum, compv_ccl_indice_t*& EQ)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
	/* Create EQ and init with 0...n */
	EQ = reinterpret_cast<compv_ccl_indice_t*>(CompVMem::malloc(ner_sum * 1 * sizeof(compv_ccl_indice_t)));
	COMPV_CHECK_EXP_RETURN(!EQ, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	const compv_ccl_indice_t n = static_cast<compv_ccl_indice_t>(ner_sum);
	for (compv_ccl_indice_t i = 0; i < n; ++i) {
		EQ[i] = i;
	}
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE build_all_labels(const compv_ccl_indice_t* A, const CompVMemZeroCclIndicePtr& ERA, const int16_t* ER, const size_t ER_width, const size_t ER_height, const size_t ER_stride, CompVMatPtrPtr EA)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
	const size_t ERA_stride = ERA->stride();

	// Create EA using same size and stride as ER
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_ccl_indice_t>(EA, ER_height, ER_width, ER_stride));

	const compv_ccl_indice_t* ERAPtr = ERA->ptr();
	compv_ccl_indice_t* EAPtr = (*EA)->ptr<compv_ccl_indice_t>();

	// #3 and #5 merged 
	// step #3: First absolute labeling
	// step #5: Second absolute labeling
	for (size_t j = 0; j < ER_height; ++j) {
		for (size_t i = 0; i < ER_width; ++i) {
			EAPtr[i] = A[ERAPtr[ER[i]]];
		}
		EAPtr += ER_stride;
		ER += ER_stride;
		ERAPtr += ERA_stride;
	}

	return COMPV_ERROR_CODE_S_OK;
}

// VERY important: binar must be binary image (allowed values: 0x01, 0xff, 0x00)
COMPV_ERROR_CODE CompVConnectedComponentLabelingLSL::process(const CompVMatPtr& binar, CompVConnectedComponentLabelingResult& result) /*Overrides(CompVConnectedComponentLabeling)*/
{
	COMPV_CHECK_EXP_RETURN(!binar || binar->isEmpty() || binar->planeCount() != 1 || binar->elmtInBytes() != sizeof(uint8_t)
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	/* Reset result */
	result.reset();
	result.label_background = kCompVConnectedComponentLabelingLSLBachgroundLabel;

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;

	const size_t width = binar->cols();
	const size_t height = binar->rows();
	const size_t __ner_max = ((width + 1) >> 1); // full dashed row
	int16_t ner_max;
	compv_ccl_indice_t ner_sum;
	std::function<COMPV_ERROR_CODE(const compv_ccl_indice_t mt_start, const compv_ccl_indice_t mt_end, int16_t* mt_ner_max, compv_ccl_indice_t* mt_ner_sum)> funcPtrStep1;
	std::function<COMPV_ERROR_CODE(const size_t mt_start, const size_t mt_end)> funcPtrStep20;

	/* Multi-threading dispatcher */
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
	const size_t threadsCountStep1 = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(width, height, maxThreads, COMPV_CCL_LSL_STEP1_MIN_SAMPLES_PER_THREAD)
		: 1;
	const size_t minSamplePerThreadStep20 = std::max(
		static_cast<size_t>(COMPV_CCL_LSL_STEP20_MIN_SAMPLES_PER_THREAD),
		(width << 1) // At least #2 rows (because of ERiminus1)
	);

	int16_t* ner = nullptr; // the number of segments of ERi - black + white -
	int16_t* ER = nullptr; //  an associative table of size w holding the relative labels er associated to Xi
	int16_t* RLC = nullptr; // a table holding the run length coding of segments of the line Xi, RLCi-1 is the similar memorization of the previous line.
	CompVMemZeroCclIndicePtr ERA; // an associative table holding the association between er and ea: ea = ERAi[er]
	compv_ccl_indice_t* EQ = nullptr; // the table holding the equivalence classes, before transitive closure
	compv_ccl_indice_t* A = nullptr; // the associative table of ancestors
	compv_ccl_indice_t nea; // the current number of absolute labels, update of EQ and ERAi
	compv_ccl_indice_t na = 0; // final number of absolute labels

	ER = reinterpret_cast<int16_t*>(CompVMem::malloc(height * width * sizeof(int16_t)));
	COMPV_CHECK_EXP_BAIL(!ER, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
	RLC = reinterpret_cast<int16_t*>(CompVMem::malloc(height * __ner_max * sizeof(int16_t)));
	COMPV_CHECK_EXP_BAIL(!RLC, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
	ner = reinterpret_cast<int16_t*>(CompVMem::malloc(height * 1 * sizeof(int16_t)));
	COMPV_CHECK_EXP_BAIL(!ner, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));

	/* Relative segment labeling: step#1 */
	funcPtrStep1 = [&](const compv_ccl_indice_t mt_start, const compv_ccl_indice_t mt_end, int16_t* mt_ner_max, compv_ccl_indice_t* mt_ner_sum) -> COMPV_ERROR_CODE {
		step1_algo13_segment_STDZ<uint8_t>(
			binar,
			ER,
			width,
			RLC,
			__ner_max,
			ner,
			mt_ner_max,
			mt_ner_sum,
			static_cast<compv_ccl_indice_t>(width),
			mt_start,
			mt_end
			);
		return COMPV_ERROR_CODE_S_OK;
	};
	if (threadsCountStep1 > 1) {
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCountStep1);
		std::vector<int16_t> mt_ner_max(threadsCountStep1);
		std::vector<compv_ccl_indice_t> mt_ner_sum(threadsCountStep1);
		const size_t heights = (height / threadsCountStep1);
		size_t YStart = 0, YEnd;
		for (size_t threadIdx = 0; threadIdx < threadsCountStep1; ++threadIdx) {
			YEnd = (threadIdx == (threadsCountStep1 - 1)) ? height : (YStart + heights);
			COMPV_CHECK_CODE_BAIL(err = threadDisp->invoke(std::bind(funcPtrStep1, static_cast<compv_ccl_indice_t>(YStart), static_cast<compv_ccl_indice_t>(YEnd), &mt_ner_max[threadIdx], &mt_ner_sum[threadIdx]),
				taskIds), "Dispatching task failed");
			YStart += heights;
		}
		COMPV_CHECK_CODE_BAIL(err = threadDisp->waitOne(taskIds[0]));
		ner_max = mt_ner_max[0];
		ner_sum = mt_ner_sum[0];
		for (size_t threadIdx = 1; threadIdx < threadsCountStep1; ++threadIdx) {
			COMPV_CHECK_CODE_BAIL(err = threadDisp->waitOne(taskIds[threadIdx]));
			if (ner_max < mt_ner_max[threadIdx]) {
				ner_max = mt_ner_max[threadIdx];
			}
			ner_sum += mt_ner_sum[threadIdx];
		}
	}
	else {
		COMPV_CHECK_CODE_BAIL(err = funcPtrStep1(0, static_cast<compv_ccl_indice_t>(height), &ner_max, &ner_sum));
	}

	/* Create ERA and init with zeros */
	COMPV_CHECK_CODE_BAIL(err = CompVMemZeroCclIndice::newObj(&ERA, height, ner_max));

	/* Equivalence construction: step#2.0 (MT-friendly) */
	funcPtrStep20 = [&](const size_t mt_start, const size_t mt_end) -> COMPV_ERROR_CODE {
		step20_algo14_equivalence_build(
			ER,
			width,
			RLC,
			__ner_max,
			ner,
			ERA,
			static_cast<compv_ccl_indice_t>(width),
			static_cast<compv_ccl_indice_t>(mt_start),
			static_cast<compv_ccl_indice_t>(mt_end)
		);
		if (!mt_start) {
			/* Build EQ */
			COMPV_CHECK_CODE_RETURN(build_EQ(ner_sum, EQ));
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_BAIL(err = CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrStep20,
		width,
		height,
		minSamplePerThreadStep20
	));
	
	/* Equivalence construction: step#2.1 (not MT-friendly) */
	step21_algo14_equivalence_build(
		ner,
		ERA,
		EQ,
		&nea
	);

	/* Create A and init first element with zero (because bacground label is equal to zero) */
	A = reinterpret_cast<compv_ccl_indice_t*>(CompVMem::malloc((nea + 1) * sizeof(compv_ccl_indice_t)));
	COMPV_CHECK_EXP_BAIL(!A, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
	A[0] = 0; // other values will be initialzed in step4_algo6_eq_resolv

	/* Equivalence resolution: step#4 */
	step4_algo6_eq_resolv(
		EQ,
		nea,
		A,
		na
	);

	/* For testing */
#if 1
	build_all_labels(
		A,
		ERA, 
		ER,
		width,
		height,
		width,
		&result.labels);
#endif
	result.labels_count = (na + 1); // +1 for the background

bail:
	CompVMem::free(reinterpret_cast<void**>(&ner));
	CompVMem::free(reinterpret_cast<void**>(&ER));
	CompVMem::free(reinterpret_cast<void**>(&A));
	CompVMem::free(reinterpret_cast<void**>(&EQ));
	CompVMem::free(reinterpret_cast<void**>(&RLC));

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