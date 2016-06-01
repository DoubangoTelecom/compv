/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_GAUSS_H_)
#define _COMPV_GAUSS_H_

#include "compv/compv_config.h"
#include "compv/compv_array.h"

COMPV_NAMESPACE_BEGIN()

template<class T>
class COMPV_API CompVGaussKern
{
public:
    static COMPV_ERROR_CODE buildKern2(CompVPtr<CompVArray<T>* >* kern, int size, float sigma);
    static COMPV_ERROR_CODE buildKern1(CompVPtr<CompVArray<T>* >* kern, int size, float sigma);
    static COMPV_ERROR_CODE buildKern1_fxp(CompVPtr<CompVArray<uint16_t>* >* kern_fxp, int size, float sigma);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_GAUSS_H_ */
