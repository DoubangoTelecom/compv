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

// Relative segment labeling: step#1
// Algorithm 12: LSL segment detection STD
// Xi: a binary line of width w (allowed values: 0x01, 0xff, 0x00)
// ERi, an associative table of size w holding the relative labels er associated to Xi
// RLCi, a table holding the run length coding of segments of the line Xi
// ner, the number of segments of ERi – black + white
template<typename T>
static void step1_algo13_segment_RLC(const T* Xi, compv_ccl_indice_t* ERi, compv_ccl_indice_t* RLCi, compv_ccl_indice_t* ner1, const compv_ccl_indice_t w)
{
	/* For i = 0 */
	compv_ccl_indice_t b = Xi[0] & 1; // right border compensation
	compv_ccl_indice_t er = b; // a relative label
	RLCi[0] = 0;
	ERi[0] = er;

	/* i = 1....w */
	for (compv_ccl_indice_t i = 1; i < w; ++i) {
		if (Xi[i] ^ Xi[i - 1]) {
			RLCi[er] = (i - b);
			b ^= 1; // b ^ f
			++er;
			COMPV_ASSERT(er < (w >> 1)); // FIXME(dmi): remove
		}
		ERi[i] = er;
	}
	RLCi[er] = (w - b);
	*ner1 = er + (Xi[w - 1] & 1);
}

