/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
* Copyright (C) 2016 Mamadou DIOP.
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
#include "compv/compv_box.h"
#include "compv/compv_mem.h"
#include "compv/compv_engine.h"
#include "compv/compv_mathutils.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

static const size_t kCompVBoxMemGrowth = 1 << 20; // 1MB
static const size_t kCompVBoxItemMaxSize = 128; // Box should be used for small object only
#if !defined(COMPV_QUICKSORT_MACRO)
#define COMPV_QUICKSORT_MACRO	0
#endif
#define COMPV_QUICKSORT_MIN_SAMPLES_PER_THREAD 100*100

template<class T>
CompVBox<T>::CompVBox()
	: m_pMem(NULL)
	, m_nCapacity(0)
	, m_nSize(0)
	, m_eSortType(COMPV_SORT_TYPE_QUICK)
{
	m_nItemSize = sizeof(T);
}

template<class T>
CompVBox<T>::~CompVBox()
{
	free();
}

template<class T>
COMPV_ERROR_CODE CompVBox<T>::alloc(size_t nCapacity)
{
	COMPV_CHECK_EXP_RETURN(!nCapacity, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	size_t newMemSize = nCapacity * m_nItemSize;

#if 0 // realloc (sloow)
	m_pMem = (T*)CompVMem::realloc(m_pMem, newMemSize);
#else
	T* pNewMem = (T*)CompVMem::malloc(newMemSize); // some systems don't support realloc_aligned(), so we're copying our data ourself. Also, realloc() is slooow on some systems
	COMPV_CHECK_EXP_RETURN(!pNewMem, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	// copy items
	if (m_nSize) {
		m_nSize = COMPV_MATH_CLIP3(0, nCapacity, m_nSize);
		size_t sizeToCopy = m_nSize * m_nItemSize;
		CompVMem::copyNTA(pNewMem, m_pMem, sizeToCopy); // TODO(dmi): NTA copy only if (sizeToCopy > cache1Size)
	}
	CompVMem::free((void**)&m_pMem);
	m_pMem = pNewMem;
#endif

	m_nCapacity = nCapacity;
	return COMPV_ERROR_CODE_S_OK;
}

template<class T>
COMPV_ERROR_CODE CompVBox<T>::lock()
{
	COMPV_CHECK_EXP_RETURN(!m_Mutex, COMPV_ERROR_CODE_E_INVALID_CALL);
	return m_Mutex->lock();
}

template<class T>
COMPV_ERROR_CODE CompVBox<T>::unlock()
{
	COMPV_CHECK_EXP_RETURN(!m_Mutex, COMPV_ERROR_CODE_E_INVALID_CALL);
	return m_Mutex->unlock();
}

template<class T>
COMPV_ERROR_CODE CompVBox<T>::push(const T& elem)
{
	if (m_nCapacity == m_nSize) {
		size_t itemsPerGrowth = ((kCompVBoxMemGrowth + m_nItemSize) / m_nItemSize);
		size_t newCapacity = (m_nCapacity < itemsPerGrowth) ? itemsPerGrowth : (m_nCapacity << 1); // start with itemsPerGrowth then double it
		COMPV_CHECK_CODE_RETURN(alloc(newCapacity));
	}
	T* newItem = end();
	*newItem = elem;

	++m_nSize;
	return COMPV_ERROR_CODE_S_OK;
}

template<class T>
COMPV_ERROR_CODE CompVBox<T>::free()
{
	CompVMem::free((void**)&m_pMem);
	m_nSize = 0;
	m_nCapacity = 0;
	return COMPV_ERROR_CODE_S_OK;
}

template<class T>
COMPV_ERROR_CODE CompVBox<T>::resize(size_t nNewSize /*= 0*/)
{
	m_nSize = nNewSize;
	return COMPV_ERROR_CODE_S_OK;
}

// Reset all items without freeing the allocated memory
// Usefull if you want to re-use the list
template<class T>
COMPV_ERROR_CODE CompVBox<T>::reset()
{
	return resize(0);
}

template<class T>
void CompVBox<T>::erase(const T* position)
{
	if (m_nSize > 0 && position >= begin() && position < end()) {
		if (m_nSize > 1) {
			// move last element to position
			*(const_cast<T*>(position)) = *(end() - 1);
		}
		m_nSize--;
	}
}

template<class T>
void CompVBox<T>::erase(bool(*CompVBoxPredicateMatch)(const T*))
{
	if (m_nSize > 0 && CompVBoxPredicateMatch) {
		for (size_t i = 0; i < size(); ) {
			if (CompVBoxPredicateMatch(at(i))) {
				erase(at(i));
			}
			else {
				++i;
			}
		}
	}
}

// https://en.wikipedia.org/wiki/Bubble_sort
template<class T>
static COMPV_ERROR_CODE __sort_bubble(CompVBox<T>* self, bool(*CompVBoxPredicateCompare)(const T*, const T*))
{
	// This is a private function, up to the caller to check imput parameters and m_nSize

	// this requires CompVBoxPredicateCompare to reture false for equality, otherwise we'll have an endless loop
	// e.g. compare(i, j) = (i > j) instead of compare(i, j) = (i >= j) 
	if (CompVBoxPredicateCompare(self->at(0), self->at(0))) {
		COMPV_DEBUG_ERROR("Invalid compare function");
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	}
	size_t i = 0, j, size_minus1 = self->size() - 1;
	T *a, *b, c;
	bool swapped;

	do {
		for (swapped = false, j = 0, a = self->at(0), b = self->at(1); j < size_minus1; ++j, ++a, ++b) {
			if (CompVBoxPredicateCompare(b, a)) {
				c = *b, *b = *a, *a = c; // swap
				swapped = true;
			}
		}
		--size_minus1;
	} while (swapped);

	return COMPV_ERROR_CODE_S_OK;
}

// https://en.wikipedia.org/wiki/Quicksort
#if COMPV_QUICKSORT_MACRO
#define __sort_quick(self, CompVBoxPredicateCompare, left, right) do { \
	const T pivot = *self->at((left + right) >> 1);  \
	T atk, *ati = self->at(left), *atj = self->at(right);  \
	const T *ati_ = ati, *atj_ = atj;  \
	while (ati <= atj) {  \
		while (CompVBoxPredicateCompare(ati, &pivot)) ++ati;  \
		while (CompVBoxPredicateCompare(&pivot, atj)) --atj;  \
		if (ati > atj) break;  \
		atk = *ati;  \
		*ati = *atj;  \
		*atj = atk;  \
		++ati;  \
		--atj;  \
	}  \
	size_t i = left + (ati - ati_);  \
	size_t j = right + (atj - atj_);  \
	if (left < j) __sort_quick(self, CompVBoxPredicateCompare, left, j);  \
	if (i < right) __sort_quick(self, CompVBoxPredicateCompare, i, right);  \
} while(0)
#else
template<class T>
static void __sort_quick(CompVBox<T>* self, bool(*CompVBoxPredicateCompare)(const T*, const T*), size_t left, size_t right);
template<class T>
static COMPV_ERROR_CODE __sort_quick_asyn_exec(const struct compv_asynctoken_param_xs* pc_params)
{
	CompVBox<T>* self = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[0].pcParamPtr, CompVBox<T>*);
	bool(*CompVBoxPredicateCompare)(const T*, const T*) = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[1].pcParamPtr, bool(*)(const T*, const T*));
	size_t left = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[2].pcParamPtr, size_t);
	size_t right = COMPV_ASYNCTASK_GET_PARAM_ASIS(pc_params[3].pcParamPtr, size_t);
	__sort_quick(self, CompVBoxPredicateCompare, left, right);
	return COMPV_ERROR_CODE_S_OK;
}
template<class T>
static COMPV_INLINE void __sort_quick(CompVBox<T>* self, bool(*CompVBoxPredicateCompare)(const T*, const T*), intptr_t left, intptr_t right)
{
	// This is a private function, up to the caller to check input params
#if 0
	CompVObjWrapper<CompVThreadDispatcher* >&threadDip = CompVEngine::getThreadDispatcher();
	int32_t threadsCount = threadDip ? threadDip->getThreadsCount() : 0;
	uint32_t threadIdx0 = UINT_MAX, threadIdx1 = UINT_MAX;
#endif
	const T pivot = *self->at((left + right) >> 1);
	T atk, *ati = self->at(left), *atj = self->at(right);
	const T *ati_ = ati, *atj_ = atj;
	while (ati <= atj) {
		while (CompVBoxPredicateCompare(ati, &pivot)) ++ati;
		while (CompVBoxPredicateCompare(&pivot, atj)) --atj;
		if (ati > atj) break;
		atk = *ati;
		*ati = *atj;
		*atj = atk;
		++ati;
		--atj;
	}
	intptr_t i = left + (ati - ati_);
	intptr_t j = right + (atj - atj_);
	if (left < j) {
#if 0
		if (threadsCount > 2 && (j - left) > COMPV_QUICKSORT_MIN_SAMPLES_PER_THREAD && !threadDip->isMotherOfTheCurrentThread()) {
			threadIdx0 = threadDip->getThreadIdxForNextToCurrentCore();
			COMPV_CHECK_CODE_ASSERT(threadDip->execute(threadIdx0, COMPV_TOKENIDX_QUICKSORT, __sort_quick_asyn_exec,
				COMPV_ASYNCTASK_SET_PARAM_ASISS(self, CompVBoxPredicateCompare, left, j),
				COMPV_ASYNCTASK_SET_PARAM_NULL()));
		}
		else 
#endif
		{
			__sort_quick(self, CompVBoxPredicateCompare, left, j);
		}
	}
	if (i < right) {
#if 0
		if (threadsCount > 2 && (right - i) > COMPV_QUICKSORT_MIN_SAMPLES_PER_THREAD && !threadDip->isMotherOfTheCurrentThread()) {
			threadIdx1 = threadDip->getThreadIdxForNextToCurrentCore() + 1;
			COMPV_CHECK_CODE_ASSERT(threadDip->execute(threadIdx1, COMPV_TOKENIDX_QUICKSORT, __sort_quick_asyn_exec,
				COMPV_ASYNCTASK_SET_PARAM_ASISS(self, CompVBoxPredicateCompare, i, right),
				COMPV_ASYNCTASK_SET_PARAM_NULL()));
		}
		else
#endif
		{
			__sort_quick(self, CompVBoxPredicateCompare, i, right);
		}
	}
#if 0
	if (threadIdx0 != UINT_MAX) {
		COMPV_CHECK_CODE_ASSERT(threadDip->wait(threadIdx0, COMPV_TOKENIDX_QUICKSORT));
	}
	if (threadIdx1 != UINT_MAX) {
		COMPV_CHECK_CODE_ASSERT(threadDip->wait(threadIdx1, COMPV_TOKENIDX_QUICKSORT));
	}
#endif
}


