/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_BIND_H_)
#define _COMPV_BASE_BIND_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVBind
{
public:
    CompVBind();
    virtual ~CompVBind();
    virtual COMPV_ERROR_CODE bind() = 0;
    virtual COMPV_ERROR_CODE unbind() = 0;
};

template <typename T>
class CompVAutoBind
{
public:
    explicit CompVAutoBind(T* objThis) : m_ptrThis(objThis) {
        COMPV_CHECK_CODE_ASSERT(m_ptrThis->bind());
    }
    virtual ~CompVAutoBind() {
        COMPV_CHECK_CODE_ASSERT(m_ptrThis->unbind());
    }
private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    T* m_ptrThis;
    COMPV_VS_DISABLE_WARNINGS_END()
};

#define COMPV_AUTOBIND(T, Obj) CompVAutoBind<T> __COMPV_autoBind__(Obj)

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_BIND_H_ */
