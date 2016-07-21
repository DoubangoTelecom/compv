/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/features/hough/compv_feature_houghstd.h"

COMPV_NAMESPACE_BEGIN()

CompVHoughStd::CompVHoughStd(float rho /*= 1.f*/, float theta /*= kfMathTrigPiOver180*/, int threshold /*= 1*/)
:CompVHough(COMPV_HOUGH_STANDARD_ID)
, m_fRho(rho)
, m_fTheta(theta)
, m_nThreshold(threshold)
{

}

CompVHoughStd:: ~CompVHoughStd()
{

}

// override CompVSettable::set
COMPV_ERROR_CODE CompVHoughStd::set(int id, const void* valuePtr, size_t valueSize)
{
	COMPV_CHECK_EXP_RETURN(valuePtr == NULL || valueSize == 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case -1:
	default:
		return CompVSettable::set(id, valuePtr, valueSize);
	}
}

static bool nonMaxSupp(CompVPtrArray(CompVHoughEntry)& accumulator, uint8_t* nms, int32_t threshold, int32_t radius)
{
	bool found = false;
	for (size_t row = radius; row < accumulator->rows() - radius; ++row) {
		for (size_t col = radius; col < accumulator->cols() - radius; ++col) {
			if (accumulator->ptr(row, col)->count > threshold && !nms[row * accumulator->cols() + col]) {
				//COMPV_DEBUG_INFO("Line at (%d, %f)", m_nRho * row, m_fTheta * (float)col);
				const CompVHoughEntry* e = accumulator->ptr(row, col);
				const CompVHoughEntry* top = accumulator->ptr(row - radius, col);
				const CompVHoughEntry* bottom = accumulator->ptr(row + radius, col);
				if (e[-radius].count >= e->count || e[+radius].count >= e->count || top[-radius].count >= e->count || top[0].count >= e->count || top[+radius].count >= e->count || bottom[-radius].count >= e->count || bottom[0].count >= e->count || bottom[+radius].count >= e->count) {
					nms[row * accumulator->cols() + col] = 1;
					found = true;
				}
			}
		}
	}
	return found;
}

static void nonMaxSupp2(CompVPtrArray(CompVHoughEntry)& accumulator, uint8_t* nms, int32_t threshold, int32_t radius)
{
	for (size_t row = 0; row < accumulator->rows(); ++row) {
		for (size_t col = 0; col < accumulator->cols(); ++col) {
			if (accumulator->ptr(row, col)->count > threshold) {
				const CompVHoughEntry* e = accumulator->ptr(row, col);
				int32_t minY = COMPV_MATH_MAX(0, int32_t(row - radius));
				int32_t maxY = COMPV_MATH_MIN(int32_t(accumulator->rows()), int32_t(row + radius));
				int32_t minX = COMPV_MATH_MAX(0, int32_t(col - radius));
				int32_t maxX = COMPV_MATH_MIN(int32_t(accumulator->cols()), int32_t(col + radius));
				for (int32_t y = minY; y < maxY; ++y) {
					for (int32_t x = minX; x < maxX; ++x) {
						if (accumulator->ptr(y, x)->count > e->count) {
							nms[row * accumulator->cols() + col] = 1;
							goto next;
						}
					}
				}
			}
		next:;
		}
	}
}

static void nonMaxSupp3(CompVPtrArray(CompVHoughEntry)& accumulator, uint8_t* nms, int32_t threshold)
{
	for (size_t row = 1; row < accumulator->rows() - 1; ++row) {
		for (size_t col = 1; col < accumulator->cols() - 1; ++col) {
			if (accumulator->ptr(row, col)->count > threshold) {
				const CompVHoughEntry* e = accumulator->ptr(row, col);
				const CompVHoughEntry* top = accumulator->ptr(row - 1, col);
				const CompVHoughEntry* bottom = accumulator->ptr(row + 1, col);
				if (e[-1].count > e->count || e[+1].count > e->count || top[-1].count > e->count || top[0].count > e->count || top[+1].count > e->count || bottom[-1].count > e->count || bottom[0].count > e->count || bottom[+1].count > e->count) {
					nms[row * accumulator->cols() + col] = 1;
				}
			}
		}
	}
}

