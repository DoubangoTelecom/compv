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

template<class T>
CompVArray<T>::CompVArray()
: m_pBuffer(NULL)
, m_pnDimSizes(NULL)
{

}

template<class T>
CompVArray<T>::~CompVArray()
{
	m_pBuffer = NULL;
	CompVMem::free((void**)&m_pnDimSizes);
}

template<class T>
const T* CompVArray<T>::getDataPtr() const
{
	return ((const T*)m_pBuffer->getPtr());
}

template<class T>
const T* CompVArray<T>::getDataPtr1(size_t nIdx0) const
{
	if (nIdx0 < m_pnDimSizes[0]) {
		return ((const T*)m_pBuffer->getPtr()) + nIdx0;
	}
	return NULL;
}

template<class T>
const T* CompVArray<T>::getDataPtr2(size_t nIdx0, size_t nIdx1) const
{
	if (nIdx0 < m_pnDimSizes[0] && nIdx1 < m_pnDimSizes[1]) {
		return ((const T*)m_pBuffer->getPtr()) + ((nIdx0 * m_pnDimSizes[0]) + nIdx1);
	}
	return NULL;
}

template<class T>
size_t CompVArray<T>::getDataSizeInBytes() const
{
	return (size_t)m_pBuffer->getSize();
}

template<class T>
size_t CompVArray<T>::getDataSizeInBytes1(size_t nDimIdx) const
{
	if (nDimIdx < m_nDimCount) {
		return m_pnDimSizes[nDimIdx] * sizeof(T);
	}
	return 0;
}

template<class T>
COMPV_ERROR_CODE CompVArray<T>::newObj(CompVObjWrapper<CompVArray<T>* >* array, size_t nDimCount, COMPV_ARRAY_DIM_SIZES)
{
	COMPV_CHECK_EXP_RETURN(!array || !nDimCount, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVObjWrapper<CompVBuffer*> buffer_ = NULL;
	size_t* pnDimSizes_ = NULL;
	CompVObjWrapper<CompVArray<T>* > array_;
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;
	size_t sizeInElmt_, totalSizeInElmt_ = 1;
	static size_t sizeOfElmt_ = sizeof(T);

	va_list ap;
	va_start(ap, nDimCount);

	pnDimSizes_ = (size_t*)CompVMem::calloc(nDimCount, sizeof(size_t));
	if (!pnDimSizes_) {
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}

	for (size_t i = 0; i < nDimCount; ++i) {
		if ((sizeInElmt_ = va_arg(ap, size_t)) == 0) {
			COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		}
		totalSizeInElmt_ *= sizeInElmt_;
		pnDimSizes_[i] = sizeInElmt_;
	}

	COMPV_CHECK_CODE_BAIL(err_ = CompVBuffer::newObj(NULL, (int32_t)(totalSizeInElmt_ * sizeOfElmt_), &buffer_));

	array_ = new CompVArray<T>();
	if (!array_) {
		COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}
	array_->m_pBuffer = buffer_;
	array_->m_pnDimSizes = pnDimSizes_;
	array_->m_nDimCount = nDimCount;

	*array = array_;

bail:
	va_end(ap);
	if (COMPV_ERROR_CODE_IS_NOK(err_)) {
		CompVMem::free((void**)&pnDimSizes_);
	}

	return err_;
}

// See header file
template class CompVArray<double >;
template class CompVArray<float >;
template class CompVArray<uint16_t >;
template class CompVArray<uint8_t >;

COMPV_NAMESPACE_END()
