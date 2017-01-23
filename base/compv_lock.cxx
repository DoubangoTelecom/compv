/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/base/compv_lock.h"

COMPV_NAMESPACE_BEGIN()

CompVLock::CompVLock()
{
    COMPV_CHECK_CODE_ASSERT(CompVMutex::newObj(&m_ptrSDLMutex));
}

CompVLock::~CompVLock()
{

}

COMPV_NAMESPACE_END()

