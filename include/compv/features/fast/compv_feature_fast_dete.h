/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_FAST_DETE_H_)
#define _COMPV_FEATURES_FAST_DETE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/features/compv_feature.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

struct RangeFAST {
    const uint8_t* IP;
    const uint8_t* IPprev;
    uint8_t* strengths;
    int32_t rowStart;
    int32_t rowEnd;
    int32_t rowCount;
    int32_t width;
    int32_t stride;
    int32_t threshold;
    int32_t N;
    const compv_scalar_t(*pixels16)[16];
};

class CompVCornerDeteFAST : public CompVCornerDete
{
protected:
    CompVCornerDeteFAST();
public:
    virtual ~CompVCornerDeteFAST();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVCornerDeteFAST";
    };

    // override CompVSettable::set
    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
    // override CompVCornerDete::process
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, CompVPtr<CompVBoxInterestPoint* >& interestPoints);

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVCornerDete* >* fast);

private:
    int32_t m_iThreshold;
    int32_t m_iType;
    int32_t m_iNumContinuous;
    int32_t m_iMaxFeatures;
    bool m_bNonMaximaSupp;
    int32_t m_nWidth;
    int32_t m_nHeight;
    int32_t m_nStride;
    RangeFAST* m_pRanges;
    int32_t m_nRanges;
    uint8_t* m_pStrengthsMap;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_FAST_DETE_H_ */
