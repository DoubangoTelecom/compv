/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/ccl/compv_core_ccl_lsl_result.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_debug.h"

#include <atomic>

#define COMPV_CCL_LSL_FLATTEN_MIN_SAMPLES_PER_THREAD			(20*20)
#define COMPV_CCL_LSL_EXTRACT_COUNT_MIN_SAMPLES_PER_THREAD		(20*20)
#define COMPV_CCL_LSL_EXTRACT_FILL_MIN_SAMPLES_PER_THREAD		(20*20)

#define COMPV_THIS_CLASSNAME "CompVConnectedComponentLabelingResultLSLImpl"

COMPV_NAMESPACE_BEGIN()

#if COMPV_OS_WINDOWS
typedef LONG compv_ccl_accumulator_t; /* InterlockedDecrement/InterlockedIncrement requires LONG */
#else
typedef int compv_ccl_accumulator_t;
#endif
typedef CompVMemZero<compv_ccl_accumulator_t > CompVMemZeroLockedAccumulator;
typedef CompVPtr<CompVMemZeroLockedAccumulator *> CompVMemZeroLockedAccumulatorPtr;

static COMPV_ERROR_CODE count_points(
	CompVMemZeroLockedAccumulatorPtr ptrxNa,
	const compv_ccl_lea_t& vec32sLEA,
	const CompVMatPtr& ptr32sA,
	const CompVMatPtr& ptr16sRLC,
	const size_t ystart,
	const size_t yend);

CompVConnectedComponentLabelingResultLSLImpl::CompVConnectedComponentLabelingResultLSLImpl()
	: m_szInput(CompVSizeSz(0, 0))
	, m_nNa1(0)
{
}

CompVConnectedComponentLabelingResultLSLImpl::~CompVConnectedComponentLabelingResultLSLImpl()
{

}

