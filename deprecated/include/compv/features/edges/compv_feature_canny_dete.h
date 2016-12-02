/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_CANNY_DETE_H_)
#define _COMPV_FEATURES_CANNY_DETE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_box.h"
#include "compv/features/compv_feature.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

#define COMPV_FEATURE_DETE_CANNY_THRESHOLD_LOW	(0.68f)
#define COMPV_FEATURE_DETE_CANNY_THRESHOLD_HIGH	(COMPV_FEATURE_DETE_CANNY_THRESHOLD_LOW * 2.f)

COMPV_NAMESPACE_BEGIN()

class CompVEdgeDeteCanny : public CompVEdgeDete
{
protected:
    CompVEdgeDeteCanny(float tLow = COMPV_FEATURE_DETE_CANNY_THRESHOLD_LOW, float tHigh = COMPV_FEATURE_DETE_CANNY_THRESHOLD_HIGH, int32_t kernSize = 3);
public:
    virtual ~CompVEdgeDeteCanny();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVEdgeDeteCanny";
    };

    // override CompVSettable::set
    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
    // override CompVEdgeDete::process
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, CompVPtrArray(uint8_t)& edges);

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVEdgeDete* >* dete, float tLow = COMPV_FEATURE_DETE_CANNY_THRESHOLD_LOW, float tHigh = COMPV_FEATURE_DETE_CANNY_THRESHOLD_HIGH, int32_t kernSize = 3);

private:
    COMPV_ERROR_CODE nms_gather(CompVPtrArray(uint8_t)& edges, uint16_t tLow, size_t rowStart, size_t rowCount);
    void nms_apply();
    COMPV_ERROR_CODE hysteresis(CompVPtrArray(uint8_t)& edges, uint16_t tLow, uint16_t tHigh, size_t rowStart, size_t rowCount);

private:
    size_t m_nImageWidth;
    size_t m_nImageHeight;
    size_t m_nImageStride;
    int16_t* m_pGx;
    int16_t* m_pGy;
    uint16_t* m_pG;
    const int16_t* m_pcKernelVt;
    const int16_t* m_pcKernelHz;
    size_t m_nKernelSize;
    float m_fThresholdLow;
    float m_fThresholdHigh;
    uint8_t* m_pNms;
};

#define COMPV_CANNY_PUSH_CANDIDATE(box, r, c) (box)->new_item(&ne), ne->row = (r), ne->col = (c)

void CannyNmsGatherRow_C(uint8_t* nms, const uint16_t* g, const int16_t* gx, const int16_t* gy, uint16_t tLow, size_t colStart, size_t width, size_t stride);
void CannyHysteresisRow_C(size_t row, size_t colStart, size_t width, size_t height, size_t stride, uint16_t tLow, uint16_t tHigh, const uint16_t* grad, const uint16_t* g0, uint8_t* e, uint8_t* e0, CompVPtr<CompVBox<CompVIndex>* >& candidates);

static const float kTangentPiOver8 = 0.414213568f; // tan(22.5)
static const int32_t kTangentPiOver8Int = static_cast<int32_t>(kTangentPiOver8 * (1 << 16));
static const float kTangentPiTimes3Over8 = 2.41421366f; // tan(67.5)
static const int32_t kTangentPiTimes3Over8Int = static_cast<int32_t>(kTangentPiTimes3Over8 * (1 << 16));

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_CANNY_DETE_H_ */
