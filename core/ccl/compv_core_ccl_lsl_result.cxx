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

#define COMPV_CCL_LSL_FLATTEN_MIN_SAMPLES_PER_THREAD						(20*20)
#define COMPV_CCL_LSL_EXTRACT_COUNT_POINTS_BLOBS_MIN_SAMPLES_PER_THREAD		(20*20)
#define COMPV_CCL_LSL_EXTRACT_FILL_BLOBS_MIN_SAMPLES_PER_THREAD				(20*20)
#define COMPV_CCL_LSL_EXTRACT_COUNT_POINTS_CONTOURS_MIN_SAMPLES_PER_THREAD	(30*30)
#define COMPV_CCL_LSL_EXTRACT_FILL_CONTOURS_MIN_SAMPLES_PER_THREAD			(30*30)

#define COMPV_THIS_CLASSNAME "CompVConnectedComponentLabelingResultLSLImpl"

COMPV_NAMESPACE_BEGIN()

#if COMPV_OS_WINDOWS
typedef LONG compv_ccl_accumulator_t; /* InterlockedDecrement/InterlockedIncrement requires LONG */
#else
typedef int compv_ccl_accumulator_t;
#endif
typedef CompVMemZero<compv_ccl_accumulator_t > CompVMemZeroLockedAccumulator;
typedef CompVPtr<CompVMemZeroLockedAccumulator *> CompVMemZeroLockedAccumulatorPtr;
typedef CompVMemZeroLockedAccumulatorPtr* CompVMemZeroLockedAccumulatorPtrPtr;

static COMPV_ERROR_CODE count_points_blobs(
	CompVMemZeroLockedAccumulatorPtr ptrxNa,
	const compv_ccl_lea_n_t& vecLEA,
	const size_t ystart,
	const size_t yend);
static COMPV_ERROR_CODE count_points_segments(
	CompVMemZeroLockedAccumulatorPtr ptrxNa,
	const compv_ccl_lea_n_t& vecLEA,
	const size_t ystart,
	const size_t yend);
static COMPV_ERROR_CODE extract_blobs(
	CompVConnectedComponentPointsVector& points,
	const compv_ccl_lea_n_t& vecLEA,
	const CompVSizeSz& szInput,
	const size_t na
);
static COMPV_ERROR_CODE extract_segments(
	CompVConnectedComponentPointsVector& points,
	const compv_ccl_lea_n_t& vecLEA,
	const CompVSizeSz& szInput,
	const size_t na
);

