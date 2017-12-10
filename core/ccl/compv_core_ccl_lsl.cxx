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
#include "compv/base/compv_md5.h" // FIXME(dmi): remove

#include "compv/core/ccl/intrin/x86/compv_core_ccl_lsl_intrin_sse2.h"

#include <numeric> /* std::iota() */

#define COMPV_CCL_LSL_STEP1_MIN_SAMPLES_PER_THREAD				(20*20)
#define COMPV_CCL_LSL_STEP20_MIN_SAMPLES_PER_THREAD				(30*30)
#define COMPV_CCL_LSL_STEP1_AND_STEP20_MIN_SAMPLES_PER_THREAD	(1*1) // Step1 and step20 bundled (very cpu intensive)

#define COMPV_THIS_CLASSNAME	"CompVConnectedComponentLabelingLSL"

COMPV_NAMESPACE_BEGIN()

// X64
#if COMPV_ASM && COMPV_ARCH_X64
COMPV_EXTERNC void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentRLE_8u16s32s_Asm_X64_CMOV(const uint8_t* Xi, const compv_uscalar_t Xi_stride,
	int16_t* RLCi, const compv_uscalar_t RLCi_stride,
	int16_t* ERi, const compv_uscalar_t ERi_stride,
	int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
	const compv_uscalar_t width, const compv_uscalar_t height);
COMPV_EXTERNC void CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_CMOV(
	const int16_t* RLCi, const compv_uscalar_t RLCi_stride,
	int32_t* ERAi, const compv_uscalar_t ERAi_stride,
	const int16_t* ERiminus1, const compv_uscalar_t ERi_stride,
	const int16_t* ner,
	const compv_uscalar_t width, const compv_uscalar_t height);
#endif /* COMPV_ASM && COMPV_ARCH_X64 */

static const compv_ccl_indice_t kCompVConnectedComponentLabelingLSLBachgroundLabel = 0; // Must be zero because of calloc()

CompVConnectedComponentLabelingLSL::CompVConnectedComponentLabelingLSL()
	:CompVConnectedComponentLabeling(static_cast<compv_ccl_indice_t>(COMPV_LSL_ID))
{

}

CompVConnectedComponentLabelingLSL::~CompVConnectedComponentLabelingLSL()
{

}

COMPV_ERROR_CODE CompVConnectedComponentLabelingLSL::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_LSL_SET_INT_TYPE: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		int type = *reinterpret_cast<const int*>(valuePtr);

		return COMPV_ERROR_CODE_S_OK;
	}
	default:
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Set with id %d not implemented", id);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
}

template<typename T>
static void CompVConnectedComponentLabelingLSL_Step1Algo13SegmentRLE_C(
	const T* Xi, const compv_uscalar_t Xi_stride,
	int16_t* RLCi, const compv_uscalar_t RLCi_stride,
	int16_t* ERi, const compv_uscalar_t ERi_stride,
	int16_t* ner, int16_t* ner_max1, compv_ccl_indice_t* ner_sum1,
	const compv_uscalar_t width, const compv_uscalar_t height)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You should ASM code which is faster");
#define SET_RLC_1(mm, ii) \
		if ((mm)) { \
			RLCi[er] = ((ii) - b); \
			b ^= 1;  \
			++er; \
		} \
		ERi[(ii)] = er

	int16_t i, b, er;
	const int16_t width4 = (width - 1) & -4; // *must be* (width - 1) back aligned because i start at 1 (checker.yuv fails with MT enabled)
	const int16_t width1 = static_cast<int16_t>(width);
	int16_t ner_max = 0;
	compv_ccl_indice_t ner_sum = 0;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		/* For i = 0 */
		b = Xi[0] & 1; // right border compensation
		er = b; // a relative label
		RLCi[0] = 0;
		ERi[0] = er;
		/* i = 1....w */
		{
			for (i = 1; i < width4; i += 4) {
				SET_RLC_1(Xi[i - 1] ^ Xi[i], i);
				SET_RLC_1(Xi[i] ^ Xi[i + 1], i + 1);
				SET_RLC_1(Xi[i + 1] ^ Xi[i + 2], i + 2);
				SET_RLC_1(Xi[i + 2] ^ Xi[i + 3], i + 3);
			}
			for (; i < width1; ++i) {
				SET_RLC_1(Xi[i - 1] ^ Xi[i], i);
			}
		}
		/* update las RLCi and ner */
		RLCi[er] = (width1 - b);
		const int16_t nerj = er + (Xi[width1 - 1] & 1);
		ner[j] = nerj;
		ner_sum += nerj;
		if (ner_max < nerj) { // TODO(dmi): asm use cmovgt
			ner_max = nerj;
		}
		/* next */
		Xi += Xi_stride;
		ERi += ERi_stride;
		RLCi += RLCi_stride;
	}

	*ner_max1 = ner_max;
	*ner_sum1 = ner_sum;
}