#endif /* COMPV_QUICKSORT_MACRO */

template<class T>
COMPV_ERROR_CODE CompVBox<T>::sort(bool(*CompVBoxPredicateCompare)(const T*, const T*))
{
	COMPV_CHECK_EXP_RETURN(!CompVBoxPredicateCompare, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (size() > 1) {
		switch (m_eSortType) {
			case COMPV_SORT_TYPE_BUBBLE:
			{
				return __sort_bubble(this, CompVBoxPredicateCompare);
			}
			default:
			{
				intptr_t size_minus1 = (intptr_t)size() - 1; // sort_quick is a macro -> don't pass a function
				__sort_quick(this, CompVBoxPredicateCompare, 0, size_minus1);
				return COMPV_ERROR_CODE_S_OK;
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

template<class T>
COMPV_ERROR_CODE CompVBox<T>::newObj(CompVObjWrapper<CompVBox<T >* >* box, size_t nCapacity /*= 0*/, bool bLockable /*= false*/)
{
	if (sizeof(T) > kCompVBoxItemMaxSize) {
		COMPV_DEBUG_ERROR("Boxing is only allowed on object with size < %u, you're boxing an object with size = ", (unsigned)kCompVBoxItemMaxSize, sizeof(T));
		return COMPV_ERROR_CODE_E_INVALID_CALL;
	}
	COMPV_CHECK_EXP_RETURN(!box, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVObjWrapper<CompVBox<T>* > box_;

	box_ = new CompVBox<T>();
	COMPV_CHECK_EXP_RETURN(!box_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	if (bLockable) {
		COMPV_CHECK_CODE_RETURN(CompVMutex::newObj(&box_->m_Mutex));
	}
	if (nCapacity > 0) {
		COMPV_CHECK_CODE_RETURN(box_->alloc(nCapacity));
	}

	*box = box_;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
