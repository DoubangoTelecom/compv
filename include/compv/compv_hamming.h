/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_HAMMING_H_)
#define _COMPV_HAMMING_H_

#include "compv/compv_config.h"
#include "compv/compv_array.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVHamming
{
public:
    static COMPV_ERROR_CODE distance(const uint8_t* dataPtr, int width, int stride, int height, const uint8_t* patch1xnPtr, int32_t* distPtr);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_HAMMING_H_ */
