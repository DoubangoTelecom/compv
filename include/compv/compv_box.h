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
#include "compv/compv_mem.h"
#include "compv/compv_common.h"
#include "compv/compv_math_utils.h"
#include "compv/parallel/compv_mutex.h"

COMPV_NAMESPACE_BEGIN()

#if !defined(kCompVBoxMemGrowth)
#	define kCompVBoxMemGrowth		(1 << 20) // 1MB
#endif
#if !defined(kCompVBoxItemMaxSize)
#	define kCompVBoxItemMaxSize		128 // Box should be used for small object only
#endif

template<class T>
class COMPV_API CompVBox : public CompVObj
{
protected:
    CompVBox(size_t nCapacity = 0, bool bLockable = false)
        : m_pMem(NULL)
        , m_nCapacity(0)
        , m_nSize(0)
        , m_eSortType(COMPV_SORT_TYPE_QUICK) {
        m_nItemSize = sizeof(T);
        if (bLockable) {
            CompVMutex::newObj(&m_Mutex);
        }
        if (nCapacity) {
            alloc(nCapacity);
        }
    }
public:
    virtual ~CompVBox() {
        free();
    }
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVBox";
    };

    COMPV_ERROR_CODE lock() {
        COMPV_CHECK_EXP_RETURN(!m_Mutex, COMPV_ERROR_CODE_E_INVALID_CALL);
        return m_Mutex->lock();
    }
    COMPV_ERROR_CODE unlock() {
        COMPV_CHECK_EXP_RETURN(!m_Mutex, COMPV_ERROR_CODE_E_INVALID_CALL);
        return m_Mutex->unlock();
    }

    COMPV_ERROR_CODE push(const T& elem) {
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
    COMPV_ERROR_CODE append(const T* begin, const T* end) { // append up2end, "end" excluded
        if (begin && end && begin < end) { // begin = null means empty array
            size_t newSize = size() + (end - begin);
            if (capacity() < newSize) {
                size_t newCapacity = newSize;
                COMPV_CHECK_CODE_RETURN(alloc(newCapacity));
            }
            CompVMem::copy((void*)(m_pMem + size()), (void*)begin, (end - begin)*m_nItemSize);
            m_nSize = newSize;
        }
        return COMPV_ERROR_CODE_S_OK;
    }
    void free() {
        CompVMem::free((void**)&m_pMem);
        m_nSize = 0;
        m_nCapacity = 0;
    }
    // Reset all items without freeing the allocated memory
    // Usefull if you want to re-use the list
    void resize(size_t nNewSize = 0) {
        if (nNewSize < m_nSize) {
            m_nSize = nNewSize;
        }
    }
    void reset() {
        resize(0);
    }

    COMPV_INLINE size_t size()const {
        return m_nSize;
    }
    COMPV_INLINE size_t capacity()const {
        return m_nCapacity;
    }
    COMPV_INLINE bool empty() {
        return (size() == 0);
    }

    COMPV_INLINE T* begin()const {
        return m_pMem;
    }
    COMPV_INLINE T* end()const {
        return (m_pMem + m_nSize);
    }
    COMPV_INLINE T* operator[](size_t idx) const {
        return at(idx);
    };
    COMPV_INLINE T* at(size_t idx) const {
        return idx < m_nSize ? (m_pMem + idx) : NULL;
    };

    void erase(const T* position) {
        if (m_nSize > 0 && position >= begin() && position < end()) {
            if (m_nSize > 1) {
                // move last element to position
                *(const_cast<T*>(position)) = *(end() - 1);
            }
            m_nSize--;
        }
    }
    void erase(bool(*CompVBoxPredicateMatch)(const T*)) {
        if (m_nSize > 0 && CompVBoxPredicateMatch) {
            for (size_t i = 0; i < size();) {
                if (CompVBoxPredicateMatch(at(i))) {
                    erase(at(i));
                }
                else {
                    ++i;
                }
            }
        }
    }

    virtual COMPV_ERROR_CODE sort(bool(*CompVBoxPredicateCompare)(const T*, const T*)) {
        COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // Should be multithreaded. See "CompVBoxInterestPoint::sort" or "retainBest()" as reference
        COMPV_CHECK_EXP_RETURN(!CompVBoxPredicateCompare, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        if (size() > 1) {
            switch (m_eSortType) {
            case COMPV_SORT_TYPE_BUBBLE: {
                return sort_bubble(this, CompVBoxPredicateCompare);
            }
            default: {
                intptr_t size_minus1 = (intptr_t)size() - 1; // sort_quick is a macro -> don't pass a function
                sort_quick(this, CompVBoxPredicateCompare, 0, size_minus1);
                return COMPV_ERROR_CODE_S_OK;
            }
            }
        }
        return COMPV_ERROR_CODE_S_OK;
    }

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVBox<T >* >* box, size_t nCapacity = 0, bool bLockable = false) {
        if (sizeof(T) > kCompVBoxItemMaxSize) {
            COMPV_DEBUG_ERROR("Boxing is only allowed on object with size < %u, you're boxing an object with size = %u", (unsigned)kCompVBoxItemMaxSize, (unsigned)sizeof(T));
            return COMPV_ERROR_CODE_E_INVALID_CALL;
        }
        COMPV_CHECK_EXP_RETURN(!box, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        CompVPtr<CompVBox<T>* > box_;

        box_ = new CompVBox<T>(nCapacity, bLockable);
        COMPV_CHECK_EXP_RETURN(!box_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
        COMPV_CHECK_EXP_RETURN(bLockable && !box_->m_Mutex, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
        COMPV_CHECK_EXP_RETURN(nCapacity && !box_->m_pMem, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

        // If you update this code, please check "CompVBoxInterestPoint::newObj" to see if it needs cleanup

        *box = box_;

        return COMPV_ERROR_CODE_S_OK;
    }

private:
    COMPV_ERROR_CODE alloc(size_t nCapacity) {
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
            CompVMem::copy(pNewMem, m_pMem, sizeToCopy); // TODO(dmi): NTA copy only if (sizeToCopy > cache1Size)
        }
        CompVMem::free((void**)&m_pMem);
        m_pMem = pNewMem;
#endif

        m_nCapacity = nCapacity;
        return COMPV_ERROR_CODE_S_OK;
    }

    // https://en.wikipedia.org/wiki/Bubble_sort
    static COMPV_ERROR_CODE sort_bubble(CompVBox<T>* self, bool(*CompVBoxPredicateCompare)(const T*, const T*)) {
        // This is a private function, up to the caller to check imput parameters and m_nSize

        // this requires CompVBoxPredicateCompare to reture false for equality, otherwise we'll have an endless loop
        // e.g. compare(i, j) = (i > j) instead of compare(i, j) = (i >= j)
        if (CompVBoxPredicateCompare(self->at(0), self->at(0))) {
            COMPV_DEBUG_ERROR("Invalid compare function");
            COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        }
        size_t j, size_minus1 = self->size() - 1;
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
        }
        while (swapped);

        return COMPV_ERROR_CODE_S_OK;
    }

    // https://en.wikipedia.org/wiki/Quicksort
    static COMPV_INLINE void sort_quick(CompVBox<T>* self, bool(*CompVBoxPredicateCompare)(const T*, const T*), intptr_t left, intptr_t right) {
        // This is a private function, up to the caller to check input params
        const T pivot = *self->at((left + right) >> 1);
        T atk, *ati = self->at(left), *atj = self->at(right);
        const T *ati_ = ati, *atj_ = atj;
        while (ati <= atj) {
            while (CompVBoxPredicateCompare(ati, &pivot)) {
                ++ati;
            }
            while (CompVBoxPredicateCompare(&pivot, atj)) {
                --atj;
            }
            if (ati > atj) {
                break;
            }
            atk = *ati;
            *ati = *atj;
            *atj = atk;
            ++ati;
            --atj;
        }
        intptr_t i = left + (ati - ati_);
        intptr_t j = right + (atj - atj_);
        if (left < j) {
            sort_quick(self, CompVBoxPredicateCompare, left, j);
        }
        if (i < right) {
            sort_quick(self, CompVBoxPredicateCompare, i, right);
        }
    }

protected:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    COMPV_SORT_TYPE m_eSortType;
    T* m_pMem;
    size_t m_nItemSize;
    size_t m_nSize;
    size_t m_nCapacity;
    CompVPtr<CompVMutex*> m_Mutex;
    COMPV_DISABLE_WARNINGS_END()
};

template class CompVBox<CompVInterestPoint >;

COMPV_NAMESPACE_END()

#endif /* _COMPV_BOX_H_ */
