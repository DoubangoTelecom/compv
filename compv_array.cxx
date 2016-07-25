/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/compv_array.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

template class CompVArray<CompVHoughEntry >;
template class CompVArray<CompVCoordPolar2f >;
template class CompVArray<CompVDMatch >;
template class CompVArray<int32_t >;
template class CompVArray<compv_float64_t >;
template class CompVArray<compv_float32_t >;
template class CompVArray<compv_float32x2_t >;
template class CompVArray<uint16_t >;
template class CompVArray<int16_t >;
template class CompVArray<uint8_t >;
template class CompVArray<size_t >;

template<class T>
CompVArray<T>::CompVArray()
    : m_pDataPtr(NULL)
	, m_bOweMem(true)
	, m_bStrideInEltsIsIntegral(true)
    , m_nDataSize(0)
    , m_nDataCapacity(0)
    , m_nCols(0)
    , m_nRows(0)
    , m_nElmtInBytes(0)
    , m_nStrideInBytes(0)
	, m_nStrideInElts(0)
    , m_nAlignV(1)
{
	//!\ If you add new member, check shrink() to see if it needs update
}

template<class T>
CompVArray<T>::~CompVArray()
{
	if (m_bOweMem) {
		CompVMem::free((void**)&m_pDataPtr);
	}
}

