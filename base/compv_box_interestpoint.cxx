/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_box_interestpoint.h"
#include "compv/base/compv_mem.h"
#include "compv/base/compv_debug.h"
#include "compv/base/parallel/compv_parallel.h"

#define COMPV_THIS_CLASSNAME	"CompVBoxInterestPoint"

COMPV_NAMESPACE_BEGIN()

#define COMPV_QUICKSORT_MIN_SAMPLES_PER_THREAD 200*100

static void sortByStrengthRange(CompVBoxInterestPoint* self, intptr_t left, intptr_t right);
static void scaleAndRoundAndGetAngleSinCos_C(COMPV_ALIGNED(x) const float* xf, COMPV_ALIGNED(x) const float *yf, COMPV_ALIGNED(x) const float *sf, COMPV_ALIGNED(x) const float* angleInDegree, COMPV_ALIGNED(x) int32_t* xi, COMPV_ALIGNED(x) int32_t* yi, COMPV_ALIGNED(x) float* cos, COMPV_ALIGNED(x) float* sin, compv_scalar_t count);

CompVBoxInterestPoint::CompVBoxInterestPoint(size_t nCapacity COMPV_DEFAULT(0), bool bLockable COMPV_DEFAULT(false))
	: CompVBox<CompVInterestPoint >(nCapacity, bLockable)
{

}

CompVBoxInterestPoint::~CompVBoxInterestPoint()
{

}