// override CompVHough::process
COMPV_ERROR_CODE CompVHoughStd::process(const CompVPtrArray(uint8_t)& edges, CompVPtrArray(CompVCoordPolar2f)& coords)
{
	COMPV_CHECK_EXP_RETURN(!edges || edges->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVPtrArray(CompVHoughEntry) accumulator;

	const size_t cols = edges->cols();
	const size_t rows = edges->rows();

	const size_t maxRhoCount = COMPV_MATH_ROUNDFU_2_INT(((cols + rows) * 2 + 1) / m_fRho, size_t);
	const size_t maxThetaCount = COMPV_MATH_ROUNDFU_2_INT(kfMathTrigPi / m_fTheta, size_t);

	COMPV_CHECK_CODE_RETURN(CompVArray<CompVHoughEntry>::newObjStrideless(&accumulator, maxRhoCount, maxThetaCount));
	COMPV_CHECK_CODE_RETURN(accumulator->zero_all());

	const size_t stride = edges->strideInBytes();
	const uint8_t* pixels = edges->ptr(0);
	CompVHoughEntry* entry;

	for (size_t row = 0; row < rows; ++row) {
		for (size_t col = 0; col < cols; ++col) {
			if (pixels[col]) {
				for (size_t t = 0; t < maxThetaCount; ++t) {
					float theta = t * m_fTheta;
					float r = col * cos(theta) + row * sin(theta); // FIXME: use m_nRho here and must be float (combined with sin and cos tables)
					size_t rabs = COMPV_MATH_ROUNDFU_2_INT(fabs(r), size_t);
					if (r < 0) {
						rabs += (cols + rows);
					}
					if (rabs < maxRhoCount) {
						entry = const_cast<CompVHoughEntry*>(accumulator->ptr(rabs, t));
						if (entry->count) {
							int kaka = 0;
						}
						entry->rho = r;
						entry->theta = theta;
						entry->count += 1;
					}
					else {
						COMPV_DEBUG_ERROR("Not expected");
					}
				}
			}
		}
		pixels += stride;
	}

	uint8_t* nms = (uint8_t*)CompVMem::calloc(accumulator->rows() * accumulator->cols(), sizeof(uint8_t));

	// NMS
	//int32_t maxRadius = (int32_t)COMPV_MATH_MIN(cols, rows);
	//int32_t radius = 1;
	//while (nonMaxSupp(accumulator, nms, m_nThreshold, radius++) && radius < maxRadius) ;
	//nonMaxSupp2(accumulator, nms, m_nThreshold, 2);
	//nonMaxSupp(accumulator, nms, m_nThreshold, 1);
	nonMaxSupp3(accumulator, nms, m_nThreshold);
	
	/*for (size_t row = 1; row < accumulator->rows(); ++row) {
		for (size_t col = 1; col < accumulator->cols(); ++col) {
			if (accumulator->ptr(row, col)->count > m_nThreshold) {
				//COMPV_DEBUG_INFO("Line at (%d, %f)", m_nRho * row, m_fTheta * (float)col);
				const CompVHoughEntry* e = accumulator->ptr(row, col);
				const CompVHoughEntry* top = accumulator->ptr(row - 1, col);
				const CompVHoughEntry* bottom = accumulator->ptr(row + 1, col);
				if (e[-1].count >= e->count || e[+1].count >= e->count || top[-1].count >= e->count || top[0].count >= e->count || top[+1].count >= e->count || bottom[-1].count >= e->count || bottom[0].count >= e->count || bottom[+1].count >= e->count) {
					nms[row * accumulator->cols() + col] = 1;
				}
			}
		}
	}*/

	for (size_t row = 0; row < accumulator->rows(); ++row) {
		for (size_t col = 0; col < accumulator->cols(); ++col) {
			if (nms[row * accumulator->cols() + col]) {
				const_cast<CompVHoughEntry*>(accumulator->ptr(row, col))->count = 0;
				//COMPV_DEBUG_INFO("Suppressed");
			}
		}
	}

	size_t count = 0;
	for (size_t row = 0; row < accumulator->rows(); ++row) {
		for (size_t col = 0; col < accumulator->cols(); ++col) {
			if (accumulator->ptr(row, col)->count > m_nThreshold) {
				//COMPV_DEBUG_INFO("Line at (%d, %f)", m_nRho * row, m_fTheta * (float)col);
				++count;
			}
		}
	}

	if (count) {
		CompVPtrArray(size_t) Idx;
		COMPV_CHECK_CODE_RETURN(CompVArray<size_t>::newObjAligned(&Idx, 1, count));
		size_t* indexes = const_cast<size_t*>(Idx->ptr());
		size_t index, oldIndex;
		for (size_t i = 0; i < count; ++i) {
			indexes[i] = i;
		}

		CompVPtrArray(CompVCoordPolar2f) coords_;
		COMPV_CHECK_CODE_RETURN(CompVArray<CompVCoordPolar2f>::newObjStrideless(&coords_, 1, count));
		size_t i = 0;
		CompVCoordPolar2f* coord;
		for (size_t row = 0; row < accumulator->rows(); ++row) {
			for (size_t col = 0; col < accumulator->cols(); ++col) {
				if (accumulator->ptr(row, col)->count > m_nThreshold) {
					entry = const_cast<CompVHoughEntry*>(accumulator->ptr(row, col));
					coord = const_cast<CompVCoordPolar2f*>(coords_->ptr(0, i++));
					//coord->rho = m_nRho * (float)row;
					//coord->theta = m_fTheta * (float)col;
					coord->rho = entry->rho;
					coord->theta = entry->theta;
					coord->count = entry->count;
				}
			}
		}
		// sorting
		bool sorted, wasSorted = true;
		do {
			sorted = true;
			for (size_t i = 0; i < count - 1; ++i) {
				index = indexes[i];
				if (coords_->ptr(0, indexes[i])->count < coords_->ptr(0, indexes[i + 1])->count) {
					oldIndex = indexes[i];
					indexes[i] = indexes[i + 1];
					indexes[i + 1] = oldIndex;
					sorted = false;
					wasSorted = false;
				}
			}
		} while (!sorted);
		if (wasSorted) {
			coords = coords_;
		}
		else {
			COMPV_CHECK_CODE_RETURN(CompVArray<CompVCoordPolar2f>::newObjStrideless(&coords, 1, count));
			for (size_t col = 0; col < count; ++col) {
				*const_cast<CompVCoordPolar2f*>(coords->ptr(0, col)) = *coords_->ptr(0, indexes[col]);
			}
		}
	}
	else {
		coords = NULL;
	}
	
	CompVMem::free((void**)&nms);

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughStd::newObj(CompVPtr<CompVHough* >* hough, float rho /*= 1.f*/, float theta /*= kfMathTrigPiOver180*/, int threshold /*= 1*/)
{
	COMPV_CHECK_EXP_RETURN(!hough, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtr<CompVHoughStd* >hough_ = new CompVHoughStd(rho, theta, threshold);
	COMPV_CHECK_EXP_RETURN(!hough_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*hough = *hough_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