template<class T>
COMPV_ERROR_CODE CompVArray<T>::alloc(size_t rows, size_t cols, size_t alignv /*= 1*/, size_t stride /*= 0*/)
{
	COMPV_CHECK_EXP_RETURN(!alignv || (stride && stride < cols), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
    size_t nElmtInBytes_ = sizeof(T);
	size_t strideInBytes_ = stride ? (stride * nElmtInBytes_) : CompVMem::alignForward((cols * nElmtInBytes_), (int)alignv);
    size_t newDataSize_ = strideInBytes_ * rows;
    void* pMem_ = NULL;
	
    if (newDataSize_ > m_nDataCapacity) {
        pMem_ = CompVMem::malloc(newDataSize_);
        COMPV_CHECK_EXP_BAIL(!pMem_, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
        m_nDataCapacity = newDataSize_;

        // realloc()
		if (m_bOweMem) {
			CompVMem::free((void**)&m_pDataPtr);
		}
        m_pDataPtr = (T*)pMem_;
		m_bOweMem = true;
    }

    // Do not update capacity
    m_nDataSize = newDataSize_;
    m_nCols = cols;
    m_nRows = rows;
    m_nElmtInBytes = nElmtInBytes_;
    m_nStrideInBytes = strideInBytes_;
	m_nStrideInElts = (m_nStrideInBytes / m_nElmtInBytes);
    m_nAlignV = alignv;
	m_bStrideInEltsIsIntegral = !(m_nStrideInBytes % m_nElmtInBytes);

bail:
    if (COMPV_ERROR_CODE_IS_NOK(err_)) {
        CompVMem::free((void**)&pMem_);
    }
    return err_;
}

// Set all (even paddings) values to zero
template<class T>
COMPV_ERROR_CODE CompVArray<T>::zero_all()
{
	void* ptr_ = (void*)ptr();
	if (ptr_ && rows() && cols()) {
		CompVMem::zero(ptr_, strideInBytes() * rows());
	}
	return COMPV_ERROR_CODE_S_OK;
}

template<class T>
COMPV_ERROR_CODE CompVArray<T>::zero_rows()
{
	if (ptr() && rows() && cols()) {
		if (rowInBytes() == strideInBytes()) {
			CompVMem::zero((void*)ptr(), strideInBytes() * rows());
		}
		else {
			size_t row_, rows_ = rows(), rowInBytes_ = rowInBytes();
			for (row_ = 0; row_ < rows_; ++row_) {
				CompVMem::zero((void*)ptr(row_), rowInBytes_);
			}
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// returned object doesn't have ownership on the internal memory and depends on its creator
template<class T>
COMPV_ERROR_CODE CompVArray<T>::shrink(CompVPtr<CompVArray<T>* >& array, size_t newRows, size_t newCols)
{
	COMPV_CHECK_EXP_RETURN(newCols > m_nCols || newRows > m_nRows, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObj(&array, 0, 0, m_nAlignV));
	array->m_nCols = newCols;
	array->m_nRows = newRows;
	array->m_nElmtInBytes = m_nElmtInBytes;
	array->m_nStrideInBytes = m_nStrideInBytes;
	array->m_nAlignV = m_nAlignV;
	array->m_bOweMem = false;
	array->m_pDataPtr = m_pDataPtr;
	return COMPV_ERROR_CODE_S_OK;
}

// Set extra bytes (between the endof the row and the endof the stride).
// This function is useful to prepare data for SIMD function designed to read beyond the row and up to the end of the stride
// alignV: SIMD-alignment function for which to prepare the data. If not defined all padding data will be set to zero (CPU consuming). 
// alignV must be pof 2
template<class T>
COMPV_ERROR_CODE CompVArray<T>::wash(int alignV /*= -1*/)
{
	if (alignV <= 0) {
		alignV = (int)m_nAlignV;
	}
	size_t rowInBytes_ = rowInBytes();
	size_t neededBytes_ = CompVMem::alignForward(rowInBytes_, alignV);
	size_t strideInBytes_ = strideInBytes();
	COMPV_CHECK_EXP_RETURN(neededBytes_ > strideInBytes_, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	size_t washBytes_ = neededBytes_ - rowInBytes_;
	if (washBytes_ == 0) {
		return COMPV_ERROR_CODE_S_OK;
	}
	uint8_t* data_ = reinterpret_cast<uint8_t*>(m_pDataPtr)+ rowInBytes_;
	size_t row_, col_;
	for (row_ = 0; row_ < rows(); ++row_) {
		for (col_ = 0; col_ <= washBytes_ - 8; col_ += 8) {
			*((uint64_t*)&data_[col_]) = 0;
		}
		for ( ; col_ < washBytes_; ++col_) {
			data_[col_] = 0;
		}
		data_ += strideInBytes_;
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Copy mem to array
template<class T>
COMPV_ERROR_CODE CompVArray<T>::copy(CompVPtr<CompVArray<T>* >& array, const T* mem, size_t rows, size_t cols, size_t arrayAlign /*= COMPV_SIMD_ALIGNV_DEFAULT*/, size_t memAlign /*= 1*/)
{
	COMPV_CHECK_EXP_RETURN(!rows || !cols || !memAlign || !arrayAlign || !mem, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN(CompVArray<T>::newObj(&array, rows, cols, arrayAlign));
	if (arrayAlign == memAlign) {
		CompVMem::copy((void*)(*array)->ptr(), mem, array->strideInBytes() * rows);
	}
	else {
		const uint8_t* src = (const uint8_t*)mem;
		uint8_t* dst = (uint8_t*)array->ptr();
		size_t rowInBytes = array->rowInBytes();
		size_t dstStrideInBytes = array->strideInBytes();
		size_t srcStrideInBytes = CompVMem::alignForward(rowInBytes, (int)memAlign);
		for (size_t j = 0; j < rows; ++j) {
			CompVMem::copy(dst, src, rowInBytes);
			dst += dstStrideInBytes;
			src += srcStrideInBytes;
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Copy array to mem
template<class T>
COMPV_ERROR_CODE CompVArray<T>::copy(T* mem, const CompVPtr<CompVArray<T>* >& array, size_t memAlign = 1)
{
	COMPV_CHECK_EXP_RETURN(!array || !array->rows() || !array->cols() || !memAlign || !mem, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (memAlign == array->alignV()) {
		CompVMem::copy(mem, array->ptr(), array->strideInBytes() * array->rows());
	}
	else {
		uint8_t* dst = (uint8_t*)mem;
		const uint8_t* src = (const uint8_t*)array->ptr();
		size_t rowInBytes = array->rowInBytes();
		size_t rows = array->rows();
		size_t srcStrideInBytes = array->strideInBytes();
		size_t dstStrideInBytes = CompVMem::alignForward(rowInBytes, (int)memAlign);
		for (size_t j = 0; j < rows; ++j) {
			CompVMem::copy(dst, src, rowInBytes);
			dst += dstStrideInBytes;
			src += srcStrideInBytes;
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

template<class T>
COMPV_ERROR_CODE CompVArray<T>::newObj(CompVPtr<CompVArray<T>* >* array, size_t rows, size_t cols, size_t alignv, size_t stride /*= 0*/)
{
    COMPV_CHECK_EXP_RETURN(!array || !alignv, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVPtr<CompVArray<T>* > array_ = *array;
	if (!array_) {
		array_ = new CompVArray<T>();
		COMPV_CHECK_EXP_RETURN(!array_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		*array = array_;
    }
	COMPV_CHECK_CODE_RETURN(array_->alloc(rows, cols, alignv, stride));
    return COMPV_ERROR_CODE_S_OK;
}

template<class T>
COMPV_ERROR_CODE CompVArray<T>::newObjStrideless(CompVPtr<CompVArray<T>* >* array, size_t rows, size_t cols)
{
	return CompVArray<T>::newObj(array, rows, cols, 1);
}

template<class T>
COMPV_ERROR_CODE CompVArray<T>::newObjAligned(CompVPtr<CompVArray<T>* >* array, size_t rows, size_t cols)
{
	return CompVArray<T>::newObj(array, rows, cols, COMPV_SIMD_ALIGNV_DEFAULT);
}

COMPV_NAMESPACE_END()