CompVConnectedComponentLabelingResultLSLImpl::CompVConnectedComponentLabelingResultLSLImpl()
	: m_nNa1(0)
	, m_szInput(CompVSizeSz(0, 0))
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
	COMPV_CHECK_EXP_RETURN(!m_szInput.width || !m_szInput.height || m_vecLEA.size() != m_szInput.height, COMPV_ERROR_CODE_E_INVALID_STATE);

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
		int32_t* EA = ptr32sEA->ptr<int32_t>(ystart);
		compv_ccl_lea_1_t::const_iterator it;
		for (size_t j = ystart; j < yend; ++j) {
			COMPV_CHECK_CODE_RETURN(ptr32sEA->zero_row(j)); // background is "0"
			const compv_ccl_lea_1_t& lea = m_vecLEA[j];
			for (it = lea.begin(); it < lea.end(); ++it) {
				const int32_t a = it->a; // a within [1, na]
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

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::extract(CompVConnectedComponentPointsVector& points, COMPV_CCL_EXTRACT_TYPE type COMPV_DEFAULT(COMPV_CCL_EXTRACT_TYPE_BLOB)) const
{
	COMPV_CHECK_EXP_RETURN(type != COMPV_CCL_EXTRACT_TYPE_SEGMENT && type != COMPV_CCL_EXTRACT_TYPE_BLOB, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!m_szInput.width || !m_szInput.height || m_vecLEA.size() != m_szInput.height, COMPV_ERROR_CODE_E_INVALID_STATE);

	points.clear();

	if (!labelsCount()) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "No point to extract");
		return COMPV_ERROR_CODE_S_OK;
	}

	// LSL can compute the blob features without extracting the points
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("You don't need to extract the points in order to compute the blob features (bounding boxes, moments...)");

	switch (type) {
	case COMPV_CCL_EXTRACT_TYPE_BLOB:
		COMPV_CHECK_CODE_RETURN(extract_blobs(points, m_vecLEA, m_szInput, static_cast<size_t>(m_nNa1)));
		break;
	case COMPV_CCL_EXTRACT_TYPE_SEGMENT:
		COMPV_CHECK_CODE_RETURN(extract_segments(points, m_vecLEA, m_szInput, static_cast<size_t>(m_nNa1)));
		break;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	}
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

static COMPV_ERROR_CODE count_points_blobs(
	CompVMemZeroLockedAccumulatorPtr ptrxNa,
	const compv_ccl_lea_n_t& vecLEA,
	const size_t ystart,
	const size_t yend)
{
	compv_ccl_accumulator_t* naPtr = ptrxNa->ptr(0, 0);
	compv_ccl_lea_n_t::const_iterator j = vecLEA.begin() + ystart;
	compv_ccl_lea_n_t::const_iterator jend = vecLEA.begin() + yend;
	compv_ccl_lea_1_t::const_iterator i;
	for (; j < jend; ++j) {
		for (i = j->begin(); i < j->end(); ++i) {
			const compv_ccl_accumulator_t v = (i->end - i->start);
			compv_atomic_add(&naPtr[i->a - 1], v); // a within [1, na]
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE count_points_segments(
	CompVMemZeroLockedAccumulatorPtr ptrxNa,
	const compv_ccl_lea_n_t& vecLEA,
	const size_t ystart,
	const size_t yend)
{
	compv_ccl_accumulator_t* naPtr = ptrxNa->ptr(0, 0);
	compv_ccl_lea_n_t::const_iterator j = vecLEA.begin() + ystart;
	compv_ccl_lea_n_t::const_iterator jend = vecLEA.begin() + yend;
	compv_ccl_lea_1_t::const_iterator i;
	for (; j < jend; ++j) {
		for (i = j->begin(); i < j->end(); ++i) {
			compv_atomic_add(&naPtr[i->a - 1], 2); // a within [1, na]
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE extract_blobs(
	CompVConnectedComponentPointsVector& points,
	const compv_ccl_lea_n_t& vecLEA,
	const CompVSizeSz& szInput,
	const size_t na
)
{
	// Private function, no need to check input parameters

	/* Count the number of points (required for features computation) */
	CompVMemZeroLockedAccumulatorPtr ptrxNa; // final number of absolute labels (only needed to extract points)
	COMPV_CHECK_CODE_RETURN(CompVMemZeroLockedAccumulator::newObj(&ptrxNa, 1, na));
	auto funcPtrCountPoints = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		COMPV_CHECK_CODE_RETURN(count_points_blobs(
			ptrxNa,
			vecLEA,
			ystart,
			yend)
		);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrCountPoints,
		szInput.width,
		szInput.height,
		COMPV_CCL_LSL_EXTRACT_COUNT_POINTS_BLOBS_MIN_SAMPLES_PER_THREAD
	));

	/* Fill points */
	const compv_ccl_accumulator_t* ptrxNaPtr = ptrxNa->ptr();
	points.resize(na);
	for (size_t a = 1; a <= na; ++a) { // a within [1, na]
		const size_t count = static_cast<size_t>(ptrxNaPtr[a - 1]);
		points[a - 1].resize(count);
	}
	std::vector<compv_ccl_accumulator_t > szFilled(na, 0);
	auto funcPtrFill = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		compv_ccl_lea_1_t::const_iterator it;
		compv_ccl_lea_n_t::const_iterator j = vecLEA.begin() + ystart;
		compv_ccl_lea_n_t::const_iterator jend = vecLEA.begin() + yend;
		int16_t y = static_cast<int16_t>(ystart);
		for (; j < jend; ++j, ++y) {
			for (it = j->begin(); it < j->end(); ++it) {
				const int32_t a = (it->a - 1); // a within [1, na]
				CompVConnectedComponentPoints& pp = points[a];
				const size_t count = static_cast<size_t>(it->end - it->start);
				const size_t gOld = static_cast<size_t>(compv_atomic_add(&szFilled[a], static_cast<compv_ccl_accumulator_t>(count)));
				int16_t x_start = it->start;
				CompVConnectedComponentPoints::iterator it_pp = pp.begin() + gOld;
				for (size_t i = 0; i < count; ++i) {
					it_pp[i].x = x_start++;
					it_pp[i].y = y;
				}
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	};

	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrFill,
		szInput.width,
		szInput.height,
		COMPV_CCL_LSL_EXTRACT_FILL_BLOBS_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE extract_segments(
	CompVConnectedComponentPointsVector& points,
	const compv_ccl_lea_n_t& vecLEA,
	const CompVSizeSz& szInput,
	const size_t na
)
{
	// Private function, no need to check input parameters

	/* Count the number of points (required for features computation) */
	CompVMemZeroLockedAccumulatorPtr ptrxNa; // final number of absolute labels (only needed to extract points)
	COMPV_CHECK_CODE_RETURN(CompVMemZeroLockedAccumulator::newObj(&ptrxNa, 1, na));
	auto funcPtrCountPoints = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		COMPV_CHECK_CODE_RETURN(count_points_segments(
			ptrxNa,
			vecLEA,
			ystart,
			yend)
		);
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrCountPoints,
		szInput.width,
		szInput.height,
		COMPV_CCL_LSL_EXTRACT_COUNT_POINTS_CONTOURS_MIN_SAMPLES_PER_THREAD
	));

	/* Fill points */
	const compv_ccl_accumulator_t* ptrxNaPtr = ptrxNa->ptr();
	points.resize(na);
	for (size_t a = 1; a <= na; ++a) { // a within [1, na]
		const size_t count = static_cast<size_t>(ptrxNaPtr[a - 1]);
		points[a - 1].resize(count);
	}
	std::vector<compv_ccl_accumulator_t > szFilled(na, 0);
	auto funcPtrFill = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		compv_ccl_lea_1_t::const_iterator it;
		for (size_t j = ystart; j < yend; ++j) {
			const compv_ccl_lea_1_t& lea = vecLEA[j];
			const int16_t y = static_cast<int16_t>(j);
			for (it = lea.begin(); it < lea.end(); ++it) {
				const int32_t a = (it->a - 1); // a within [1, na]
				CompVConnectedComponentPoints& pp = points[a];
				const size_t gOld = static_cast<size_t>(compv_atomic_add(&szFilled[a], 2));
				CompVConnectedComponentPoints::iterator it_pp = pp.begin() + gOld;
				it_pp->x = it->start;
				it_pp->y = y;
				++it_pp;
				it_pp->x = it->end;
				it_pp->y = y;
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	};

	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrFill,
		szInput.width,
		szInput.height,
		COMPV_CCL_LSL_EXTRACT_FILL_CONTOURS_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
