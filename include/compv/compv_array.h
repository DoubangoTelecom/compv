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

#define COMPV_ARRAY_DIM_SIZES ... // size_t values
#define COMPV_ARRAY_DIM_SIZE1(s1) ((size_t)s1), ((size_t)0)
#define COMPV_ARRAY_DIM_SIZE2(s1, s2) ((size_t)s1), ((size_t)s2), ((size_t)0)
#define COMPV_ARRAY_DIM_SIZE3(s1, s2, s3) ((size_t)s1), ((size_t)s2), ((size_t)s3), ((size_t)0)

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
    virtual COMPV_INLINE size_t getDimCount() {
        return m_nDimCount;
    }
    const T* getDataPtr() const;
    const T* getDataPtr1(size_t nIdx0) const;
    const T* getDataPtr2(size_t nIdx0, size_t nIdx1) const;
    size_t getDataSizeInBytes() const;
    size_t getDataSizeInBytes1(size_t nDimIdx) const;
    static COMPV_ERROR_CODE newObj(CompVPtr<CompVArray<T>* >* array, size_t nDimCount, COMPV_ARRAY_DIM_SIZES);

private:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    CompVPtr<CompVBuffer*> m_pBuffer;
    size_t* m_pnDimSizes;
    size_t m_nDimCount;
    COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_ARRAY_H_ */
