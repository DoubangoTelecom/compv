/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_FEATURES_EDGES_DETE_H_)
#define _COMPV_FEATURES_EDGES_DETE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/features/compv_feature.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVEdgeDeteBASE : public CompVEdgeDete
{
protected:
    CompVEdgeDeteBASE(int id, const int16_t* kernelPtrVt, const int16_t* kernelPtrHz, size_t kernelSize);
public:
    virtual ~CompVEdgeDeteBASE();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVEdgeDeteBASE";
    };

    // override CompVSettable::set
    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
    // override CompVEdgeDete::process
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVImage*>& image, CompVPtrArray(uint8_t)& edges);

    static COMPV_ERROR_CODE newObjSobel(CompVPtr<CompVEdgeDete* >* dete, float tLow = 0.68f, float tHigh = 0.68f*2.f, int32_t kernSize = 3);
    static COMPV_ERROR_CODE newObjScharr(CompVPtr<CompVEdgeDete* >* dete, float tLow = 0.68f, float tHigh = 0.68f*2.f, int32_t kernSize = 3);
    static COMPV_ERROR_CODE newObjPrewitt(CompVPtr<CompVEdgeDete* >* dete, float tLow = 0.68f, float tHigh = 0.68f*2.f, int32_t kernSize = 3);

private:
    static COMPV_ERROR_CODE newObj(CompVPtr<CompVEdgeDete* >* dete, int id, float tLow = 0.68f, float tHigh = 0.68f*2.f, int32_t kernSize = 3);
    const int16_t* m_pcKernelVt;
    const int16_t* m_pcKernelHz;
    size_t m_nKernelSize;
    size_t m_nImageWidth;
    size_t m_nImageHeight;
    size_t m_nImageStride;
    int16_t* m_pGx;
    int16_t* m_pGy;
    uint16_t* m_pG;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_FEATURES_EDGES_DETE_H_ */
