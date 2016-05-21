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
#if !defined(_COMPV_ARRAY_H_)
#define _COMPV_ARRAY_H_

#include "compv/compv_config.h"
#include "compv/compv_buffer.h"

#include "compv/parallel/compv_threaddisp.h"

COMPV_NAMESPACE_BEGIN()

template<class T>
class COMPV_API CompVArray : public CompVObj
{
protected:
    CompVArray();
public:
    virtual ~CompVArray();
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVArray";
    };
    COMPV_ERROR_CODE alloc(size_t rows, size_t cols, size_t alignv = 1);
    COMPV_INLINE const T* ptr(size_t row = 0, size_t col = 0)const {
        return (row > m_nRows || col > m_nCols) ? NULL : (const T*)(((const uint8_t*)m_pDataPtr) + (row * m_nStrideInBytes) + (col * m_nElmtInBytes));
    }
    COMPV_INLINE size_t rows()const {
        return m_nRows;    // In samples
    }
    COMPV_INLINE size_t cols()const {
        return m_nCols;    // In samples
    }
    COMPV_INLINE size_t elmtInBytes() {
        return m_nElmtInBytes;
    }
    COMPV_INLINE size_t rowInBytes()const {
        return m_nCols * m_nElmtInBytes;    // in bytes
    }
    COMPV_INLINE size_t strideInBytes()const {
        return m_nStrideInBytes;    // in bytes
    }
    COMPV_INLINE size_t alignV()const {
        return m_nAlignV;
    }
    COMPV_INLINE bool isEmpty()const {
        return !m_nCols || !m_nRows;
    };
    static COMPV_ERROR_CODE newObj(CompVPtr<CompVArray<T>* >* array, size_t rows, size_t cols, size_t alignv = 1);

private:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    T* m_pDataPtr;
    size_t m_nCols;
    size_t m_nRows;
    size_t m_nStrideInBytes;
    size_t m_nElmtInBytes;
    size_t m_nAlignV;
    size_t m_nDataSize;
    size_t m_nDataCapacity;
    COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_ARRAY_H_ */
