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
	const compv_ccl_lea_n_t& vecLEA,
	const CompVMatPtr& ptr32sA,
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
		m_vecLEA.size() != m_szInput.height ||
		!m_ptr32sA || m_ptr32sA->rows() != 1,
		COMPV_ERROR_CODE_E_INVALID_STATE
	);

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("This function is needed for debugging only and you should not call it");

	COMPV_DEBUG_INFO_CODE_TODO("Use scallable allocator for vecLEA");

	// Create EA (image of absolute labels)
	CompVMatPtr ptr32sEA = *ptr32sLabels;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjStrideless<int32_t>(&ptr32sEA, m_szInput.height, m_szInput.width));

	const size_t EA_stride = ptr32sEA->stride();

	// #3 and #5 merged 
	// step #3: First absolute labeling
	// step #5: Second absolute labeling
	auto funcPtr = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		const int32_t* A = m_ptr32sA->ptr<const int32_t>(0, 0);
		int32_t* EA = ptr32sEA->ptr<int32_t>(ystart);
		compv_ccl_lea_1_t::const_iterator it;
		for (size_t j = ystart; j < yend; ++j) {
			COMPV_CHECK_CODE_RETURN(ptr32sEA->zero_row(j)); // background is "0"
			const compv_ccl_lea_1_t& lea = m_vecLEA[j];
			for (it = lea.begin(); it < lea.end(); ++it) {
				const int32_t a = A[it->ea]; // a within [1, na]
				const int16_t er0 = it->start;
				const int16_t er1 = it->end;
				for (int16_t er = er0; er < er1; ++er) {
					EA[er] = a;
				}
			}
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
		m_vecLEA.size() != m_szInput.height ||
		!m_ptr32sA || m_ptr32sA->rows() != 1,
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
			m_vecLEA,
			m_ptr32sA,
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
	const compv_ccl_accumulator_t* ptrxNaPtr = ptrxNa->ptr();
	points.resize(m_nNa1);
	for (size_t na = 1; na <= m_nNa1; ++na) { // a within [1, na]
		const size_t count = static_cast<size_t>(ptrxNaPtr[na - 1]);
		COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<compv_float32_t>(&points[na - 1], 2, count));
	}
	std::vector<compv_ccl_accumulator_t > szFilled(m_nNa1, 0);
	const int32_t* A = m_ptr32sA->ptr<const int32_t>(0, 0);
	auto funcPtrFill = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		compv_ccl_lea_1_t::const_iterator it;
		for (size_t j = ystart; j < yend; ++j) {
			const compv_ccl_lea_1_t& lea = m_vecLEA[j];
			const compv_float32_t y32f = static_cast<compv_float32_t>(j);
			for (it = lea.begin(); it < lea.end(); ++it) {
				const int32_t a = (A[it->ea] - 1); // a within [1, na]
				CompVMatPtr& pp = points[a];
				const size_t count = static_cast<size_t>(it->end - it->start);
				const size_t count4 = count & -4;
				const size_t gOld = static_cast<size_t>(compv_atomic_add(&szFilled[a], static_cast<compv_ccl_accumulator_t>(count)));
				compv_float32_t x32f = static_cast<compv_float32_t>(it->start);
				compv_float32_t* x32fPtr = pp->data<compv_float32_t>() + gOld;
				compv_float32_t* y32fPtr = x32fPtr + pp->stride();
				size_t i;
				for (i = 0; i < count4; i+=4, x32f+=4.f) {
					x32fPtr[i] = x32f;
					x32fPtr[i + 1] = x32f + 1.f;
					x32fPtr[i + 2] = x32f + 2.f;
					x32fPtr[i + 3] = x32f + 3.f;

					y32fPtr[i] = y32f;
					y32fPtr[i + 1] = y32f;
					y32fPtr[i + 2] = y32f;
					y32fPtr[i + 3] = y32f;
				}
				for (; i < count; ++i, ++x32f) {
					x32fPtr[i] = x32f;
					y32fPtr[i] = y32f;
				}
			}
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
	const compv_ccl_lea_n_t& vecLEA,
	const CompVMatPtr& ptr32sA,
	const size_t ystart,
	const size_t yend)
{
	compv_ccl_accumulator_t* naPtr = ptrxNa->ptr(0, 0);
	const int32_t* A = ptr32sA->ptr<const int32_t>(0, 0);

	// #3 and #5 merged 
	// step #3: First absolute labeling
	// step #5: Second absolute labeling
	compv_ccl_lea_1_t::const_iterator it;
	for (size_t j = ystart; j < yend; ++j) {
		const compv_ccl_lea_1_t& lea = vecLEA[j];
		for (it = lea.begin(); it < lea.end(); ++it) {
			const int32_t a = (A[it->ea] - 1); // a within [1, na]
			const compv_ccl_accumulator_t v = (it->end - it->start);
			compv_atomic_add(&naPtr[a], v);
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
