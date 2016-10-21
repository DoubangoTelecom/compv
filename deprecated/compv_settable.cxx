/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/compv_settable.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVSettable::CompVSettable()
{

}

CompVSettable::~CompVSettable()
{

}

COMPV_ERROR_CODE CompVSettable::set(int id, const void* valuePtr, size_t valueSize)
{
    switch (id) {
    case -1:
    default:
        COMPV_DEBUG_ERROR("id=%d not supported", id);
        return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
    }
}

COMPV_ERROR_CODE CompVSettable::get(int id, const void*& valuePtr, size_t valueSize)
{
    switch (id) {
    case -1:
    default:
        COMPV_DEBUG_ERROR("id=%d not supported", id);
        return COMPV_ERROR_CODE_E_NOT_IMPLEMENTED;
    }
}

COMPV_NAMESPACE_END()
