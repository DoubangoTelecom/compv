/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/core/features/hough/compv_core_feature_houghkht.h"
#include "compv/base/image/compv_image.h"
#include "compv/base/math/compv_math_utils.h"
#include "compv/base/parallel/compv_parallel.h"

#include <algorithm> /* std::reverse */

#define COMPV_THIS_CLASSNAME	"CompVHoughKht"

// Documentation:
//	- http://www2.ic.uff.br/~laffernandes/projects/kht/
//	- https://www.academia.edu/11637890/Real-time_line_detection_through_an_improved_Hough_transform_voting_scheme
//	- https://en.wikipedia.org/wiki/Hough_transform#Kernel-based_Hough_transform_.28KHT.29
//	- http://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.145.5388&rep=rep1&type=pdf

COMPV_NAMESPACE_BEGIN()

CompVHoughKht::CompVHoughKht(float rho COMPV_DEFAULT(1.f), float theta COMPV_DEFAULT(kfMathTrigPiOver180), size_t threshold COMPV_DEFAULT(1))
	:CompVHough(COMPV_HOUGHKHT_ID)
	, m_fRho(rho)
	, m_fTheta(theta)
	, m_nThreshold(threshold)
	, m_nWidth(0)
	, m_nHeight(0)
	, m_nMaxLines(INT_MAX)
{

}

CompVHoughKht:: ~CompVHoughKht()
{

}

