/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_LOCK_H_)
#define _COMPV_BASE_LOCK_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_obj.h"
#include "compv/base/parallel/compv_mutex.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVLock
{
public:
    CompVLock();
    virtual ~CompVLock();
    COMPV_INLINE bool isInitialized()const {
        return !!m_ptrSDLMutex;
    }
    COMPV_INLINE COMPV_ERROR_CODE lock() {
        COMPV_CHECK_CODE_RETURN(m_ptrSDLMutex->lock());
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_INLINE COMPV_ERROR_CODE unlock() {
        COMPV_CHECK_CODE_RETURN(m_ptrSDLMutex->unlock());
        return COMPV_ERROR_CODE_S_OK;
    }
private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    CompVMutexPtr m_ptrSDLMutex;
    COMPV_VS_DISABLE_WARNINGS_END()
};

template <typename T>
class CompVAutoLock
{
public:
    explicit CompVAutoLock(T* This) : m_pThis(This) {
        m_pThis->lock();
    }
    virtual ~CompVAutoLock() {
        m_pThis->unlock();
    }
private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    T* m_pThis;
    COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_LOCK_H_ */