// 2.2 Equivalence construction: step#2
// Algorithm 14: LSL equivalence construction
// ERi, an associative table of size w holding the relative labels er associated to Xi
// RLCi, a table holding the run length coding of segments of the line Xi
// EQ,  the table holding the equivalence classes, before transitive closure
// ERAi, an associative table holding the association between er and ea: ea = ERAi[er]
// ner, the number of segments of ERi - black + white
// nea the current number of absolute labels, update of EQ and ERAi
static void step2_algo14_equivalence_build(const compv_ccl_indice_t* ERiminus1, const compv_ccl_indice_t* RLCi, const compv_ccl_indice_t ner, const compv_ccl_indice_t w, compv_ccl_indice_t* ERAiminus1, compv_ccl_indice_t* ERAi, compv_ccl_indice_t* EQ, compv_ccl_indice_t* nea1)
{
	const compv_ccl_indice_t wminus1 = (w - 1);
	compv_ccl_indice_t nea = *nea1;
	for (compv_ccl_indice_t er = 1; er < ner; er += 2) {
		compv_ccl_indice_t j0 = RLCi[er - 1];
		compv_ccl_indice_t j1 = RLCi[er];
		// [check extension in case of 8-connect algorithm]
		j0 -= (j0 > 0);		
		j1 += (j1 < wminus1);
		compv_ccl_indice_t er0 = ERiminus1 ? ERiminus1[j0] : 0;
		compv_ccl_indice_t er1 = ERiminus1 ? ERiminus1[j1] : 0;
		// [check label parity: segments are odd]
		er0 += ((er0 & 1) ^ 1);
		er1 -= ((er1 & 1) ^ 1);
		if (er1 >= er0) {
			compv_ccl_indice_t ea = ERAiminus1[er0];
			compv_ccl_indice_t a = EQ[ea]; // FindRoot(ea)
			for (compv_ccl_indice_t erk = er0 + 2; erk <= er1; erk += 2) {
				const compv_ccl_indice_t eak = ERAiminus1[erk];
				const compv_ccl_indice_t ak = EQ[eak]; // FindRoot(eak)
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
			ERAi[er] = ++nea;
		}
	}

	*nea1 = nea;
}

// 2.4 Equivalence resolution: step#4
// EQ, the table holding the equivalence classes, before transitive closure
// A, the associative table of ancestors
// nea, the current number of absolute labels
// na, final number of absolute labels (background not counted)
static void step4_algo6_eq_resolv(const CompVMatPtr& EQ, const compv_ccl_indice_t nea, CompVMatPtr A, compv_ccl_indice_t& na)
{
	const compv_ccl_indice_t* EQPtr = EQ->ptr<const compv_ccl_indice_t>();
	compv_ccl_indice_t* APtr = A->ptr<compv_ccl_indice_t>();
	na = 0;

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Could be nea only instead of size");
	for (compv_ccl_indice_t e = 1; e <= nea; ++e) {
		const compv_ccl_indice_t eq = EQPtr[e];
		COMPV_ASSERT(eq <= nea); // FIXME(dmi): remove
		APtr[e] = (eq != e)
			? APtr[eq]
			: ++na;
	}
}

static COMPV_ERROR_CODE build_all_labels(const CompVMatPtr& A, const CompVMatPtr& ERA, const CompVMatPtr& ER, CompVMatPtrPtr EA)
{
	const size_t ER_width = ER->cols();
	const size_t ER_height = ER->rows();
	const size_t ER_stride = ER->stride();
	const size_t ERA_stride = ERA->stride();

	// Create EA using same size and stride as ER
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_ccl_indice_t>(EA, ER_height, ER_width, ER_stride));

	const compv_ccl_indice_t* APtr = A->ptr<const compv_ccl_indice_t>();
	const compv_ccl_indice_t* ERAPtr = ERA->ptr<const compv_ccl_indice_t>();
	const compv_ccl_indice_t* ERPtr = ER->ptr<compv_ccl_indice_t>();
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

	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<compv_ccl_indice_t>(&ER, height, width));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<compv_ccl_indice_t>(&RLC, height, __ner_max));
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<compv_ccl_indice_t>(&ner, 1, height));

	/* Relative segment labeling: step#1 */
	compv_ccl_indice_t ner_max = 0, ner_sum = 0;
	for (size_t j = 0; j < height; ++j) {
		const uint8_t* Xi = binar->ptr<const uint8_t>(j);
		compv_ccl_indice_t* ERi = ER->ptr<compv_ccl_indice_t>(j);
		compv_ccl_indice_t* RLCi = RLC->ptr<compv_ccl_indice_t>(j);
		compv_ccl_indice_t* ner1 = ner->ptr<compv_ccl_indice_t>(0, j);
		step1_algo13_segment_RLC<uint8_t>(
			Xi,
			ERi,
			RLCi,
			ner1,
			static_cast<compv_ccl_indice_t>(width)
			);
		if (ner_max < *ner1) {
			ner_max = *ner1;
		}
		ner_sum += *ner1;
	}
	
	/* Create ERA and init with zeros */
	{
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<compv_ccl_indice_t>(&ERA, height, ner_max));
		COMPV_CHECK_CODE_RETURN(ERA->zero_all());
	}

	/* Create EQ and init with 0...n (itoa) */
	{
		size_t EQ_count = COMPV_MATH_MIN(static_cast<size_t>(ner_sum), (height * (width >> 2)));
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<compv_ccl_indice_t>(&EQ, 1, EQ_count));
		compv_ccl_indice_t* EQPtr = EQ->ptr<compv_ccl_indice_t>();
		const compv_ccl_indice_t n = static_cast<compv_ccl_indice_t>(EQ_count);
		for (compv_ccl_indice_t i = 0; i < n; ++i) {
			EQPtr[i] = i;
		}
	}

	/* Equivalence construction: step#2 */
	compv_ccl_indice_t nea = 0; // the current number of absolute labels, update of EQ and ERAi
	for (size_t j = 0; j < height; ++j) {
		const compv_ccl_indice_t* ner1 = ner->ptr<const compv_ccl_indice_t>(0, j);
		const compv_ccl_indice_t* ERiminus1 = j ? ER->ptr<const compv_ccl_indice_t>(j - 1) : nullptr;
		const compv_ccl_indice_t* RLCi = RLC->ptr<const compv_ccl_indice_t>(j);
		compv_ccl_indice_t* ERAiminus1 = j ? ERA->ptr<compv_ccl_indice_t>(j - 1) : nullptr;
		compv_ccl_indice_t* ERAi = ERA->ptr<compv_ccl_indice_t>(j);
		step2_algo14_equivalence_build(
			ERiminus1,
			RLCi,
			*ner1,
			static_cast<compv_ccl_indice_t>(width),
			ERAiminus1,
			ERAi,
			EQ->ptr<compv_ccl_indice_t>(),
			&nea
		);
	}
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
