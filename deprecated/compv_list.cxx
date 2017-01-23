/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/compv_list.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

/*
* Item structure: index || next_index || data
* index: The distance from the memory start. Type: size_t.
* next_index: The index for the next item. Type: size_t.
* data: the data. Type: data_t.
*/

static const size_t kCompVListIndexInvalid = (size_t)-1;
static const size_t kCompVListIndexSize = sizeof(size_t);
static const size_t kCompVListHeaderSize = /*index*/kCompVListIndexSize + /*next_index*/kCompVListIndexSize;
static const size_t kCompVListMemGrowth = 1000 << 10; // 1MB

#define COMPVLIST_PITEM(iIndex)							(void*)(((uint8_t*)(m_pMem)) + iIndex)
#define COMPVLIST_PDATA(iIndex)							(void*)(((uint8_t*)(m_pMem)) + iIndex + kCompVListHeaderSize)

#define COMPVLIST_PITEM_PDATA(pItem)					(void*)(((uint8_t*)(pItem)) + kCompVListHeaderSize)
#define COMPVLIST_PITEM_INDEX(pItem)					*((size_t*)(pItem))
#define COMPVLIST_PITEM_NEXT_INDEX(pItem)				*(((size_t*)(pItem)) + 1)

#define COMPVLIST_PDATA_ITEM(pData)						(void*)(((uint8_t*)(pData)) - kCompVListHeaderSize)
#define COMPVLIST_PDATA_NEXT_INDEX(pData)				*((size_t*)(((uint8_t*)(pData)) - kCompVListIndexSize))
#define COMPVLIST_PDATA_INDEX(pData)					*((size_t*)(((uint8_t*)(pData)) - kCompVListHeaderSize))

#define COMPVLIST_PITEM_SET_NEXT_INDEX(pItem, iIndex)	*((size_t*)(((uint8_t*)pItem) + kCompVListIndexSize)) = iIndex
#define COMPVLIST_PITEM_SET_INDEX(pItem)				*((size_t*)(m_pMem)) = (size_t)(((uint8_t*)pItem) - ((uint8_t*)m_pMem))

template<class T>
CompVList<T>::CompVList()
    : m_pHead(NULL)
    , m_pTail(NULL)
    , m_pMem(NULL)
    , m_nCapacity(0)
    , m_nItems(0)
{
    m_nDataSize = sizeof(T);
    m_nDataStride = m_nDataSize /*CompVMem::alignForward((uintptr_t)m_nDataSize, COMPV_SIMD_ALIGNV_DEFAULT)*/;
    m_nItemSize = kCompVListHeaderSize + m_nDataStride;
}

template<class T>
CompVList<T>::~CompVList()
{
    free();
}

template<class T>
void* CompVList<T>::nextFreeMemBlock(size_t** index, size_t** next_index, T** data)
{
    if (m_nCapacity > m_nItems) {
        void* mem = (((uint8_t*)m_pMem) + (m_nItems * m_nItemSize));
        *index = ((size_t*)mem);
        *next_index = *index + 1;
        *data = (T*)((uint8_t*)mem + kCompVListHeaderSize);
        return mem;
    }
    return NULL;
}

template<class T>
COMPV_ERROR_CODE CompVList<T>::lock()
{
    COMPV_CHECK_EXP_RETURN(!m_Mutex, COMPV_ERROR_CODE_E_INVALID_CALL);
    return m_Mutex->lock();
}

template<class T>
COMPV_ERROR_CODE CompVList<T>::unlock()
{
    COMPV_CHECK_EXP_RETURN(!m_Mutex, COMPV_ERROR_CODE_E_INVALID_CALL);
    return m_Mutex->unlock();
}