COMPV_ERROR_CODE CompVBoxInterestPoint::sortByStrength()
{
	sortByStrengthRange(this, 0, static_cast<intptr_t>(size()) - 1);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBoxInterestPoint::retainBest(size_t count)
{
	COMPV_CHECK_CODE_RETURN(sortByStrength());
	resize(count);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBoxInterestPoint::eraseTooCloseToBorder(size_t img_width, size_t img_height, size_t border_size)
{
	if (m_nSize > 0) {
		const CompVInterestPoint* p;
		compv_float32_t w = static_cast<compv_float32_t>(img_width), h = static_cast<compv_float32_t>(img_height), b = static_cast<compv_float32_t>(border_size);
		for (size_t i = 0; i < size(); ) {
			p = ptr(i);
			if ((p->x < b || (p->x + b) >= w || (p->y < b) || (p->y + b) >= h)) {
				erase(ptr(i));
			}
			else {
				++i;
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVBoxInterestPoint::newObj(CompVBoxInterestPointPtrPtr box, size_t nCapacity COMPV_DEFAULT(0), bool bLockable COMPV_DEFAULT(false))
{
	if (sizeof(CompVBoxInterestPoint) > kCompVBoxItemMaxSize) {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Boxing is only allowed on object with size < %u, you're boxing an object with size = %zu", static_cast<unsigned>(kCompVBoxItemMaxSize), sizeof(CompVBoxInterestPoint));
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	COMPV_CHECK_EXP_RETURN(!box, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVBoxInterestPointPtr box_;
	box_ = new CompVBoxInterestPoint(nCapacity, bLockable);
	COMPV_CHECK_EXP_RETURN(!box_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	COMPV_CHECK_CODE_RETURN(box_->init());
	*box = box_;
	return COMPV_ERROR_CODE_S_OK;
}

static void sortByStrengthRange(CompVBoxInterestPoint* self, intptr_t left, intptr_t right)
{
	CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
	size_t threadsCount = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 0;
	CompVAsyncTaskIds taskIds;
	const CompVInterestPoint pivot = *self->ptr((left + right) >> 1);
	CompVInterestPoint atk, *ati = self->ptr(left), *atj = self->ptr(right);
	const CompVInterestPoint *ati_ = ati, *atj_ = atj;
	while (ati <= atj) {
		while (ati->strength > pivot.strength) {
			++ati;
		}
		while (pivot.strength > atj->strength) {
			--atj;
		}
		if (ati > atj) {
			break;
		}
		atk = *ati;
		*ati = *atj;
		*atj = atk;
		++ati;
		--atj;
	}
	intptr_t i = left + (ati - ati_);
	intptr_t j = right + (atj - atj_);
	auto funcPtr = [&](intptr_t left_, intptr_t right_) -> void {
		sortByStrengthRange(self, left_, right_);
	};
	if (left < j) {
		if (threadsCount > 2 && (j - left) > COMPV_QUICKSORT_MIN_SAMPLES_PER_THREAD && !threadDisp->isMotherOfTheCurrentThread()) {
			COMPV_CHECK_CODE_ASSERT(threadDisp->invoke(std::bind(funcPtr, left, j), taskIds), "Dispatching task failed");
		}
		else {
			sortByStrengthRange(self, left, j);
		}
	}
	if (i < right) {
		if (threadsCount > 2 && (right - i) > COMPV_QUICKSORT_MIN_SAMPLES_PER_THREAD && !threadDisp->isMotherOfTheCurrentThread()) {
			COMPV_CHECK_CODE_ASSERT(threadDisp->invoke(std::bind(funcPtr, i, right), taskIds), "Dispatching task failed");
		}
		else {
			sortByStrengthRange(self, i, right);
		}
	}
	if (!taskIds.empty()) {
		COMPV_CHECK_CODE_ASSERT(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
	}
}

// Scale the point: xf *= sf, yf *= sf
// Round the point: xi = round(xf), yi = round(yf)
// Convert the angle to radian: angleInRad = degToRad(angleInDegree)
// Get the angle cos and sin: cos = cos(angleInRad), sin = sin(angleInRad)
void CompVInterestPointScaleAndRoundAndGetAngleSinCos(COMPV_ALIGNED(x) const float* xf, COMPV_ALIGNED(x) const float *yf, COMPV_ALIGNED(x) const float *sf, COMPV_ALIGNED(x) const float* angleInDegree, COMPV_ALIGNED(x) int32_t* xi, COMPV_ALIGNED(x) int32_t* yi, COMPV_ALIGNED(x) float* cos, COMPV_ALIGNED(x) float* sin, compv_scalar_t count)
{
	void(*scaleAndRoundAndGetAngleSinCos)(COMPV_ALIGNED(x) const float* xf, COMPV_ALIGNED(x) const float *yf, COMPV_ALIGNED(x) const float *sf, COMPV_ALIGNED(x) const float* angleInDegree, COMPV_ALIGNED(x) int32_t* xi, COMPV_ALIGNED(x) int32_t* yi, COMPV_ALIGNED(x) float* cos, COMPV_ALIGNED(x) float* sin, compv_scalar_t count)
		= scaleAndRoundAndGetAngleSinCos_C;

	scaleAndRoundAndGetAngleSinCos(xf, yf, sf, angleInDegree, xi, yi, cos, sin, count);
}

static void scaleAndRoundAndGetAngleSinCos_C(COMPV_ALIGNED(x) const float* xf, COMPV_ALIGNED(x) const float *yf, COMPV_ALIGNED(x) const float *sf, COMPV_ALIGNED(x) const float* angleInDegree, COMPV_ALIGNED(x) int32_t* xi, COMPV_ALIGNED(x) int32_t* yi, COMPV_ALIGNED(x) float* cos, COMPV_ALIGNED(x) float* sin, compv_scalar_t count)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No SIMD or GPU implementation found"); // TODO: SIMD
	float fx, fy, angleInRad;
	for (compv_scalar_t i = 0; i < count; ++i) {
		// Scale
		fx = (xf[i] * sf[i]);
		fy = (yf[i] * sf[i]);
		// Convert the angle from degree to radian
		angleInRad = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(angleInDegree[i]);
		// Get angle's cos and sin
		cos[i] = ::cos(angleInRad);
		sin[i] = ::sin(angleInRad);
		// Round the point
		xi[i] = COMPV_MATH_ROUNDFU_2_NEAREST_INT(fx, int32_t);
		yi[i] = COMPV_MATH_ROUNDFU_2_NEAREST_INT(fy, int32_t);
	}
}

COMPV_NAMESPACE_END()
