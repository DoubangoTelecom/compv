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
#include "compv/image/scale/compv_imagescale_pyramid.h"
#include "compv/image/scale/compv_imagescale_common.h"
#include "compv/compv_engine.h"
#include "compv/compv_mem.h"

COMPV_NAMESPACE_BEGIN()

CompVImageScalePyramid::CompVImageScalePyramid(float fScaleFactor, int32_t nLevels, COMPV_SCALE_TYPE eScaleType /*= COMPV_SCALE_TYPE_BILINEAR*/)
: m_bValid(false)
, m_fScaleFactor(fScaleFactor)
, m_nLevels(nLevels)
, m_pImages(NULL)
{
	COMPV_ASSERT(m_nLevels > 0 && m_fScaleFactor > 0); // never happen, we already checked it in newObj()
	m_pImages = (CompVObjWrapper<CompVImage *>*)CompVMem::calloc(m_nLevels, sizeof(CompVObjWrapper<CompVImage *>));
	if (!m_pImages) {
		COMPV_DEBUG_ERROR("Failed to allocate the images");
		return;
	}
	m_bValid = true;
}

CompVImageScalePyramid::~CompVImageScalePyramid()
{
	if (m_pImages) {
		for (int32_t i = 0; i < m_nLevels; ++i) {
			m_pImages[i] = NULL;
		}
		CompVMem::free((void**)&m_pImages);
	}
}

COMPV_ERROR_CODE CompVImageScalePyramid::process(const CompVObjWrapper<CompVImage*>& inImage)
{
	COMPV_CHECK_EXP_RETURN(!inImage, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	int32_t outWidth, outHeight;

	// level0 -> no change
	outWidth = inImage->getWidth();
	outHeight = inImage->getHeight();
	COMPV_CHECK_CODE_RETURN(inImage->scale(m_eScaleType, outWidth, outHeight, &m_pImages[0]));

	for (int32_t level = 1; level < m_nLevels; ++level) {
		outWidth = (int32_t)(outWidth * m_fScaleFactor + 1.f);
		outHeight = (int32_t)(outHeight * m_fScaleFactor + 1.f);
		COMPV_CHECK_CODE_RETURN(inImage->scale(m_eScaleType, outWidth, outHeight, &m_pImages[level]));
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageScalePyramid::getImage(int32_t level, CompVObjWrapper<CompVImage *>* image)
{
	COMPV_CHECK_EXP_RETURN(image == NULL || level >= m_nLevels, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	*image = m_pImages[level];
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVImageScalePyramid::newObj(float fScaleFactor, int32_t nLevels, COMPV_SCALE_TYPE eScaleType, CompVObjWrapper<CompVImageScalePyramid*>* pyramid)
{
	COMPV_CHECK_CODE_RETURN(CompVEngine::init());
	COMPV_CHECK_EXP_RETURN(pyramid == NULL || nLevels <= 0 || fScaleFactor <= 0, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVObjWrapper<CompVImageScalePyramid*>pyramid_ = new CompVImageScalePyramid(fScaleFactor, nLevels, eScaleType);
	if (!pyramid_ || !pyramid_->m_bValid) {
		COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	}
	*pyramid = pyramid_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
