/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
* Copyright (C) 2016 Mamadou DIOP
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
#if !defined(_COMPV_BOX_H_)
#define _COMPV_BOX_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"
#include "compv/parallel/compv_mutex.h"

COMPV_NAMESPACE_BEGIN()

template<class T>
class COMPV_API CompVBox : public CompVObj
{
protected:
	CompVBox();
public:
	virtual ~CompVBox();
	virtual COMPV_INLINE const char* getObjectId() { return "CompVBox"; };

	COMPV_ERROR_CODE lock();
	COMPV_ERROR_CODE unlock();

	COMPV_ERROR_CODE push(const T& elem);
	COMPV_ERROR_CODE free();
	COMPV_ERROR_CODE resize(size_t newSize = 0);
	COMPV_ERROR_CODE reset();

	COMPV_INLINE size_t size()const { return m_nSize; }
	COMPV_INLINE size_t capacity()const { return m_nCapacity; }

	COMPV_INLINE T* begin()const { return m_pMem; }
	COMPV_INLINE T* end()const { return (m_pMem + m_nSize); }
	COMPV_INLINE T* operator[](size_t idx) const { return at(idx); };
	COMPV_INLINE T* at(size_t idx) const { return idx < m_nSize ? (m_pMem + idx) : NULL; };

	void erase(const T* position);
	void erase(bool(*CompVBoxPredicateMatch)(const T*));

	COMPV_ERROR_CODE sort(bool(*CompVBoxPredicateCompare)(const T*, const T*));

	static COMPV_ERROR_CODE newObj(CompVObjWrapper<CompVBox<T >* >* box, size_t nCapacity = 0, bool bLockable = false);

private:
	COMPV_ERROR_CODE alloc(size_t nCapacity);

private:
	COMPV_SORT_TYPE m_eSortType;
	T* m_pMem;
	size_t m_nItemSize;
	size_t m_nSize;
	size_t m_nCapacity;
	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
	CompVObjWrapper<CompVMutex*> m_Mutex;
	COMPV_DISABLE_WARNINGS_END()
};

template class CompVBox<CompVInterestPoint >;
typedef CompVBox<CompVInterestPoint > CompVBoxInterestPoint;

template class CompVBox<int >;
typedef CompVBox<int > CompVBoxInt;

COMPV_NAMESPACE_END()

#endif /* _COMPV_BOX_H_ */
