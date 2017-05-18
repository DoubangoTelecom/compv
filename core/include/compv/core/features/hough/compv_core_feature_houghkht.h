/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_HOUGHKHT_H_)
#define _COMPV_CORE_FEATURES_HOUGHKHT_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_memz.h"
#include "compv/base/compv_features.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

typedef std::vector<CompVMatIndex> CompVHoughKhtString;
typedef std::vector<CompVHoughKhtString> CompVHoughKhtStrings;

class CompVHoughKht : public CompVHough
{
protected:
	CompVHoughKht(float rho = 1.f, float theta = kfMathTrigPiOver180, size_t threshold = 1);
public:
	virtual ~CompVHoughKht();
	COMPV_OBJECT_GET_ID(CompVHoughKht);

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& edges, CompVHoughLineVector& lines, const CompVMatPtr& directions = NULL) override /*Overrides(CompVHough)*/;

	static COMPV_ERROR_CODE newObj(CompVHoughPtrPtr hough, float rho = 1.f, float theta = kfMathTrigPiOver180, size_t threshold = 1);

private:
	COMPV_ERROR_CODE initCoords(float fRho, float fTheta, size_t nThreshold, size_t nWidth = 0, size_t nHeight = 0);
	COMPV_ERROR_CODE linking_AppendixA(CompVMatPtr& edges, CompVHoughKhtStrings& strings);
	void linking_link_Algorithm5(uint8_t* edgesPtr, const size_t edgesWidth, const size_t edgesHeight, const size_t edgesStride, CompVHoughKhtString& string, const size_t x_ref, const size_t y_ref);
	uint8_t* linking_next_Algorithm6(uint8_t* edgesPtr, const size_t edgesWidth, const size_t edgesHeight, const size_t edgesStride, size_t &x_seed, size_t &y_seed);

private:
	float m_fRho;
	float m_fTheta;
	size_t m_nThreshold;
	size_t m_nWidth;
	size_t m_nHeight;
	size_t m_nMaxLines;
	CompVMatPtr m_edges;
	CompVHoughKhtStrings m_strings;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_HOUGHKHT_H_ */
