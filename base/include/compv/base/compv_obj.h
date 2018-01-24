/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_OBJECT_H_)
#define _COMPV_BASE_OBJECT_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_base.h"

COMPV_NAMESPACE_BEGIN()

#define COMPV_OBJECT_GET_ID(objName) virtual COMPV_INLINE const char* objectId()override { return #objName; };
#define COMPV_OBJECT_DECLARE_PTRS(objName) \
	class CompV##objName;  \
	typedef CompVPtr<CompV##objName*> CompV##objName##Ptr;  \
	typedef CompV##objName##Ptr* CompV##objName##PtrPtr;


class COMPV_BASE_API CompVObj
{
public:
    CompVObj() : m_nRefCount(0) {
		COMPV_ASSERT(CompVBase::isInitialized() || CompVBase::isInitializing()); // Must call 'CompVInit' from 'compv_api.h'
        compv_atomic_inc(&s_nObjCount);
    }
    CompVObj(const CompVObj &) :m_nRefCount(0) {
        compv_atomic_inc(&s_nObjCount);
    }
    virtual ~CompVObj() {
        compv_atomic_dec(&s_nObjCount);
    }

public:
    virtual COMPV_INLINE const char* objectId() = 0;
#if !defined(SWIG)
    COMPV_INLINE long refCount() const {
        return m_nRefCount;
    }
    void operator=(const CompVObj &) {}
#endif


public:
    COMPV_INLINE long takeRef() { /*const*/
        compv_atomic_inc(&m_nRefCount);
        return m_nRefCount;
    }
    COMPV_INLINE long releaseRef() { /*const*/
        if (m_nRefCount) { // must never be equal to zero
            compv_atomic_dec(&m_nRefCount);
        }
        return m_nRefCount;
    }
    static long totalObjCount() {
        return s_nObjCount;
    }
    static bool isEmpty() {
        return totalObjCount() == 0;
    }

private:
    volatile long m_nRefCount;
    static long s_nObjCount;
};

//
//	CompVPtr declaration
//
template<class CompVObjType>
class CompVPtr
{

public:
    COMPV_INLINE CompVPtr(CompVObjType obj
#if !defined(SWIG)
                          = NULL
#endif
                         );
    COMPV_INLINE CompVPtr(const CompVPtr<CompVObjType> &obj);
    virtual ~CompVPtr();

public:
#if defined(SWIG)
    CompVObjType unWrap() {
        return wrappedObject();
    }
#else
    COMPV_INLINE CompVPtr<CompVObjType>& operator=(const CompVObjType other);
    COMPV_INLINE CompVPtr<CompVObjType>& operator=(const CompVPtr<CompVObjType> &other);
    COMPV_INLINE bool operator ==(const CompVPtr<CompVObjType> other) const;
    COMPV_INLINE bool operator!=(const CompVPtr<CompVObjType> &other) const;
    COMPV_INLINE bool operator <(const CompVPtr<CompVObjType> other) const;
    COMPV_INLINE CompVObjType operator->() const;
    COMPV_INLINE CompVObjType operator*() const;
    COMPV_INLINE operator bool() const;
#endif

protected:
    long takeRef();
    long releaseRef();

    CompVObjType wrappedObject() const;
    void wrapObject(CompVObjType obj);

private:
    CompVObjType m_WrappedObject;
};

//
//	CompVPtr implementation
//
template<class CompVObjType>
CompVPtr<CompVObjType>::CompVPtr(CompVObjType obj)
{
    wrapObject(obj), takeRef();
    if (obj) {
        //--COMPV_DEBUG_INFO("%s::ctor()", obj->getObjectId());
    }
}

template<class CompVObjType>
CompVPtr<CompVObjType>::CompVPtr(const CompVPtr<CompVObjType> &obj)
{
    wrapObject(obj.wrappedObject()), takeRef();
    if (obj) {
        //--COMPV_DEBUG_INFO("%s::ctor()", obj->getObjectId());
    }
}

template<class CompVObjType>
CompVPtr<CompVObjType>::~CompVPtr()
{
    if (m_WrappedObject) {
        //--COMPV_DEBUG_INFO("~%s::dtor()", m_WrappedObject->getObjectId());
    }
    releaseRef(), wrapObject(NULL);
}


template<class CompVObjType>
long CompVPtr<CompVObjType>::takeRef()
{
    if (m_WrappedObject /*&& m_WrappedObject->refCount() At startup*/) {
        return m_WrappedObject->takeRef();
    }
    return 0;
}

template<class CompVObjType>
long CompVPtr<CompVObjType>::releaseRef()
{
    if (m_WrappedObject && m_WrappedObject->refCount()) {
        if (m_WrappedObject->releaseRef() == 0) {
            delete m_WrappedObject, m_WrappedObject = nullptr;
        }
        else {
            return m_WrappedObject->refCount();
        }
    }
    return 0;
}

template<class CompVObjType>
CompVObjType CompVPtr<CompVObjType>::wrappedObject() const
{
    return m_WrappedObject;
}

template<class CompVObjType>
void CompVPtr<CompVObjType>::wrapObject(const CompVObjType obj)
{
    if (obj) {
        if (!(m_WrappedObject = dynamic_cast<CompVObjType>(obj))) {
            COMPV_DEBUG_ERROR("Trying to wrap an object with an invalid type");
        }
    }
    else {
        m_WrappedObject = NULL;
    }
}

template<class CompVObjType>
CompVPtr<CompVObjType>& CompVPtr<CompVObjType>::operator=(const CompVObjType obj)
{
    releaseRef();
    wrapObject(obj), takeRef();
    return *this;
}

template<class CompVObjType>
CompVPtr<CompVObjType>& CompVPtr<CompVObjType>::operator=(const CompVPtr<CompVObjType> &obj)
{
    releaseRef();
    wrapObject(obj.wrappedObject()), takeRef();
    return *this;
}

template<class CompVObjType>
bool CompVPtr<CompVObjType>::operator ==(const CompVPtr<CompVObjType> other) const
{
    return wrappedObject() == other.wrappedObject();
}

template<class CompVObjType>
bool CompVPtr<CompVObjType>::operator!=(const CompVPtr<CompVObjType> &other) const
{
    return wrappedObject() != other.wrappedObject();
}

template<class CompVObjType>
bool CompVPtr<CompVObjType>::operator <(const CompVPtr<CompVObjType> other) const
{
    return wrappedObject() < other.wrappedObject();
}

template<class CompVObjType>
CompVPtr<CompVObjType>::operator bool() const
{
    return (wrappedObject() != NULL);
}

template<class CompVObjType>
CompVObjType CompVPtr<CompVObjType>::operator->() const
{
    return wrappedObject();
}

template<class CompVObjType>
CompVObjType CompVPtr<CompVObjType>::operator*() const
{
    return wrappedObject();
}

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_OBJECT_H_ */
