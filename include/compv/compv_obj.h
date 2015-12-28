/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#if !defined(_COMPV_OBJECT_H_)
#define _COMPV_OBJECT_H_

#include "compv/compv_config.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVObj
{
public:
	CompVObj() :m_nRefCount(0) {}
	CompVObj(const CompVObj &) :m_nRefCount(0) {}
	virtual ~CompVObj() {}

public:
	virtual COMPV_INLINE const char* getObjectId() = 0;
#if !defined(SWIG)
	COMPV_INLINE long getRefCount() const {
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

private:
	volatile long m_nRefCount;
};

//
//	CompVObjWrapper declaration
//
template<class CompVObjType>
class CompVObjWrapper
{

public:
	COMPV_INLINE CompVObjWrapper(CompVObjType obj
#if !defined(SWIG)
		= NULL
#endif
		);
	COMPV_INLINE CompVObjWrapper(const CompVObjWrapper<CompVObjType> &obj);
	virtual ~CompVObjWrapper();

public:
#if defined(SWIG)
	CompVObjType unWrap() { return getWrappedObject(); }
#else
	COMPV_INLINE CompVObjWrapper<CompVObjType>& operator=(const CompVObjType other);
	COMPV_INLINE CompVObjWrapper<CompVObjType>& operator=(const CompVObjWrapper<CompVObjType> &other);
	COMPV_INLINE bool operator ==(const CompVObjWrapper<CompVObjType> other) const;
	COMPV_INLINE bool operator!=(const CompVObjWrapper<CompVObjType> &other) const;
	COMPV_INLINE bool operator <(const CompVObjWrapper<CompVObjType> other) const;
	COMPV_INLINE CompVObjType operator->() const;
	COMPV_INLINE CompVObjType operator*() const;
	COMPV_INLINE operator bool() const;
#endif

protected:
	long takeRef();
	long releaseRef();

	CompVObjType getWrappedObject() const;
	void wrapObject(CompVObjType obj);

private:
	CompVObjType m_WrappedObject;
};

//
//	CompVObjWrapper implementation
//
template<class CompVObjType>
CompVObjWrapper<CompVObjType>::CompVObjWrapper(CompVObjType obj)
{
	wrapObject(obj), takeRef();
}

template<class CompVObjType>
CompVObjWrapper<CompVObjType>::CompVObjWrapper(const CompVObjWrapper<CompVObjType> &obj)
{
	wrapObject(obj.getWrappedObject()),
		takeRef();
}

template<class CompVObjType>
CompVObjWrapper<CompVObjType>::~CompVObjWrapper()
{
	releaseRef(),
		wrapObject(NULL);
}


template<class CompVObjType>
long CompVObjWrapper<CompVObjType>::takeRef()
{
	if (m_WrappedObject /*&& m_WrappedObject->getRefCount() At startup*/) {
		return m_WrappedObject->takeRef();
	}
	return 0;
}

template<class CompVObjType>
long CompVObjWrapper<CompVObjType>::releaseRef()
{
	if (m_WrappedObject && m_WrappedObject->getRefCount()) {
		if (m_WrappedObject->releaseRef() == 0) {
			delete m_WrappedObject, m_WrappedObject = NULL;
		}
		else {
			return m_WrappedObject->getRefCount();
		}
	}
	return 0;
}

template<class CompVObjType>
CompVObjType CompVObjWrapper<CompVObjType>::getWrappedObject() const
{
	return m_WrappedObject;
}

template<class CompVObjType>
void CompVObjWrapper<CompVObjType>::wrapObject(const CompVObjType obj)
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
CompVObjWrapper<CompVObjType>& CompVObjWrapper<CompVObjType>::operator=(const CompVObjType obj)
{
	releaseRef();
	wrapObject(obj), takeRef();
	return *this;
}

template<class CompVObjType>
CompVObjWrapper<CompVObjType>& CompVObjWrapper<CompVObjType>::operator=(const CompVObjWrapper<CompVObjType> &obj)
{
	releaseRef();
	wrapObject(obj.getWrappedObject()), takeRef();
	return *this;
}

template<class CompVObjType>
bool CompVObjWrapper<CompVObjType>::operator ==(const CompVObjWrapper<CompVObjType> other) const
{
	return getWrappedObject() == other.getWrappedObject();
}

template<class CompVObjType>
bool CompVObjWrapper<CompVObjType>::operator!=(const CompVObjWrapper<CompVObjType> &other) const
{
	return getWrappedObject() != other.getWrappedObject();
}

template<class CompVObjType>
bool CompVObjWrapper<CompVObjType>::operator <(const CompVObjWrapper<CompVObjType> other) const
{
	return getWrappedObject() < other.getWrappedObject();
}

template<class CompVObjType>
CompVObjWrapper<CompVObjType>::operator bool() const
{
	return (getWrappedObject() != NULL);
}

template<class CompVObjType>
CompVObjType CompVObjWrapper<CompVObjType>::operator->() const
{
	return getWrappedObject();
}

template<class CompVObjType>
CompVObjType CompVObjWrapper<CompVObjType>::operator*() const
{
	return getWrappedObject();
}

COMPV_NAMESPACE_END()

#endif /* _COMPV_OBJECT_H_ */