template<class T>
COMPV_ERROR_CODE CompVList<T>::push(const T& elem, bool bBack /*= true*/)
{
    void* pItem;
    T* pItemData;
    size_t* pItemIndex;
    size_t* pItemNextIndex;
    if (!(pItem = nextFreeMemBlock(&pItemIndex, &pItemNextIndex, &pItemData))) {
        size_t itemsPerGrowth = ((kCompVListMemGrowth + m_nItemSize) / m_nItemSize);
        size_t oldMemSize = m_nCapacity * m_nItemSize;
        size_t oldItems = m_nItems;
        size_t oldCapacity = m_nCapacity;
        size_t newCapacity = (oldCapacity < itemsPerGrowth) ? itemsPerGrowth : (oldCapacity << 1); // start with itemsPerGrowth then double it
        size_t newMemSize = newCapacity * m_nItemSize;
        size_t oldHeadIndex = m_pHead ? COMPVLIST_PITEM_INDEX(m_pHead) : kCompVListIndexInvalid;
        size_t oldTailIndex = m_pTail ? COMPVLIST_PITEM_INDEX(m_pTail) : kCompVListIndexInvalid;
        void* pNewMem = CompVMem::malloc(newMemSize);

        // Set values to zero in case memory alloc fails
        m_nItems = 0;
        m_nCapacity = 0;

        COMPV_CHECK_EXP_RETURN(!pNewMem, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
        if (m_pMem) {
            // some systems don't support realloc_aligned(), so we're copying our data ourself
            CompVMem::copyNTA(pNewMem, m_pMem, oldMemSize);
            CompVMem::free(&m_pMem);
        }
        m_pMem = pNewMem;
        m_nCapacity = newCapacity;
        m_nItems = oldItems;

        pItem = nextFreeMemBlock(&pItemIndex, &pItemNextIndex, &pItemData);

        // Update Head and tail
        if (m_pHead) {
            m_pHead = COMPVLIST_PITEM(oldHeadIndex);
            m_pTail = COMPVLIST_PITEM(oldTailIndex);
        }

        COMPV_DEBUG_INFO("Increasing list memory size from %ukB to %uKB", (unsigned)(oldMemSize>>10), (unsigned)(newMemSize>>10));
    }

    COMPV_CHECK_EXP_RETURN(!pItem, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

    *pItemData = elem;
    *pItemIndex = (size_t)((uintptr_t)pItem - (uintptr_t)m_pMem);

    bool bFirst = !m_pHead; // check it here before "m_pHead" is changed

    if (bBack && m_pTail) {
        COMPVLIST_PITEM_SET_NEXT_INDEX(m_pTail, *pItemIndex);
        m_pTail = pItem;
        COMPVLIST_PITEM_SET_NEXT_INDEX(m_pTail, kCompVListIndexInvalid);
    }
    else {
        if (m_pHead) {
            size_t headIndex = COMPVLIST_PITEM_INDEX(m_pHead);
            COMPVLIST_PITEM_SET_NEXT_INDEX(pItem, headIndex);
        }
        m_pHead = pItem;
    }

    if (bFirst) {
        m_pTail = m_pHead = pItem;
        COMPVLIST_PITEM_SET_NEXT_INDEX(m_pTail, kCompVListIndexInvalid);
    }
    m_nItems++;
    return COMPV_ERROR_CODE_S_OK;
}

template<class T>
COMPV_ERROR_CODE CompVList<T>::push_back(const T& elem)
{
    return push(elem, true);
}

template<class T>
COMPV_ERROR_CODE CompVList<T>::push_front(const T& elem)
{
    return push(elem, false);
}

template<class T>
COMPV_ERROR_CODE CompVList<T>::free()
{
    CompVMem::free(&m_pMem);
    m_pTail = m_pHead = NULL;
    m_nItems = m_nCapacity = 0;
    return COMPV_ERROR_CODE_S_OK;
}

// Reset all items without freeing the allocated memory
// Usefull if you want to re-use the list
template<class T>
COMPV_ERROR_CODE CompVList<T>::reset()
{
    m_pTail = m_pHead = NULL;
    m_nItems = 0;
    return COMPV_ERROR_CODE_S_OK;
}

template<class T>
const T* CompVList<T>::front()
{
    return m_pHead
           ? (const T*)COMPVLIST_PITEM_PDATA(m_pHead)
           : NULL;
}

template<class T>
const T* CompVList<T>::next(const T* curr)
{
    if (curr) {
        size_t nextIndex = COMPVLIST_PDATA_NEXT_INDEX(curr);
        if (nextIndex != kCompVListIndexInvalid) {
            return (const T*)COMPVLIST_PDATA(nextIndex);
        }
    }
    return NULL;
}

template<class T>
const T* CompVList<T>::back()
{
    return m_pTail
           ? (const T*)COMPVLIST_PITEM_PDATA(m_pTail)
           : NULL;
}

template<class T>
COMPV_ERROR_CODE CompVList<T>::newObj(CompVPtr<CompVList<T>*>* list, bool bLockable /*= false*/)
{
    COMPV_CHECK_EXP_RETURN(!list, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVPtr<CompVList<T>* > list_;

    list_ = new CompVList<T>();
    COMPV_CHECK_EXP_RETURN(!list_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    if (bLockable) {
        COMPV_CHECK_CODE_RETURN(CompVMutex::newObj(&list_->m_Mutex));
    }

    *list = list_;

    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
