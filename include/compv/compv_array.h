/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
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
	COMPV_ERROR_CODE zero_all();
	COMPV_ERROR_CODE zero_rows();
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
	static COMPV_ERROR_CODE wrap(CompVPtr<CompVArray<T>* >& array, const T* mem, size_t rows, size_t cols, size_t arrayAlign = COMPV_SIMD_ALIGNV_DEFAULT, size_t memAlign = 1);
	static COMPV_ERROR_CODE unwrap(T* mem, const CompVPtr<CompVArray<T>* >& array, size_t memAlign = 1);
    static COMPV_ERROR_CODE newObj(CompVPtr<CompVArray<T>* >* array, size_t rows, size_t cols, size_t alignv);
	static COMPV_ERROR_CODE newObjStrideless(CompVPtr<CompVArray<T>* >* array, size_t rows, size_t cols);
	static COMPV_ERROR_CODE newObjAligned(CompVPtr<CompVArray<T>* >* array, size_t rows, size_t cols);

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
