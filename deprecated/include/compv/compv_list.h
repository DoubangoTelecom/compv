/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_LIST_H_)
#define _COMPV_LIST_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"
#include "compv/parallel/compv_mutex.h"

COMPV_NAMESPACE_BEGIN()

template<class T>
class COMPV_API CompVList : public CompVObj
{
protected:
    CompVList();
public:
    virtual ~CompVList();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVList";
    };

    COMPV_ERROR_CODE lock();
    COMPV_ERROR_CODE unlock();

    COMPV_ERROR_CODE push(const T& elem, bool bBack = true);
    COMPV_ERROR_CODE push_back(const T& elem);
    COMPV_ERROR_CODE push_front(const T& elem);
    COMPV_ERROR_CODE free();
    COMPV_ERROR_CODE reset();


    const T* front();
    const T* next(const T* curr);
    const T* back();

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVList<T >* >* list, bool bLockable = false);

protected:
    void* nextFreeMemBlock(size_t** index, size_t** next_index, T** data);

private:
    void* m_pMem;
    const void* m_pHead;
    const void* m_pTail;
    size_t m_nDataSize;
    size_t m_nDataStride;
    size_t m_nItemSize;
    size_t m_nItems;
    size_t m_nCapacity;
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    CompVPtr<CompVMutex*> m_Mutex;
    COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_LIST_H_ */
