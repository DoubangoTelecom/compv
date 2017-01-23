/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_PATCH_H_)
#define _COMPV_PATCH_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVPatch : public CompVObj
{
protected:
    CompVPatch();
public:
    virtual ~CompVPatch();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVPatch";
    };
    void moments0110(const uint8_t* ptr, int center_x, int center_y, int img_width, int img_stride, int img_height, int* m01, int* m10);
    static COMPV_ERROR_CODE newObj(CompVPtr<CompVPatch* >* patch, int diameter);

private:
    void initXYMax();

private:
    int m_nRadius;
    int16_t* m_pMaxAbscissas;
    int16_t* m_pX;
    int16_t* m_pY;
    uint8_t* m_pTop;
    uint8_t* m_pBottom;
    size_t m_nCount;
    size_t m_nStride;
    void(*m_Moments0110)(const uint8_t* top, const uint8_t* bottom, const int16_t* x, const int16_t* y, compv_scalar_t count, compv_scalar_t* s01, compv_scalar_t* s10);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PATCH_H_ */
