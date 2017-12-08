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


#define COMPV_THIS_CLASSNAME	"CompVConnectedComponentLabelingLSL"

COMPV_NAMESPACE_BEGIN()

static const int kCompVConnectedComponentLabelingLSLBachgroundLabel = 0; // Must be zero because of calloc()

CompVConnectedComponentLabelingLSL::CompVConnectedComponentLabelingLSL()
	:CompVConnectedComponentLabeling(static_cast<int>(COMPV_LSL_ID))
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

// Relative segment labeling: step#1
// Algorithm 12: LSL segment detection STD
// Xi: a binary line of width w
// ERi, an associative table of size w holding the relative labels er associated to Xi
// RLCi, a table holding the run length coding of segments of the line Xi
// ner, the number of segments of ERi – black + white
template<typename T>
static void step1_algo13_segment_RLC(const T* Xi, int* ERi, int* RLCi, int* ner1, const int w)
{
	int x1 = 0; // previous value of X
	int f = 0; //  front detection
	int b = 0; // right border compensation
	int er = 0; // a relative label
	int x0;

	for (int i = 0; i < w; ++i) {
		x0 = Xi[i] & 1; // Xi must be binary (allowed values: 0x01, 0xff, 0x00)
		f = x0 ^ x1;
		if (f != 0) {
			RLCi[er] = (i - b);
			b = b ^ 1; // b ^ f
			er = er + 1;
			COMPV_ASSERT(er < (w >> 1)); // FIXME(dmi): remove
		}
		ERi[i] = er;
		x1 = x0;
	}
	x0 = 0;
	f = x0 ^ x1;
	RLCi[er] = (w - b);
	er = er + f;
	*ner1 = er;
}

