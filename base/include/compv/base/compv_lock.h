/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
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
        return !!m_ptrMutex;
    }
    COMPV_INLINE COMPV_ERROR_CODE lock() {
        COMPV_CHECK_CODE_RETURN(m_ptrMutex->lock());
        return COMPV_ERROR_CODE_S_OK;
    }
    COMPV_INLINE COMPV_ERROR_CODE unlock() {
        COMPV_CHECK_CODE_RETURN(m_ptrMutex->unlock());
        return COMPV_ERROR_CODE_S_OK;
    }
private:
    COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    CompVMutexPtr m_ptrMutex;
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

class CompVAutoLock2
{
public:
	explicit CompVAutoLock2(CompVMutexPtr& mutex) : m_Mutex(mutex) {
		m_Mutex->lock();
	}
	virtual ~CompVAutoLock2() {
		m_Mutex->unlock();
	}
private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	CompVMutexPtr m_Mutex;
	COMPV_VS_DISABLE_WARNINGS_END()
};

#define COMPV_AUTOLOCK_OBJ(T, obj)		CompVAutoLock<T> __COMPV_autoLock__((obj))
#define COMPV_AUTOLOCK_THIS(T)			COMPV_AUTOLOCK_OBJ(T, this)
#define COMPV_AUTOLOCK_THIS_CONST(T)	COMPV_AUTOLOCK_OBJ(T, const_cast<T*>(this))
#define COMPV_AUTOLOCK(mutex)			CompVAutoLock2 __COMPV_autoLock2__((mutex))

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_LOCK_H_ */