// override CompVSettable::set
COMPV_ERROR_CODE CompVHoughKht::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVCaps)*/
{
	COMPV_CHECK_EXP_RETURN(!valuePtr || !valueSize, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	switch (id) {
	case COMPV_HOUGH_SET_FLT32_RHO: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) <= 0.f || *reinterpret_cast<const compv_float32_t*>(valuePtr) > 1.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const compv_float32_t fRho = *reinterpret_cast<const compv_float32_t*>(valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(fRho, m_fTheta, m_nThreshold, m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_FLT32_THETA: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(compv_float32_t) || *reinterpret_cast<const compv_float32_t*>(valuePtr) <= 0.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const compv_float32_t fTheta = *reinterpret_cast<const compv_float32_t*>(valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(m_fRho, fTheta, m_nThreshold, m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_INT_THRESHOLD: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int) || *reinterpret_cast<const int*>(valuePtr) <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		const int nThreshold = *reinterpret_cast<const int*>(valuePtr);
		COMPV_CHECK_CODE_RETURN(initCoords(m_fRho, m_fTheta, static_cast<size_t>(nThreshold), m_nWidth, m_nHeight));
		return COMPV_ERROR_CODE_S_OK;
	}
	case COMPV_HOUGH_SET_INT_MAXLINES: {
		COMPV_CHECK_EXP_RETURN(valueSize != sizeof(int), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		m_nMaxLines = static_cast<size_t>(reinterpret_cast<const int*>(valuePtr) <= 0 ? INT_MAX : *reinterpret_cast<const int*>(valuePtr));
		return COMPV_ERROR_CODE_S_OK;
	}
	default: {
		COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Set with id %d not implemented", id);
		return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
	}
	}
}

COMPV_ERROR_CODE CompVHoughKht::process(const CompVMatPtr& edges, CompVHoughLineVector& lines, const CompVMatPtr& directions COMPV_DEFAULT(NULL)) /*Overrides(CompVHough)*/
{
	COMPV_CHECK_EXP_RETURN(!edges || edges->isEmpty() || edges->subType() != COMPV_SUBTYPE_PIXELS_Y, COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Edges null or not grayscale");

	// Clone the edges (the linking procedure modifify the data)
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Do not create a copy of the edges. Use a map (like nms)");
	COMPV_CHECK_CODE_RETURN(edges->clone(&m_edges));

	// Init coords (sine and cosine tables)
	COMPV_CHECK_CODE_RETURN(initCoords(m_fRho, m_fTheta, m_nThreshold, edges->cols(), edges->rows()));

	lines.clear();

	// Appendix A. Linking procedure
	COMPV_CHECK_CODE_RETURN(linking_AppendixA(m_edges, m_strings));

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughKht::newObj(CompVHoughPtrPtr hough, float rho COMPV_DEFAULT(1.f), float theta COMPV_DEFAULT(kfMathTrigPiOver180), size_t threshold COMPV_DEFAULT(1))
{
	COMPV_CHECK_EXP_RETURN(!hough || rho <= 0 || rho > 1.f, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVHoughPtr hough_ = new CompVHoughKht(rho, theta, threshold);
	COMPV_CHECK_EXP_RETURN(!hough_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*hough = *hough_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVHoughKht::initCoords(float fRho, float fTheta, size_t nThreshold, size_t nWidth COMPV_DEFAULT(0), size_t nHeight COMPV_DEFAULT(0))
{
	m_fRho = fRho;
	m_fTheta = fTheta;
	m_nThreshold = nThreshold;
	m_nWidth = nWidth ? nWidth : m_nWidth;
	m_nHeight = nHeight ? nHeight : m_nHeight;

	return COMPV_ERROR_CODE_S_OK;
}

// Appendix A. Linking procedure
COMPV_ERROR_CODE CompVHoughKht::linking_AppendixA(CompVMatPtr& edges, CompVHoughKhtStrings& strings)
{
	uint8_t* edgesPtr = edges->ptr<uint8_t>(1);
	const size_t edgesWidth = edges->cols();
	const size_t edgesHeight = edges->rows();
	const size_t edgesStride = edges->strideInBytes();
	const size_t maxi = edges->cols() - 1;
	const size_t maxj = edges->rows() - 1;
	const size_t min_size = 10; // FIXME(dmi) -> must be parameter

	COMPV_DEBUG_INFO_CODE_FOR_TESTING("min_size must be parameter");

	strings.clear();

	for (size_t y_ref = 1; y_ref < maxj; ++y_ref) {
		for (size_t x_ref = 1; x_ref < maxi; ++x_ref) {
			if (edgesPtr[x_ref]) {
				CompVHoughKhtString string;
				linking_link_Algorithm5(&edgesPtr[x_ref], edgesWidth, edgesHeight, edgesStride, string, x_ref, y_ref);
				if (string.size() >= min_size) {
					strings.push_back(string);
				}
			}
		}
		edgesPtr += edges->strideInBytes();
	}

	return COMPV_ERROR_CODE_S_OK;
}

// Algorithm 5 - Linking of neighboring edge pixels into strings
void CompVHoughKht::linking_link_Algorithm5(uint8_t* edgesPtr, const size_t edgesWidth, const size_t edgesHeight, const size_t edgesStride, CompVHoughKhtString& string, const size_t x_ref, const size_t y_ref)
{
	string.clear();
	
	// {Find and add feature pixels to the end of the string}
	size_t x_seed = x_ref;
	size_t y_seed = y_ref;
	uint8_t* next_seed = edgesPtr;
	do {
		string.push_back(CompVMatIndex(y_seed, x_seed));
		*next_seed = 0x00;
	} while((next_seed = linking_next_Algorithm6(next_seed, edgesWidth, edgesHeight, edgesStride, x_seed, y_seed)));

	std::reverse(string.begin(), string.end());

	// {Find and add feature pixels to the begin of the string}
	x_seed = x_ref;
	y_seed = y_ref;
	next_seed = edgesPtr;
	if ((next_seed = linking_next_Algorithm6(next_seed, edgesWidth, edgesHeight, edgesStride, x_seed, y_seed))) {
		do {
			string.push_back(CompVMatIndex(y_seed, x_seed));
			*next_seed = 0x00;
		} while ((next_seed = linking_next_Algorithm6(next_seed, edgesWidth, edgesHeight, edgesStride, x_seed, y_seed)));
	}
}

// Algorithm 6 - Function Next(). It complements the linking procedure(Algorithm 5).
uint8_t* CompVHoughKht::linking_next_Algorithm6(uint8_t* edgesPtr, const size_t edgesWidth, const size_t edgesHeight, const size_t edgesStride, size_t &x_seed, size_t &y_seed)
{
	const bool left_avail = x_seed > 0;
	const bool right_avail = (x_seed + 1) < edgesWidth;

	/* == top == */
	if (y_seed > 0) {
		uint8_t* top = edgesPtr - edgesStride;
		if (left_avail && top[-1]) { // top-left
			--x_seed, --y_seed; return &top[-1];
		}
		if (*top) { // top-center
			--y_seed; return top;
		}
		if (right_avail && top[1]) { // top-right
			++x_seed, --y_seed; return &top[1];
		}
	}
	
	/* == center == */
	if (left_avail && edgesPtr[-1]) { // center-left
		--x_seed; return &edgesPtr[-1];
	}
	if (right_avail && edgesPtr[1]) { // center-right
		++x_seed; return &edgesPtr[1];
	}

	/* == bottom == */
	if ((y_seed + 1) < edgesHeight) {
		uint8_t* bottom = edgesPtr + edgesStride;
		if (left_avail && bottom[-1]) { // bottom-left
			--x_seed, ++y_seed;	return &bottom[-1];
		}
		if (*bottom) { // bottom-center
			++y_seed; return bottom;
		}
		if (right_avail && bottom[1]) { // bottom-right
			++x_seed, ++y_seed; return &bottom[1];
		}
	}
	return NULL;
}

COMPV_NAMESPACE_END()
