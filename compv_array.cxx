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
#include "compv/compv_array.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

template class CompVArray<CompVDMatch >;
template class CompVArray<int32_t >;
template class CompVArray<double >;
template class CompVArray<float >;
template class CompVArray<uint16_t >;
template class CompVArray<int16_t >;
template class CompVArray<uint8_t >;

template<class T>
CompVArray<T>::CompVArray()
	: m_pDataPtr(NULL)
	, m_nDataSize(0)
	, m_nDataCapacity(0)
	, m_nCols(0)
	, m_nRows(0)
	, m_nElmtInBytes(0)
	, m_nStrideInBytes(0)
	, m_nAlignV(1)
{

}

template<class T>
CompVArray<T>::~CompVArray()
{
	CompVMem::free((void**)&m_pDataPtr);
}

template<class T>
COMPV_ERROR_CODE CompVArray<T>::alloc(size_t rows, size_t cols, size_t alignv /*= 1*/)
{
	COMPV_CHECK_EXP_RETURN(!rows || !cols || !alignv, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	size_t nElmtInBytes_ = sizeof(T);
	size_t strideInBytes_ = CompVMem::alignForward((cols * nElmtInBytes_), (int)alignv);
	size_t newDataSize_ = strideInBytes_ * rows;
	void* pMem_ = NULL;

	if (newDataSize_ > m_nDataCapacity) {
		pMem_ = CompVMem::malloc(newDataSize_);
		COMPV_CHECK_EXP_BAIL(!pMem_, (err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
		m_nDataCapacity = newDataSize_;

		// realloc()
		CompVMem::free((void**)&m_pDataPtr);
		m_pDataPtr = (T*)pMem_;
	}
	
	// Do not update capacity
	m_nDataSize = newDataSize_;
	m_nCols = cols;
	m_nRows = rows;
	m_nElmtInBytes = nElmtInBytes_;
	m_nStrideInBytes = strideInBytes_;
	m_nAlignV = alignv;

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err_)) {
		CompVMem::free((void**)&pMem_);
	}
	return err_;
}

// alignv must be 1 for backward compatibility and to create compact array by default
template<class T>
COMPV_ERROR_CODE CompVArray<T>::newObj(CompVPtr<CompVArray<T>* >* array, size_t rows, size_t cols, size_t alignv /*= 1*/)
{
	COMPV_CHECK_EXP_RETURN(!array || !rows || !cols || !alignv, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	if (!*array) {
		*array = new CompVArray<T>();
		COMPV_CHECK_EXP_RETURN(!*array, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}
	COMPV_CHECK_CODE_RETURN((*array)->alloc(rows, cols, alignv));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
