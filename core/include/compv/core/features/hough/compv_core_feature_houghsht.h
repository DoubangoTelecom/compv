/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_HOUGHSHT_H_)
#define _COMPV_CORE_FEATURES_HOUGHSHT_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_memz.h"
#include "compv/base/compv_features.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

struct CompVHoughAccThreadsCtx {
	CompVMutexPtr mutex;
	CompVPtr<CompVMemZero<int32_t> *> acc;
	CompVMatPtr directions;
	size_t threadsCount;
};

// ARM NEON: use vld2_s32 to load the edges
#if defined(_MSC_VER)
#	pragma pack( push )
#	pragma pack( 1 )
#endif
#if defined (__GNUC__)
struct __attribute__((__packed__)) CompVHoughShtEdge
#else
struct CompVHoughShtEdge
#endif
{
	int32_t row;
	int32_t col;
public:
	CompVHoughShtEdge() : row(0), col(0) { }
	CompVHoughShtEdge(int32_t row_, int32_t col_) : row(row_), col(col_) { }
};
#if defined(_MSC_VER)
#	pragma pack( pop )
#endif

class CompVHoughSht : public CompVHough
{
protected:
	CompVHoughSht(float rho = 1.f, float theta = 1.f, size_t threshold = 1);
public:
	virtual ~CompVHoughSht();
	COMPV_OBJECT_GET_ID(CompVHoughSht);

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& edges, CompVHoughLineVector& lines, const CompVMatPtr& directions = NULL) override /*Overrides(CompVHough)*/;
	virtual COMPV_ERROR_CODE toCartesian(const size_t imageWidth, const size_t imageHeight, const CompVHoughLineVector& polar, CompVLineFloat32Vector& cartesian) override /*Overrides(CompVHough)*/;

	static COMPV_ERROR_CODE newObj(CompVHoughPtrPtr hough, float rho = 1.f, float theta = kfMathTrigPiOver180, size_t threshold = 1);

private:
	COMPV_ERROR_CODE initCoords(float fRho, float fTheta, size_t nThreshold, size_t nWidth = 0, size_t nHeight = 0);
	COMPV_ERROR_CODE acc_gather(std::vector<CompVHoughShtEdge >::const_iterator start, std::vector<CompVHoughShtEdge >::const_iterator end, CompVHoughAccThreadsCtx* threadsCtx);
	COMPV_ERROR_CODE nms_gather(size_t rowStart, size_t rowCount, CompVPtr<CompVMemZero<int32_t> *>& acc);
	COMPV_ERROR_CODE nms_apply(size_t rowStart, size_t rowCount, CompVPtr<CompVMemZero<int32_t> *>& acc, CompVHoughLineVector& lines);
	COMPV_ERROR_CODE toCartesian(const size_t imageWidth, const size_t imageHeight, CompVHoughLineVector::const_iterator polar_begin, CompVHoughLineVector::const_iterator polar_end, CompVLineFloat32Vector& cartesian);

private:
	float m_fRho;
	float m_fTheta;
	size_t m_nThreshold;
	size_t m_nWidth;
	size_t m_nHeight;
	size_t m_nMaxLines;
	size_t m_nBarrier;
	CompVMatPtr m_SinRho; // CompVMatPtr<int32_t>
	CompVMatPtr m_CosRho; // CompVMatPtr<int32_t>
	CompVMatPtr m_NMS; // CompVMatPtr<uint8_t>
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_HOUGHSHT_H_ */