// Relative segment labeling: step#1
// Algorithm 12: LSL segment detection STD
// Xi: a binary line of width w (allowed values: 0x01, 0xff, 0x00)
// ERi, an associative table of size w holding the relative labels er associated to Xi
// RLCi, a table holding the run length coding of segments of the line Xi
// ner, the number of segments of ERi – black + white
template<typename T>
static void step1_algo13_segment_RLE(const CompVMatPtr& X, CompVMatPtr ER, CompVMatPtr RLC, CompVMatPtr ner, int16_t* ner_max1, compv_ccl_indice_t* ner_sum1, const compv_ccl_indice_t w, const compv_ccl_indice_t start, const compv_ccl_indice_t end)
{
	const T* Xi = X->ptr<T>(start);
	int16_t* ERi = ER->ptr<int16_t>(start);
	int16_t* RLCi = RLC->ptr<int16_t>(0); // zero-based (per thread)
	int16_t* ner0 = ner->ptr<int16_t>(0, start); // zero-based (per row)

	const size_t X_stride = X->stride();
	const size_t ER_stride = ER->stride();
	const size_t RLC_stride = RLC->stride();

	/* Hook to processing function */
	typedef void(*FunPtr)(const T* Xi, const compv_uscalar_t Xi_stride,
		int16_t* RLCi, const compv_uscalar_t RLCi_stride,
		int16_t* ERi, const compv_uscalar_t ERi_stride,
		int16_t* ner, int16_t* ner_max1, compv_ccl_indice_t* ner_sum1,
		const compv_uscalar_t width, const compv_uscalar_t height);
	FunPtr funPtr = [](const T* Xi, const compv_uscalar_t Xi_stride,
		int16_t* RLCi, const compv_uscalar_t RLCi_stride,
		int16_t* ERi, const compv_uscalar_t ERi_stride,
		int16_t* ner, int16_t* ner_max1, compv_ccl_indice_t* ner_sum1,
		const compv_uscalar_t width, const compv_uscalar_t height)  {
		CompVConnectedComponentLabelingLSL_Step1Algo13SegmentRLE_C<T >(
			Xi, Xi_stride,
			RLCi, RLCi_stride,
			ERi, ERi_stride,
			ner,
			ner_max1,
			ner_sum1,
			width, height);
	};

	if (std::is_same<T, uint8_t>::value && std::is_same<int32_t, compv_ccl_indice_t>::value) {
		void(*funPtr_8u16s32s)(const uint8_t* Xi, const compv_uscalar_t Xi_stride,
			int16_t* RLCi, const compv_uscalar_t RLCi_stride,
			int16_t* ERi, const compv_uscalar_t ERi_stride,
			int16_t* ner, int16_t* ner_max1, int32_t* ner_sum1,
			const compv_uscalar_t width, const compv_uscalar_t height)
			= nullptr;
#if COMPV_ARCH_X86
		if (CompVCpu::isEnabled(kCpuFlagCMOV)) { // All X64 archs support CMOVcc but we want to allow disabling it
			COMPV_EXEC_IFDEF_ASM_X64(funPtr_8u16s32s = CompVConnectedComponentLabelingLSL_Step1Algo13SegmentRLE_8u16s32s_Asm_X64_CMOV);
		}
#elif COMPV_ARCH_ARM
		//COMPV_EXEC_IFDEF_ASM_ARM32(funPtr_8u16s32s = CompVConnectedComponentLabelingLSL_Step1Algo13SegmentRLE_8u16s_Asm_NEON32);
		//COMPV_EXEC_IFDEF_ASM_ARM64(funPtr_8u16s32s = CompVConnectedComponentLabelingLSL_Step1Algo13SegmentRLE_8u16s_Asm_NEON64);
#endif
		if (funPtr_8u16s32s) {
			funPtr = reinterpret_cast<FunPtr>(funPtr_8u16s32s);
		}
	}

	funPtr(
		Xi, X_stride,
		RLCi, RLC_stride,
		ERi, ER_stride,
		ner0, ner_max1, ner_sum1,
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

	const int16_t wminus1 = static_cast<int16_t>(width - 1);
	int16_t er, er0, er1, j0, j1, nerj;

	for (compv_uscalar_t j = 0; j < height; ++j) {
		nerj = ner[j];
		for (er = 1; er < nerj; er += 2) {
			j0 = RLCi[er - 1];
			j1 = RLCi[er];
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
static void step20_algo14_equivalence_build(const CompVMatPtr& ER, const CompVMatPtr& RLC, const CompVMatPtr& ner, CompVMatPtr ERA, const compv_ccl_indice_t w, const compv_ccl_indice_t start, const compv_ccl_indice_t end)
{
	const compv_ccl_indice_t jstart = !start ? 1 : start;
	if (!start) {
		const int16_t ner0 = *ner->ptr<const int16_t>(0);
		compv_ccl_indice_t* ERA0 = ERA->ptr<compv_ccl_indice_t>(0);
		for (int16_t er = 1; er < ner0; er += 2) {
			ERA0[er] = 0;
		}
	}

	const int16_t* ERiminus1 = ER->ptr<const int16_t>(jstart - 1);
	const int16_t* RLCi = RLC->ptr<const int16_t>((jstart - start)); // zero-based (per thread)
	const int16_t* ner0 = ner->ptr<const int16_t>(0, jstart); // zero-based (per row)
	compv_ccl_indice_t* ERAi = ERA->ptr<compv_ccl_indice_t>((jstart - start)); // zero-based (per thread)
	const size_t ER_stride = ER->stride();
	const size_t RLC_stride = RLC->stride();
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
			ERiminus1,  ERi_stride,
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
		COMPV_EXEC_IFDEF_ASM_X64(funPtr_16s32s = CompVConnectedComponentLabelingLSL_Step20Algo14EquivalenceBuild_16s32s_Asm_X64_CMOV);
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
static void step21_algo14_equivalence_build(const CompVMatPtr ner, CompVMatPtr ERA, std::vector<compv_ccl_indice_t>& EQ, compv_ccl_indice_t* nea1, const size_t start, const size_t end)
{
	const size_t jstart = !start ? 1 : start;
	compv_ccl_indice_t nea = *nea1;
	if (!start) {
		const int16_t ner00 = *ner->ptr<int16_t>(0, 0);
		compv_ccl_indice_t* ERA0 = ERA->ptr<compv_ccl_indice_t>(0);
		for (int16_t er = 1; er < ner00; er += 2) {
			ERA0[er] = ++nea; // [new label]
		}
	}
	
	const size_t ERA_stride = ERA->stride();
	const size_t rows = ERA->rows();
	compv_ccl_indice_t* ERAi = ERA->ptr<compv_ccl_indice_t>((jstart - start)); // zero-based (per thread)
	const compv_ccl_indice_t* ERAiminus1 = ERAi - ERA_stride;
	int16_t* ner0 = ner->ptr<int16_t>(0, 0); // zero-based (per row)

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM implementation found");
	for (size_t j = jstart; j < end; ++j) {
		if (j == 159 || j == 160) {
			int kaka = 0;
		}
		const int16_t ner0j = ner0[j];
		for (int16_t er = 1; er < ner0j; er += 2) {
			if (ERAi[er]) {
				const compv_ccl_indice_t er0 = ERAi[er] & 0xffff;
				const compv_ccl_indice_t er1 = (ERAi[er] >> 16) & 0xffff;
				compv_ccl_indice_t ea = ERAiminus1[er0];
				// FIXME(dmi): remove
				if (ea > 100) {
					int kaka = 0;
				}
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
				// FIXME(dmi)
				if (a > 100) {
					int kaka = 0;
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

// 2.4 Equivalence resolution: step#4 (MT friendly)
// EQ, the table holding the equivalence classes, before transitive closure
// A, the associative table of ancestors
// nea, the current number of absolute labels
// na, final number of absolute labels (background not counted)
static void step4_algo6_eq_resolv(const std::vector<compv_ccl_indice_t>& EQ, const compv_ccl_indice_t nea, CompVMatPtr A, compv_ccl_indice_t& na)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No ASM implementation found (cmov)");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No unroll implementation found");
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
	compv_ccl_indice_t* APtr = A->ptr<compv_ccl_indice_t>();
	na = 0;

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Could be nea only instead of size");
	for (compv_ccl_indice_t e = 1; e <= nea; ++e) {
		const compv_ccl_indice_t eq = EQ[e];
		COMPV_ASSERT(eq <= nea); // FIXME(dmi): remove
		APtr[e] = (eq != e)
			? APtr[eq]
			: ++na;
	}
}

static COMPV_ERROR_CODE build_all_labels(const CompVMatPtr& A, const CompVMatPtr& ERA, const CompVMatPtr& ER, CompVMatPtrPtr EA)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation found");
	const size_t ER_width = ER->cols();
	const size_t ER_height = ER->rows();
	const size_t ER_stride = ER->stride();
	const size_t ERA_stride = ERA->stride();

	// Create EA using same size and stride as ER
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_ccl_indice_t>(EA, ER_height, ER_width, ER_stride));

	const compv_ccl_indice_t* APtr = A->ptr<const compv_ccl_indice_t>();
	const compv_ccl_indice_t* ERAPtr = ERA->ptr<const compv_ccl_indice_t>();
	const int16_t* ERPtr = ER->ptr<int16_t>();
	compv_ccl_indice_t* EAPtr = (*EA)->ptr<compv_ccl_indice_t>();

	/* #3 and #5 merged */
	// step #3: First absolute labeling
	// step #5: Second absolute labeling
	for (size_t j = 0; j < ER_height; ++j) {
		for (size_t i = 0; i < ER_width; ++i) {
			COMPV_ASSERT(static_cast<compv_ccl_indice_t>(A->cols()) > ERAPtr[ERPtr[i]]); // FIXME(dmi): remove
			EAPtr[i] = APtr[ERAPtr[ERPtr[i]]];
		}
		EAPtr += ER_stride;
		ERPtr += ER_stride;
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

	static CompVMatPtr ner; // the number of segments of ERi - black + white -
	static CompVMatPtr ER; //  an associative table of size w holding the relative labels er associated to Xi
	static CompVMatPtr ERA; // an associative table holding the association between er and ea: ea = ERAi[er]
	static CompVMatPtr EA; // an image of size h × w of absolute labels ea before equivalence resolution
	static CompVMatPtr A; // the associative table of ancestors
	static std::vector<compv_ccl_indice_t> EQ; // the table holding the equivalence classes, before transitive closure
	compv_ccl_indice_t nea = 0; // the current number of absolute labels, update of EQ and ERAi

	const size_t width = binar->cols();
	const size_t height = binar->rows();
	const size_t stride = binar->stride();
	const size_t __ner_max_per_row = ((width + 1) >> 1); // full dashed row

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("ner_max and ner_sum are local to threads");
	int16_t ner_max; // FIXME(dmi): remove
	compv_ccl_indice_t ner_sum; // FIXME(dmi): remove

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int16_t>(&ER, height, width));
	//COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int16_t>(&RLC, height, __ner_max));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int16_t>(&ner, 1, height));

	/* Multi-threading dispatcher */
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 1;

	/* Create ERA and init with zeros (FIXME(use calloc) */
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No need to keep having ERA, use it as local then destroy or just create one per thread");
	{
		COMPV_DEBUG_INFO_CODE_FOR_TESTING("ner_max hard-coded");
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<compv_ccl_indice_t>(&ERA, height, __ner_max_per_row));
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Do not call zero_all on ERA but use calloc");
		COMPV_CHECK_CODE_RETURN(ERA->zero_all());
	}

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Remove RLC creation");

	/* Step #1 and #20  function pointer */	
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("remove mt_ner_max from the parameters");
	auto funcPtrStep1and20 = [&](const size_t mt_start, const size_t mt_end, int16_t* mt_ner_max, compv_ccl_indice_t* mt_ner_sum, CompVMatPtrPtr ERA_bind) -> COMPV_ERROR_CODE {
		CompVMatPtr RLC, RLC_bind;
		/* Relative segment labeling: step#1 */
		// Step #20 needs RLCi-1 (RLC[start - 1]) which may cross thread bundaries. This is 
		// why in #1 compute [clip(0, start-1), end] range.
		const size_t mt_step1_start = (mt_start ? (mt_start - 1) : mt_start);
		const size_t mt_step1_end = mt_end;
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int16_t>(&RLC, (mt_step1_end - mt_step1_start), __ner_max_per_row));
		step1_algo13_segment_RLE<uint8_t>(
			binar,
			ER,
			RLC,
			ner,
			mt_ner_max,
			mt_ner_sum,
			static_cast<compv_ccl_indice_t>(width),
			static_cast<compv_ccl_indice_t>(mt_step1_start),
			static_cast<compv_ccl_indice_t>(mt_step1_end)
			);

		/* Equivalence construction: step#2.0 (MT-friendly) */
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Here is where to create ERA with cols = *mt_ner_max");
		const CompVRectFloat32 roi_ERA = {
			0.f, // left
			static_cast<compv_float32_t>(mt_start), // top
			static_cast<compv_float32_t>(ERA->cols() - 1), // right
			static_cast<compv_float32_t>(mt_end - 1)// bottom
		};
		const CompVRectFloat32 roi_RLC = {
			0.f, // left
			static_cast<compv_float32_t>(mt_start ? 1 : 0), // top
			static_cast<compv_float32_t>(RLC->cols() - 1), // right
			static_cast<compv_float32_t>(RLC->rows() - 1)// bottom
		};
		COMPV_CHECK_CODE_RETURN(ERA->bind(ERA_bind, roi_ERA));
		COMPV_CHECK_CODE_RETURN(RLC->bind(&RLC_bind, roi_RLC)); // zero-based RLC
		step20_algo14_equivalence_build(
			ER,
			RLC_bind,
			ner,
			*ERA_bind,
			static_cast<compv_ccl_indice_t>(width),
			static_cast<compv_ccl_indice_t>(mt_start),
			static_cast<compv_ccl_indice_t>(mt_end)
		);
		return COMPV_ERROR_CODE_S_OK;
	};

	/* Step #1 and #20 calling process */
	const size_t minSamplePerThreadStep1and20 = std::max(
		static_cast<size_t>(COMPV_CCL_LSL_STEP1_AND_STEP20_MIN_SAMPLES_PER_THREAD),
		(width << 1) // At least #2 rows (because of ERiminus1)
	);
	const size_t threadsCountStep1and20 = (threadDisp && !threadDisp->isMotherOfTheCurrentThread())
		? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(width, height, maxThreads, minSamplePerThreadStep1and20)
		: 1;
	if (threadsCountStep1and20 > 1) {
		CompVAsyncTaskIds taskIds;
		taskIds.reserve(threadsCountStep1and20);
		std::vector<int16_t> mt_ner_max(threadsCountStep1and20);
		std::vector<compv_ccl_indice_t> mt_ner_sum(threadsCountStep1and20);
		std::vector<std::pair<size_t, size_t> >mt_ranges(threadsCountStep1and20);
		std::vector<CompVMatPtr > mt_ERAs(threadsCountStep1and20);
		const size_t heights = (height / threadsCountStep1and20) & ~1; // must be even to make sure ERA and ERAiminus1 will be on the same thread
		size_t YStart = 0, YEnd;
		for (size_t threadIdx = 0; threadIdx < threadsCountStep1and20; ++threadIdx) {
			YEnd = (threadIdx == (threadsCountStep1and20 - 1)) ? height : (YStart + heights);
			COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrStep1and20, static_cast<compv_ccl_indice_t>(YStart), static_cast<compv_ccl_indice_t>(YEnd), &mt_ner_max[threadIdx], &mt_ner_sum[threadIdx], &mt_ERAs[threadIdx]),
				taskIds), "Dispatching task failed");
			mt_ranges[threadIdx].first = YStart;
			mt_ranges[threadIdx].second = YEnd;
			YStart += heights;
		}
		/* Wait for result and build equivalance for each thread */
		ner_max = 0;
		ner_sum = 0;
		for (size_t threadIdx = 0; threadIdx < threadsCountStep1and20; ++threadIdx) {
			COMPV_CHECK_CODE_RETURN(threadDisp->waitOne(taskIds[threadIdx]));
			const compv_ccl_indice_t new_ner_sum = mt_ner_sum[threadIdx];
			CompVMatPtr mt_ERA = mt_ERAs[threadIdx];

			COMPV_DEBUG_INFO_CODE_FOR_TESTING("No need to compute ner_max");
			if (ner_max < mt_ner_max[threadIdx]) {
				ner_max = mt_ner_max[threadIdx];
			}
			if (new_ner_sum) {
				const size_t mt_start = mt_ranges[threadIdx].first;
				const size_t mt_end = mt_ranges[threadIdx].second;
				/* Build EQ */
				if (!threadIdx) {
					EQ.resize(new_ner_sum);
					std::iota(EQ.begin(), EQ.end(), 0);
				}
				else {
					std::vector<compv_ccl_indice_t > mt_EQ(new_ner_sum);
					std::iota(mt_EQ.begin(), mt_EQ.end(), ner_sum);
					EQ.insert(EQ.end(), mt_EQ.begin(), mt_EQ.end());
				}
				/* Equivalence construction: step#2.1 (not MT-friendly) */
				step21_algo14_equivalence_build(
					ner,
					mt_ERA,
					EQ,
					&nea,
					mt_start,
					mt_end
				);
			}
			ner_sum += new_ner_sum;
		}

#if 0
		EQ.resize(ner_sum);
		std::iota(EQ.begin(), EQ.end(), 0);

		/* Equivalence construction: step#2.1 (not MT-friendly) */
		step21_algo14_equivalence_build(
			ner,
			ERA,
			EQ,
			&nea,
			0,
			height
		);
#endif

		/*for (size_t threadIdx = 0; threadIdx < threadsCountStep1and20; ++threadIdx) {
			const size_t mt_start = mt_ranges[threadIdx].first;
			const size_t mt_end = mt_ranges[threadIdx].second;
			CompVMatPtr mt_ERABind;

			const CompVRectFloat32 roi_ERA = {
				0.f, // left
				static_cast<compv_float32_t>(mt_start ? (mt_start - 1) : mt_start), // top
				static_cast<compv_float32_t>(ERA->cols() - 1), // right
				static_cast<compv_float32_t>(mt_end - 1)// bottom
			};
			COMPV_CHECK_CODE_RETURN(ERA->bind(&mt_ERABind, roi_ERA));

			step21_algo14_equivalence_build(
				ner,
				mt_ERABind,
				EQ,
				&nea,
				mt_start ? (mt_start - 1) : mt_start,
				(mt_start ? (mt_start - 1) : mt_start) + mt_ERABind->rows()
			);
		}*/
		
		/* Equivalence construction: step#2.1 (not MT-friendly) */
		/*step21_algo14_equivalence_build(
			ner,
			ERA,
			EQ,
			&nea,
			0,
			height
		);*/
	}
	else {
		CompVMatPtr ERA_bind; // useless for know
		COMPV_CHECK_CODE_RETURN(funcPtrStep1and20(0, static_cast<compv_ccl_indice_t>(height), &ner_max, &ner_sum, &ERA_bind));
		/* Build EQ */
		EQ.resize(ner_sum);
		std::iota(EQ.begin(), EQ.end(), 0);

		/* Equivalence construction: step#2.1 (not MT-friendly) */
		step21_algo14_equivalence_build(
			ner,
			ERA,
			EQ,
			&nea,
			0,
			height
		);
	}

	//COMPV_DEBUG_INFO("MD5-Binar=%s", CompVMd5::compute2(binar->ptr(), binar->dataSizeInBytes()).c_str());
	//COMPV_DEBUG_INFO("MD5-ER=%s", CompVMd5::compute2(ER->ptr(), ER->dataSizeInBytes()).c_str());
	//COMPV_DEBUG_INFO("MD5-ner=%s", CompVMd5::compute2(ner->ptr(), ner->dataSizeInBytes()).c_str());
	//COMPV_DEBUG_INFO("MD5-ERA=%s", CompVMd5::compute2(ERA->ptr(), ERA->dataSizeInBytes()).c_str());

	COMPV_ASSERT(ner_max < __ner_max_per_row); // FIXME(dmi): remove

#if 0
	/* Build EQ */
	EQ.resize(ner_sum);
	std::iota(EQ.begin(), EQ.end(), 0);

	/* Equivalence construction: step#2.1 (not MT-friendly) */
	step21_algo14_equivalence_build(
		ner,
		ERA,
		EQ,
		&nea
	);
#endif
	COMPV_ASSERT(nea < ner_sum); // FIXME(dmi): remove

	/* Create A and init first element with zero (because bacground label is equal to zero) */
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<compv_ccl_indice_t>(&A, 1, (nea + 1)));
	*A->ptr<compv_ccl_indice_t>(0, 0) = 0; // other values will be initialzed in step4_algo6_eq_resolv

	/* Equivalence resolution: step#4 */
	compv_ccl_indice_t na = 0; // final number of absolute labels
	step4_algo6_eq_resolv(
		EQ,
		nea,
		A,
		na
	);

	/* For testing */
	//build_all_labels(A, ERA, ER, &EA);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Directly write to result.labels");
	result.labels = EA;
	result.labels_count = (na + 1); // +1 for the background

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
