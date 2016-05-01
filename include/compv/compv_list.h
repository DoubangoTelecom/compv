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
