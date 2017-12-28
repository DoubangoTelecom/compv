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
#define COMPV_CCL_LSL_EXTRACT_BOXES_MIN_SAMPLES_PER_THREAD					(40*40)
#define COMPV_CCL_LSL_LABELS_REMOVE_MIN_SAMPLES_PER_THREAD					(60*60)

#define COMPV_THIS_CLASSNAME "CompVConnectedComponentLabelingResultLSLImpl"

COMPV_NAMESPACE_BEGIN()

static COMPV_ERROR_CODE count_points_blobs(
	CompVMemZeroLockedAccumulatorPtrPtr ptrxNa,
	const CompVConnectedComponentLabelingResultLSLImplPtr& result
);
static COMPV_ERROR_CODE count_points_segments(
	CompVMemZeroLockedAccumulatorPtrPtr ptrxNa,
	const CompVConnectedComponentLabelingResultLSLImplPtr& result
);

CompVConnectedComponentLabelingResultLSLImpl::CompVConnectedComponentLabelingResultLSLImpl()
	: m_szInput(CompVSizeSz(0, 0))
{
}

CompVConnectedComponentLabelingResultLSLImpl::~CompVConnectedComponentLabelingResultLSLImpl()
{

}

size_t CompVConnectedComponentLabelingResultLSLImpl::labelsCount() const
{
	return m_vecIds.size();
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

	CompVConnectedComponentLabelingResultLSLImplPtr hackedThis = const_cast<CompVConnectedComponentLabelingResultLSLImpl*>(this);

	switch (type) {
	case COMPV_CCL_EXTRACT_TYPE_BLOB:
		COMPV_CHECK_CODE_RETURN(hackedThis->extract_blobs(points));
		break;
	case COMPV_CCL_EXTRACT_TYPE_SEGMENT:
		COMPV_CHECK_CODE_RETURN(hackedThis->extract_segments(points));
		break;
	default:
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	}
	return COMPV_ERROR_CODE_S_OK;
}

