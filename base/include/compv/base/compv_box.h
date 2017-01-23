/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_BOX_H_)
#define _COMPV_BASE_BOX_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_debug.h"
#include "compv/base/parallel/compv_mutex.h"
#include "compv/base/math/compv_math_utils.h"

COMPV_NAMESPACE_BEGIN() 

#if !defined(kCompVBoxMemGrowth)
#	define kCompVBoxMemGrowth		(1 << 20) // 1MB
#endif
#if !defined(kCompVBoxItemMaxSize)
#	define kCompVBoxItemMaxSize		128 // Box should be used for small object only
#endif

template<class T>
class COMPV_BASE_API CompVBox : public CompVObj
{
protected:
	CompVBox(size_t nCapacity = 0, bool bLockable = false)
		: m_eSortType(COMPV_SORT_TYPE_QUICK)
		, m_pMem(NULL)
		, m_nItemSize(sizeof(T))
		, m_nSize(0)
		, m_nCapacity(0) 
	{
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
	COMPV_OBJECT_GET_ID(CompVBox);

	COMPV_ERROR_CODE lock() {
		COMPV_CHECK_EXP_RETURN(!m_Mutex, COMPV_ERROR_CODE_E_INVALID_CALL);
		return m_Mutex->lock();
	}
	COMPV_ERROR_CODE unlock() {
		COMPV_CHECK_EXP_RETURN(!m_Mutex, COMPV_ERROR_CODE_E_INVALID_CALL);
		return m_Mutex->unlock();
	}

	COMPV_INLINE COMPV_ERROR_CODE new_item(T** newIem) {
		if (m_nCapacity == m_nSize) {
			size_t itemsPerGrowth = ((kCompVBoxMemGrowth + m_nItemSize) / m_nItemSize);
			size_t newCapacity = (m_nCapacity < itemsPerGrowth) ? itemsPerGrowth : (m_nCapacity << 1); // start with itemsPerGrowth then double it
			COMPV_CHECK_CODE_RETURN(alloc(newCapacity));
		}
		*newIem = end();
		++m_nSize;
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_INLINE COMPV_ERROR_CODE push(const T& elem) {
		T* newIem;
		COMPV_CHECK_CODE_RETURN(new_item(&newIem));
		*newIem = elem;
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_INLINE const T* pop_back() {
		if (m_nSize) {
			--m_nSize;
			return end();
		}
		return NULL;
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
	COMPV_INLINE void free() {
		CompVMem::free((void**)&m_pMem);
		m_nSize = 0;
		m_nCapacity = 0;
	}
	COMPV_INLINE void resize(size_t nNewSize = 0, bool bForce = false) {
		if (bForce) {
			m_nSize = COMPV_MATH_MIN(nNewSize, m_nCapacity);
		}
		else if (nNewSize < m_nSize) {
			m_nSize = nNewSize;
		}
	}
	// Reset all items without freeing the allocated memory
	// Usefull if you want to re-use the list
	COMPV_INLINE void reset() {
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
		return ptr(idx);
	};
	COMPV_INLINE T* ptr(size_t idx = 0) const {
		return idx < m_nSize ? (m_pMem + idx) : NULL;
	};

	COMPV_INLINE void erase(const T* position) {
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
				if (CompVBoxPredicateMatch(ptr(i))) {
					erase(ptr(i));
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
			COMPV_DEBUG_ERROR_EX("CompVBox", "Boxing is only allowed on object with size < %u, you're boxing an object with size = %u", (unsigned)kCompVBoxItemMaxSize, (unsigned)sizeof(T));
			return COMPV_ERROR_CODE_E_INVALID_CALL;
		}
		COMPV_CHECK_EXP_RETURN(!box, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVPtr<CompVBox<T>* > box_;

		box_ = new CompVBox<T>(nCapacity, bLockable);
		COMPV_CHECK_EXP_RETURN(!box_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		COMPV_CHECK_CODE_RETURN(box_->init()); // all initializations must be called inside 'init()' to make sure childs (e.g. CompVBoxInterestPoint) have same code
		//!\ Must not any initialization code here
		*box = box_;
		return COMPV_ERROR_CODE_S_OK;
	}

protected:
	COMPV_ERROR_CODE init(size_t nCapacity = 0, bool bLockable = false) {
		COMPV_CHECK_EXP_RETURN(bLockable && !m_Mutex, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		COMPV_CHECK_EXP_RETURN(nCapacity && !m_pMem, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		// for now nothing special is done here
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
		if (CompVBoxPredicateCompare(self->ptr(0), self->ptr(0))) {
			COMPV_DEBUG_ERROR("Invalid compare function");
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		}
		size_t j, size_minus1 = self->size() - 1;
		T *a, *b, c;
		bool swapped;

		do {
			for (swapped = false, j = 0, a = self->ptr(0), b = self->ptr(1); j < size_minus1; ++j, ++a, ++b) {
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
	static COMPV_INLINE void sort_quick(CompVBox<T>* self, bool(*CompVBoxPredicateCompare)(const T*, const T*), intptr_t left, intptr_t right) {
		// This is a private function, up to the caller to check input params
		const T pivot = *self->ptr((left + right) >> 1);
		T atk, *ati = self->ptr(left), *atj = self->ptr(right);
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
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	COMPV_SORT_TYPE m_eSortType;
	T* m_pMem;
	size_t m_nItemSize;
	size_t m_nSize;
	size_t m_nCapacity;
	CompVMutexPtr m_Mutex;
	COMPV_VS_DISABLE_WARNINGS_END()
};

typedef CompVBox<CompVDMatch > CompVBoxDMatch;
typedef CompVPtr<CompVBoxDMatch *> CompVBoxDMatchPtr;
typedef CompVBoxDMatchPtr* CompVBoxDMatchPtrPtr;

COMPV_OBJECT_DECLARE_PTRS(BoxInterestPoint)

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_BOX_H_ */
