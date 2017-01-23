/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/compv_common.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

// Private function used as extern
COMPV_API const char* CompVGetErrorString(COMPV_ERROR_CODE code)
{
    switch (code) {
    case COMPV_ERROR_CODE_S_OK:
        return "Success";
    case COMPV_ERROR_CODE_E_NOT_INITIALIZED:
        return "Not initialized";
    case COMPV_ERROR_CODE_E_INVALID_PARAMETER:
        return "Invalid parameter";
    case COMPV_ERROR_CODE_E_INVALID_STATE:
        return "Invalid state";
    default:
        return "Unknown code";
    }
}

COMPV_NAMESPACE_END()