const std::vector<int32_t>& CompVConnectedComponentLabelingResultLSLImpl::labelIds() const
{
	return m_vecIds;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::boundingBoxes(CompVConnectedComponentBoundingBoxesVector& boxes) const
{
	CompVConnectedComponentLabelingResultLSLImplPtr hackedThis = const_cast<CompVConnectedComponentLabelingResultLSLImpl*>(this);

	if (!labelsCount()) {
		boxes.clear();
		return COMPV_ERROR_CODE_S_OK;
	}
	
	boxes = CompVConnectedComponentBoundingBoxesVector(m_nNa1,
		CompVConnectedComponentBoundingBox(
			static_cast<int16_t>(m_szInput.width), // left - min
			static_cast<int16_t>(m_szInput.height), // top - min
			0, // right - max
			0 // bottom - max
		));

	auto funcPtrBoxes = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		compv_ccl_lea_1_t::const_iterator it;
		for (size_t j = ystart; j < yend; ++j) {
			const compv_ccl_lea_1_t& lea = m_vecLEA[j];
			const int16_t y = static_cast<int16_t>(j);
			for (it = lea.begin(); it < lea.end(); ++it) {
				const int32_t a = (it->a - 1); // a within [1, na]
				CompVConnectedComponentBoundingBox& bb = boxes[a];
				// Visual studio 2015 nicely generate branchless code using conditional movs
				bb.left = COMPV_MATH_MIN(bb.left, it->start);
				bb.top = COMPV_MATH_MIN(bb.top, y);
				bb.right = COMPV_MATH_MAX(bb.right, it->end); // bb.right + ((it->end - bb.right) & ((bb.right - it->end) >> 15));
				bb.bottom = COMPV_MATH_MAX(bb.bottom, y); // bb.bottom + ((y - bb.bottom) & ((bb.bottom - y) >> 15));
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	};

#if 0 // Not thread-safe
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrBoxes,
		m_szInput.width,
		m_szInput.height,
		COMPV_CCL_LSL_EXTRACT_BOXES_MIN_SAMPLES_PER_THREAD
	));
#else
	// Not a high prio: duration on "text_1122x1182_white_gray.yuv" -> 0.1145 millis
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No MT implementation could be found");
	COMPV_CHECK_CODE_RETURN(funcPtrBoxes(0, m_szInput.height));
#endif

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::boundingBoxes(const CompVConnectedComponentPointsVector& segments, CompVConnectedComponentBoundingBoxesVector& boxes) const
{
	if (segments.empty()) {
		boxes.clear();
		return COMPV_ERROR_CODE_S_OK;
	}

	const size_t na = segments.size();

	boxes = CompVConnectedComponentBoundingBoxesVector(na,
		CompVConnectedComponentBoundingBox(
			static_cast<int16_t>(m_szInput.width), // left - min
			static_cast<int16_t>(m_szInput.height), // top - min
			0, // right - max
			0 // bottom - max
		));

	auto funcPtrBoxes = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		for (size_t j = ystart; j < yend; ++j) {
			const CompVConnectedComponentPoints& points = segments[j];
			COMPV_CHECK_EXP_RETURN(points.size() & 1, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
			for (CompVConnectedComponentPoints::const_iterator it = points.begin(); it < points.end(); it += 2) {
				CompVConnectedComponentBoundingBox& bb = boxes[j];
				// Visual studio 2015 nicely generate branchless code using conditional movs
				bb.left = COMPV_MATH_MIN(bb.left, it->x);
				bb.top = COMPV_MATH_MIN(bb.top, it->y);
				bb.right = COMPV_MATH_MAX(bb.right, (it + 1)->x); // bb.right + ((it->end - bb.right) & ((bb.right - it->end) >> 15));
				bb.bottom = COMPV_MATH_MAX(bb.bottom, (it + 1)->y); // bb.bottom + ((y - bb.bottom) & ((bb.bottom - y) >> 15));
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	};

	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrBoxes,
		m_szInput.width,
		na,
		COMPV_CCL_LSL_EXTRACT_BOXES_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::firstOrderMoment() const
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::remove(CompVConnectedComponentCallbackRemoveLabel funcPtr, size_t &removedCount)
{
	COMPV_CHECK_EXP_RETURN(!funcPtr, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_DEBUG_INFO_CODE_FOR_TESTING("You should not use this function unless you know what you're doing. Data will be corrupted after calling it");
	
	// Collect labels to remove
	std::vector<int32_t> vecLabelsToRemove;
	vecLabelsToRemove.reserve(m_vecIds.size());
	for (std::vector<int32_t>::const_iterator a = m_vecIds.begin(); a < m_vecIds.end(); ++a) {
		if (funcPtr(*a)) {
			vecLabelsToRemove.push_back(*a);
		}
	}
	removedCount = vecLabelsToRemove.size();

	// Remove collected labels
	if (removedCount) {
		auto funcPtrRemove = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
			std::vector<int32_t>::const_iterator r_begin = vecLabelsToRemove.begin();
			std::vector<int32_t>::const_iterator r_end = vecLabelsToRemove.end();

			// Remove labels from the vec-id list
			if (!ystart) {
				for (std::vector<int32_t>::const_iterator i = r_begin; i < r_end; ++i) {
					m_vecIds.erase(std::remove(m_vecIds.begin(), m_vecIds.end(), *i), m_vecIds.end());
				}
				// Must not update "m_nNa1", it's the reference number of labels and never change
			}

			// Remove labels from run-length list
			for (size_t j = ystart; j < yend; ++j) {
				compv_ccl_lea_1_t& lea = m_vecLEA[j];
				lea.erase(std::remove_if(lea.begin(),
					lea.end(),
					[&](const compv_ccl_range_t& range) { return std::find(r_begin, r_end, range.a) != r_end; }),
					lea.end());
			}
			return COMPV_ERROR_CODE_S_OK;
		};

		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtrRemove,
			m_szInput.width,
			m_szInput.height,
			COMPV_CCL_LSL_LABELS_REMOVE_MIN_SAMPLES_PER_THREAD
		));

		// Reset local variables (no longer valid)
		m_ptrCountPointsSegment = nullptr;
		m_ptrCountPointsBlobs = nullptr;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::reset()
{
	m_nNa1 = 0;
	m_vecIds.clear();
	m_ptrCountPointsBlobs = nullptr;
	m_ptrCountPointsSegment = nullptr;
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

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::extract_blobs(CompVConnectedComponentPointsVector& points)
{
	// Private function, no need to check input parameters
	
	/* Count the number of points (required for features computation) */
	if (!m_ptrCountPointsBlobs) {
		COMPV_CHECK_CODE_RETURN(count_points_blobs(
			&m_ptrCountPointsBlobs,
			this
		));
	}

	/* Fill points */
	const compv_ccl_accumulator_t* ptrxNaPtr = m_ptrCountPointsBlobs->ptr();
	points.resize(m_nNa1);
	for (size_t a = 1; a <= m_nNa1; ++a) { // a within [1, na]
		const size_t count = static_cast<size_t>(ptrxNaPtr[a - 1]);
		points[a - 1].resize(count);
	}
	std::vector<compv_ccl_accumulator_t > szFilled(m_nNa1, 0);
	auto funcPtrFill = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		compv_ccl_lea_1_t::const_iterator it;
		compv_ccl_lea_n_t::const_iterator j = m_vecLEA.begin() + ystart;
		compv_ccl_lea_n_t::const_iterator jend = m_vecLEA.begin() + yend;
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
		m_szInput.width,
		m_szInput.height,
		COMPV_CCL_LSL_EXTRACT_FILL_BLOBS_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingResultLSLImpl::extract_segments(CompVConnectedComponentPointsVector& points)
{
	// Private function, no need to check input parameters

	/* Count the number of points (required for features computation) */
	if (!m_ptrCountPointsSegment) {
		COMPV_CHECK_CODE_RETURN(count_points_segments(
			&m_ptrCountPointsSegment,
			this
		));
	}

	/* Fill points */
	const compv_ccl_accumulator_t* ptrxNaPtr = m_ptrCountPointsSegment->ptr();
	points.resize(m_nNa1);
	for (size_t a = 1; a <= m_nNa1; ++a) { // a within [1, na]
		const size_t count = static_cast<size_t>(ptrxNaPtr[a - 1]);
		points[a - 1].resize(count);
	}
	std::vector<compv_ccl_accumulator_t > szFilled(m_nNa1, 0);
	auto funcPtrFill = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		compv_ccl_lea_1_t::const_iterator it;
		for (size_t j = ystart; j < yend; ++j) {
			const compv_ccl_lea_1_t& lea = m_vecLEA[j];
			const int16_t y = static_cast<int16_t>(j);
			for (it = lea.begin(); it < lea.end(); ++it) {
				const int32_t a = (it->a - 1); // a within [1, na]
				CompVConnectedComponentPoints& pp = points[a];
				if (!pp.empty()) { // number of segments could be zero if labels removed
					const size_t gOld = static_cast<size_t>(compv_atomic_add(&szFilled[a], 2));
					CompVConnectedComponentPoints::iterator it_pp = pp.begin() + gOld;
					it_pp->x = it->start;
					it_pp->y = y;
					++it_pp;
					it_pp->x = it->end;
					it_pp->y = y;
				}
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	};

	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrFill,
		m_szInput.width,
		m_szInput.height,
		COMPV_CCL_LSL_EXTRACT_FILL_CONTOURS_MIN_SAMPLES_PER_THREAD
	));

	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE count_points_blobs(
	CompVMemZeroLockedAccumulatorPtrPtr ptrxNa,
	const CompVConnectedComponentLabelingResultLSLImplPtr& result
)
{
	const compv_ccl_lea_n_t& vecLEA = result->vecLEA();
	const CompVSizeSz& szInput = result->szInput();
	const size_t& na = result->na1();

	CompVMemZeroLockedAccumulatorPtr ptrxNa_; // final number of absolute labels (only needed to extract points)
	COMPV_CHECK_CODE_RETURN(CompVMemZeroLockedAccumulator::newObj(&ptrxNa_, 1, na));

	auto funcPtrCountPoints = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		compv_ccl_accumulator_t* naPtr = ptrxNa_->ptr(0, 0);
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
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrCountPoints,
		szInput.width,
		szInput.height,
		COMPV_CCL_LSL_EXTRACT_COUNT_POINTS_BLOBS_MIN_SAMPLES_PER_THREAD
	));

	*ptrxNa = ptrxNa_;
	return COMPV_ERROR_CODE_S_OK;
}

static COMPV_ERROR_CODE count_points_segments(
	CompVMemZeroLockedAccumulatorPtrPtr ptrxNa,
	const CompVConnectedComponentLabelingResultLSLImplPtr& result
)
{
	const compv_ccl_lea_n_t& vecLEA = result->vecLEA();
	const CompVSizeSz& szInput = result->szInput();
	const size_t& na = result->na1();

	CompVMemZeroLockedAccumulatorPtr ptrxNa_; // final number of absolute labels (only needed to extract points)
	COMPV_CHECK_CODE_RETURN(CompVMemZeroLockedAccumulator::newObj(&ptrxNa_, 1, na));
	auto funcPtrCountPoints = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		compv_ccl_accumulator_t* naPtr = ptrxNa_->ptr(0, 0);
		compv_ccl_lea_n_t::const_iterator j = vecLEA.begin() + ystart;
		compv_ccl_lea_n_t::const_iterator jend = vecLEA.begin() + yend;
		compv_ccl_lea_1_t::const_iterator i;
		for (; j < jend; ++j) {
			for (i = j->begin(); i < j->end(); ++i) {
				compv_atomic_add(&naPtr[i->a - 1], 2); // a within [1, na]
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrCountPoints,
		szInput.width,
		szInput.height,
		COMPV_CCL_LSL_EXTRACT_COUNT_POINTS_CONTOURS_MIN_SAMPLES_PER_THREAD
	));

	*ptrxNa = ptrxNa_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