size_t CompVConnectedComponentLabelingResultLSLImpl::labelsCount() const
{
	return static_cast<size_t>(m_nNa1);
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::debugFlatten(CompVMatPtrPtr ptr32sLabels) const
{
	COMPV_CHECK_EXP_RETURN(!ptr32sLabels, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(
		!m_szInput.width || !m_szInput.height ||
		m_vec32sLEA.size() != m_szInput.height ||
		!m_ptr32sA || m_ptr32sA->rows() != 1 ||
		!m_ptr16sRLC || m_ptr16sRLC->rows() != m_szInput.height || m_ptr16sRLC->cols() >= m_szInput.width,
		COMPV_ERROR_CODE_E_INVALID_STATE
	);

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("This function is needed for debugging only and you should not call it");

	// Create EA (image of absolute labels)
	CompVMatPtr ptr32sEA = *ptr32sLabels;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int32_t>(&ptr32sEA, m_szInput.height, m_szInput.width));

	const size_t EA_stride = ptr32sEA->stride();
	const size_t RLC_stride = m_ptr16sRLC->stride();

	// #3 and #5 merged 
	// step #3: First absolute labeling
	// step #5: Second absolute labeling
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const int16_t* RLC = m_ptr16sRLC->ptr<const int16_t>(ystart);
		const int32_t* A = m_ptr32sA->ptr<const int32_t>(0, 0);
		int32_t* EA = ptr32sEA->ptr<int32_t>(ystart);
		int16_t er;
		CompVVec32s::const_iterator it;
		for (size_t j = ystart; j < yend; ++j) {
			COMPV_CHECK_CODE_RETURN(ptr32sEA->zero_row(j)); // background is "0"
			const CompVVec32s& lea = m_vec32sLEA[j];
			for (er = 1, it = lea.begin(); it < lea.end(); er += 2, ++it) {
				const int32_t a = A[*it]; // A[ea]
				const int16_t er0 = RLC[er - 1];
				const int16_t er1 = RLC[er];
				for (int16_t e = er0; e < er1; ++e) {
					EA[e] = a;
				}
			}
			RLC += RLC_stride;
			EA += EA_stride;
		}
		return COMPV_ERROR_CODE_S_OK;
	};

	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtr,
		m_szInput.width,
		m_szInput.height,
		COMPV_CCL_LSL_FLATTEN_MIN_SAMPLES_PER_THREAD
	));

	*ptr32sLabels = ptr32sEA;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::extract(std::vector<CompVMatPtr>& points) const
{
	COMPV_CHECK_EXP_RETURN(
		!m_szInput.width || !m_szInput.height ||
		m_vec32sLEA.size() != m_szInput.height ||
		!m_ptr32sA || m_ptr32sA->rows() != 1 ||
		!m_ptr16sRLC || m_ptr16sRLC->rows() != m_szInput.height || m_ptr16sRLC->cols() >= m_szInput.width,
		COMPV_ERROR_CODE_E_INVALID_STATE
	);

	points.clear();

	if (!labelsCount()) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No point to extract");
		return COMPV_ERROR_CODE_S_OK;
	}

	// LSL can compute the blob features without extracting the points
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You don't need to extract the points in order to compute the blob features (bounding boxes, moments...)");

	/* Count the number of points (required for features computation) */
	CompVMemZeroLockedAccumulatorPtr ptrxNa; // final number of absolute labels (only needed to extract points)
	COMPV_CHECK_CODE_RETURN(CompVMemZeroLockedAccumulator::newObj(&ptrxNa, 1, m_nNa1));
	auto funcPtrCountPoints = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		COMPV_CHECK_CODE_RETURN(count_points(
			ptrxNa,
			m_vec32sLEA,
			m_ptr32sA,
			m_ptr16sRLC,
			ystart,
			yend)
		);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrCountPoints,
		m_szInput.width,
		m_szInput.height,
		COMPV_CCL_LSL_EXTRACT_COUNT_MIN_SAMPLES_PER_THREAD
	));

	/* Fill points */
	const size_t RLC_stride = m_ptr16sRLC->stride();
	const compv_ccl_accumulator_t* ptrxNaPtr = ptrxNa->ptr();
	points.resize(m_nNa1);
	for (size_t na = 1; na <= m_nNa1; ++na) { // a within [1, na]
		const size_t count = static_cast<size_t>(ptrxNaPtr[na - 1]);
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&points[na - 1], 2, count));
	}
	std::vector<compv_ccl_accumulator_t > szFilled(m_nNa1, 0);
	const int32_t* A = m_ptr32sA->ptr<const int32_t>(0, 0);
	auto funcPtrFill = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const int16_t* RLC = m_ptr16sRLC->ptr<const int16_t>(ystart);
		int16_t er;
		CompVVec32s::const_iterator it;
		for (size_t j = ystart; j < yend; ++j) {
			const CompVVec32s& lea = m_vec32sLEA[j];
			const compv_float32_t y32f = static_cast<compv_float32_t>(j);
			for (er = 1, it = lea.begin(); it < lea.end(); er += 2, ++it) {
				const int32_t a = (A[*it] - 1); // a within [1, na]
				compv_ccl_accumulator_t* gCount = &szFilled[a];
				CompVMatPtr& pp = points[a];
				const size_t count = static_cast<size_t>(RLC[er] - RLC[er - 1]);
				const size_t gOld = static_cast<size_t>(compv_atomic_add(gCount, static_cast<compv_ccl_accumulator_t>(count)));
				compv_float32_t x32f = static_cast<compv_float32_t>(RLC[er - 1]);
				compv_float32_t* x32fPtr = pp->ptr<compv_float32_t>(0, gOld);
				compv_float32_t* y32fPtr = pp->ptr<compv_float32_t>(1, gOld);
				for (size_t i = 0; i < count; ++i) {
					x32fPtr[i] = x32f++;
					y32fPtr[i] = y32f;
				}
			}
			RLC += RLC_stride;
		}
		return COMPV_ERROR_CODE_S_OK;
	};

	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrFill,
		m_szInput.width,
		m_szInput.height,
		COMPV_CCL_LSL_EXTRACT_FILL_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::boundingBoxes() const
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::firstOrderMoment() const
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::reset()
{
	m_nNa1 = 0;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::newObj(CompVConnectedComponentLabelingResultLSLImplPtrPtr result)
{
	COMPV_CHECK_EXP_RETURN(!result, COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	CompVConnectedComponentLabelingResultLSLImplPtr result_ = new CompVConnectedComponentLabelingResultLSLImpl();
	COMPV_CHECK_EXP_RETURN(!result_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*result = result_;
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE count_points(
	CompVMemZeroLockedAccumulatorPtr ptrxNa,
	const compv_ccl_lea_t& vec32sLEA,
	const CompVMatPtr& ptr32sA,
	const CompVMatPtr& ptr16sRLC,
	const size_t ystart,
	const size_t yend)
{
	compv_ccl_accumulator_t* naPtr = ptrxNa->ptr(0, 0);
	const int32_t* A = ptr32sA->ptr<const int32_t>(0, 0);
	const int16_t* RLC = ptr16sRLC->ptr<const int16_t>(ystart);
	const size_t RLC_stride = ptr16sRLC->stride();

	// #3 and #5 merged 
	// step #3: First absolute labeling
	// step #5: Second absolute labeling
	int16_t er;
	CompVVec32s::const_iterator it;
	for (size_t j = ystart; j < yend; ++j) {
		const CompVVec32s& lea = vec32sLEA[j];
		for (er = 1, it = lea.begin(); it < lea.end(); er += 2, ++it) {
			const int32_t a = (A[*it] - 1); // a within [1, na]
			const compv_ccl_accumulator_t v = (RLC[er] - RLC[er - 1]);
			compv_atomic_add(&naPtr[a], v);
		}
		RLC += RLC_stride;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
