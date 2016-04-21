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
#include "compv/compv_convlt.h"
#include "compv/compv_mem.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

CompVConvlt::CompVConvlt()
: m_pDataPtr(NULL)
, m_pDataPtr0(NULL)
, m_nDataSize(0)
, m_nDataSize0(0)
{

}

CompVConvlt::~CompVConvlt()
{
	CompVMem::free((void**)&m_pDataPtr);
	CompVMem::free((void**)&m_pDataPtr0);
}

COMPV_ERROR_CODE CompVConvlt::convlt2(const uint8_t* img_ptr, int img_width, int img_stride, int img_height, const double* kern_ptr, int kern_size, const void** ret_ptr /*= NULL*/, int img_border /*= 0*/)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // TODO(dmi): Multi-threading and SIMD acceleration

	// Check inputs
	COMPV_CHECK_EXP_RETURN(!img_ptr || !img_width || (img_stride < img_width) || !kern_ptr || !(kern_size & 1) || img_border < 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Alloc memory
	size_t neededSize = (img_height + (img_border << 1)) * (img_stride + (img_border << 1));
	if (m_nDataSize < neededSize) {
		m_pDataPtr = CompVMem::realloc(m_pDataPtr, neededSize);
		if (!m_pDataPtr) {
			m_nDataSize = 0;
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		}
		m_nDataSize = neededSize;
	}

	// Init variables
	uint8_t* outImg = (uint8_t*)m_pDataPtr;
	const uint8_t *topleft, *img_ptr_;
	double sum;
	const double *ker_ptr;
	int imgpad, i, j, row, col;
	int ker_size_div2 = kern_size >> 1;
	int start_idx = (img_border > ker_size_div2) ? ker_size_div2 : (ker_size_div2 - img_border);
	img_ptr_ = img_ptr; // FIXME: not correct -> use start_idx
	imgpad = (img_stride - img_width) + ker_size_div2 + ker_size_div2;

	// Process
	for (j = start_idx; j < img_height - start_idx; ++j) {
		for (i = start_idx; i < img_width - start_idx; ++i) {
			sum = 0;
			topleft = img_ptr_;
			ker_ptr = kern_ptr;
			for (row = 0; row < kern_size; ++row) {
				for (col = 0; col < kern_size; ++col) {
					sum += topleft[col] * ker_ptr[col];
				}
				ker_ptr += kern_size;
				topleft += img_stride;
			}
			outImg[(j * img_stride) + i] = (uint8_t)sum;  // TODO(dmi): do not mul() but add()
			++img_ptr_;
		}
		img_ptr_ += imgpad;
	}

	if (ret_ptr) {
		*ret_ptr = m_pDataPtr;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConvlt::convlt1(const uint8_t* img_ptr, int img_width, int img_stride, int img_height, const double* vkern_ptr, const double* hkern_ptr, int kern_size, const void** ret_ptr /*= NULL*/, int img_border /*= 0*/)
{
	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED(); // TODO(dmi): Multi-threading and SIMD acceleration

	// Check inputs
	COMPV_CHECK_EXP_RETURN(!img_ptr || !img_width || (img_stride < img_width) || !vkern_ptr || !hkern_ptr || !(kern_size & 1) || img_border < 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	// Alloc memory
	size_t neededSize = (img_height + (img_border << 1)) * (img_stride + (img_border << 1));
	if (m_nDataSize < neededSize) {
		m_pDataPtr = CompVMem::realloc(m_pDataPtr, neededSize);
		if (!m_pDataPtr) {
			m_nDataSize = 0;
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		}
		m_nDataSize = neededSize;
	}
	if (m_nDataSize0 < neededSize) {
		m_pDataPtr0 = CompVMem::realloc(m_pDataPtr0, neededSize);
		if (!m_pDataPtr0) {
			m_nDataSize0 = 0;
			COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		}
		m_nDataSize0 = neededSize;
	}

	uint8_t *imgTmp0, *imgTmp1;
	const uint8_t *topleft, *ptr_;
	double sum;
	int imgpad, i, j, row, col;
	int ker_size_div2 = kern_size >> 1;
	int start_idx = (img_border > ker_size_div2) ? ker_size_div2 : (ker_size_div2 - img_border);

	imgTmp0 = (uint8_t*)m_pDataPtr0;
	imgTmp1 = (uint8_t*)m_pDataPtr;

	// Process

	// Horizontal
	topleft = img_ptr; // FIXME: not correct -> use start_idx
	imgpad = (img_stride - img_width) + start_idx + start_idx;
	for (j = 0; j < img_height; ++j) {
		for (i = start_idx; i < img_width - start_idx; ++i) {
			sum = 0;
			for (col = 0; col < kern_size; ++col) {
				sum += topleft[col] * hkern_ptr[col];
			}
			imgTmp0[(j * img_stride) + i] = (uint8_t)sum; // TODO(dmi): do not mul() but add()
			++topleft;
		}
		topleft += imgpad;
	}

	// Vertical
	// FIXME: not correct -> use start_idx
	topleft = imgTmp0; // output from hz filtering is now used as input
	imgpad = (img_stride - img_width);
	for (j = start_idx; j < img_height - start_idx; ++j) {
		for (i = 0; i < img_width; ++i) {
			sum = 0;
			ptr_ = topleft;
			for (row = 0; row < kern_size; ++row, ptr_ += img_stride) {
				sum += *ptr_ * vkern_ptr[row];
			}
			imgTmp1[(j * img_stride) + i] = (uint8_t)sum; // TODO(dmi): do not mul() but add()
			++topleft;
		}
		topleft += imgpad;
	}

	if (ret_ptr) {
		*ret_ptr = m_pDataPtr;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConvlt::newObj(CompVObjWrapper<CompVConvlt* >* convlt)
{
	COMPV_CHECK_EXP_RETURN(!convlt, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVObjWrapper<CompVConvlt*> convlt_ = NULL;

	convlt_ = new CompVConvlt();
	COMPV_CHECK_EXP_RETURN(!convlt, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*convlt = convlt_;

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