// 2.2 Equivalence construction: step#2
// Algorithm 14: LSL equivalence construction
// ERi, an associative table of size w holding the relative labels er associated to Xi
// RLCi, a table holding the run length coding of segments of the line Xi
// EQ,  the table holding the equivalence classes, before transitive closure
// ERAi, an associative table holding the association between er and ea: ea = ERAi[er]
// ner, the number of segments of ERi - black + white
// nea the current number of absolute labels, update of EQ and ERAi
static void step2_algo14_equivalence_build(const int* ERiminus1, const int* RLCi, const int ner, const int w, int* ERAiminus1, int* ERAi, int* EQ, int* nea1)
{
	const int wminus1 = (w - 1);
	int nea = *nea1;
	for (int er = 1; er < ner; er += 2) {
		int j0 = RLCi[er - 1];
		int j1 = RLCi[er];
		// [check extension in case of 8-connect algorithm]
		if (j0 > 0) {
			j0 = j0 - 1;
		}
		if (j1 < wminus1) {
			j1 = j1 + 1;
		}
		int er0 = ERiminus1 ? ERiminus1[j0] : 0;
		int er1 = ERiminus1 ? ERiminus1[j1] : 0;
		// [check label parity: segments are odd]
		if (!(er0 & 1)) { // er0 is even?
			er0 = er0 + 1;
		}
		if (!(er1 & 1)) { // er1 is even?
			er1 = (er1 - 1); // sub_sat()
		}
		if (er1 >= er0) {
			int ea = ERAiminus1[er0];
			int a = EQ[ea]; // FindRoot(ea) // EQ must be initialized with zeros
			for (int erk = er0 + 2; erk <= er1; erk += 2) { // TODO(dmi): step 1 or 2 ?
				int eak = ERAiminus1[erk];
				int ak = EQ[eak]; // FindRoot(eak)
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
			ERAi[er] = a; // the global min
		}
		else {
			// [new label]
			nea = nea + 1;
			ERAi[er] = nea;
		}
	}

	*nea1 = nea;
}

// 2.4 Equivalence resolution: step#4
// EQ, the table holding the equivalence classes, before transitive closure
// A, the associative table of ancestors
// nea, the current number of absolute labels
// na, final number of absolute labels (background not counted)
static void step4_algo6_eq_resolv(const CompVMatPtr& EQ, const int nea, CompVMatPtr A, int& na)
{
	const int* EQPtr = EQ->ptr<const int>();
	int* APtr = A->ptr<int>();
	na = 0;

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Could be nea only instead of size");
	for (int e = 1; e <= nea; ++e) {
		const int eq = EQPtr[e];
		if (eq != e) {
			APtr[e] = APtr[eq];
		}
		else {
			APtr[e] = ++na;
		}
	}
}

// Second absolute labeling: step#5
// EA, an image of size h × w of absolute labels ea before equivalence resolution
// A, the associative table of ancestors
static void step5_algo16_2st_abs_labeling(const CompVMatPtr& A, CompVMatPtr EA)
{
	const size_t EA_width = EA->cols();
	const size_t EA_height = EA->rows();
	const size_t EA_stride = EA->stride();

	const int* APtr = A->ptr<const int>();
	int* EAPtr = EA->ptr<int>();

	for (size_t j = 0; j < EA_height; ++j) {
		for (size_t i = 0; i < EA_width; ++i) {
			EAPtr[i] = APtr[EAPtr[i]];
		}
		EAPtr += EA_stride;
	}
}

static COMPV_ERROR_CODE build_all_labels(const CompVMatPtr& A, const CompVMatPtr& ERA, const CompVMatPtr& ER, CompVMatPtrPtr EA)
{
	const size_t ER_width = ER->cols();
	const size_t ER_height = ER->rows();
	const size_t ER_stride = ER->stride();
	const size_t ERA_stride = ERA->stride();

	// Create EA using same size and stride as ER
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<int>(EA, ER_height, ER_width, ER_stride));

	const int* APtr = A->ptr<const int>();
	const int* ERAPtr = ERA->ptr<const int>();
	const int* ERPtr = ER->ptr<int>();
	int* EAPtr = (*EA)->ptr<int>();

	// step #3: First absolute labeling
	for (size_t j = 0; j < ER_height; ++j) {
		for (size_t i = 0; i < ER_width; ++i) {
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

	CompVMatPtr ner; // the number of segments of ERi - black + white -
	CompVMatPtr ER; //  an associative table of size w holding the relative labels er associated to Xi
	CompVMatPtr RLC; // a table holding the run length coding of segments of the line Xi, RLCi-1 is the similar memorization of the previous line.
	CompVMatPtr ERA; // an associative table holding the association between er and ea: ea = ERAi[er]
	CompVMatPtr EQ; // the table holding the equivalence classes, before transitive closure
	CompVMatPtr EA; // an image of size h × w of absolute labels ea before equivalence resolution
	CompVMatPtr A; // the associative table of ancestors

	const size_t width = binar->cols();
	const size_t height = binar->rows();
	const size_t stride = binar->stride();
	const size_t __ner_max = ((width + 1) >> 1); // full dashed row

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int>(&ER, height, width));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int>(&RLC, height, __ner_max));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int>(&ner, 1, height));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int>(&EQ, 1, (height * (width >> 2))));

	/* Relative segment labeling: step#1 */
	int ner_max = 0;
	for (size_t j = 0; j < height; ++j) {
		const uint8_t* Xi = binar->ptr<const uint8_t>(j);
		int* ERi = ER->ptr<int>(j);
		int* RLCi = RLC->ptr<int>(j);
		int* ner1 = ner->ptr<int>(0, j);
		step1_algo13_segment_RLC<uint8_t>(
			Xi,
			ERi,
			RLCi,
			ner1,
			static_cast<int>(width)
			);
		if (ner_max < *ner1) {
			ner_max = *ner1;
		}
	}

	/* Init EQ with 0...n (itoa) */
	{
		int* EQPtr = EQ->ptr<int>();
		const int n = static_cast<int>(EQ->cols());
		for (int i = 0; i < n; ++i) {
			EQPtr[i] = i;
		}
	}
	/* Create ERA and init with zeros */
	{
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int>(&ERA, height, ner_max));
		COMPV_CHECK_CODE_RETURN(ERA->zero_all());
	}

	/* Equivalence construction: step#2 */
	int nea = 0; // the current number of absolute labels, update of EQ and ERAi
	for (size_t j = 0; j < height; ++j) {
		const int* ner1 = ner->ptr<const int>(0, j);
		const int* ERiminus1 = j ? ER->ptr<const int>(j - 1) : nullptr;
		const int* RLCi = RLC->ptr<const int>(j);
		int* ERAiminus1 = j ? ERA->ptr<int>(j - 1) : nullptr;
		int* ERAi = ERA->ptr<int>(j);
		step2_algo14_equivalence_build(
			ERiminus1,
			RLCi,
			*ner1,
			static_cast<int>(width),
			ERAiminus1,
			ERAi,
			EQ->ptr<int>(),
			&nea
		);
	}

	/* Create A and init with zeros (because bacground label is equal to zero) */
	{
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int>(&A, EQ->rows(), EQ->cols()));
		COMPV_CHECK_CODE_RETURN(A->zero_all());
	}

	/* Equivalence resolution: step#4 */
	int na = 0; // final number of absolute labels
	step4_algo6_eq_resolv(
		EQ,
		nea,
		A,
		na
	);

	/* For testing */
	build_all_labels(A, ERA, ER, &EA);

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
