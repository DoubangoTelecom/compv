/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_FAST_DETE_H_)
#define _COMPV_CORE_FEATURES_FAST_DETE_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/gpu/core/features/fast/compv_gpu_feature_fast_dete.h"
#include "compv/base/compv_features.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

struct RangeFAST {
    const uint8_t* IP;
    uint8_t* strengths;
	uint8_t* nms;
    size_t rowStart;
	size_t rowEnd;
	size_t rowCount;
	size_t width;
	size_t stride;
    int32_t threshold;
    int32_t N;
    const compv_scalar_t *pixels16;
};

COMPV_OBJECT_DECLARE_PTRS(CornerDeteFAST);

class CompVCornerDeteFAST : public CompVCornerDete
{
protected:
    CompVCornerDeteFAST();
public:
    virtual ~CompVCornerDeteFAST();
	COMPV_OBJECT_GET_ID(CompVCornerDeteFAST);

    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
    virtual COMPV_ERROR_CODE process(const CompVMatPtr& image, std::vector<CompVInterestPoint>& interestPoints) override /*Overrides(CompVCornerDete)*/;

    static COMPV_ERROR_CODE newObj(CompVCornerDetePtrPtr fast);

private:
    int32_t m_iThreshold;
    int32_t m_iType;
    int32_t m_iNumContinuous;
    int32_t m_iMaxFeatures;
    bool m_bNonMaximaSupp;
    size_t m_nWidth;
	size_t m_nHeight;
	size_t m_nStride;
    RangeFAST* m_pRanges;
    size_t m_nRanges;
    uint8_t* m_pStrengthsMap;
	uint8_t* m_pNmsMap;
	CompVGpuCornerDeteFASTPtr m_ptrGpuFAST;
	CompVMatPtr m_ptrImageGray;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_FAST_DETE_H_ */
