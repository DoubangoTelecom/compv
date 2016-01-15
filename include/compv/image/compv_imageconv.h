///* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
//*
//* This file is part of Open Source ComputerVision (a.k.a CompV) project.
//* Source code hosted at https://github.com/DoubangoTelecom/compv
//* Website hosted at http://compv.org
//*
//* CompV is free software: you can redistribute it and/or modify
//* it under the terms of the GNU General Public License as published by
//* the Free Software Foundation, either version 3 of the License, or
//* (at your option) any later version.
//*
//* CompV is distributed in the hope that it will be useful,
//* but WITHOUT ANY WARRANTY; without even the implied warranty of
//* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//* GNU General Public License for more details.
//*
//* You should have received a copy of the GNU General Public License
//* along with CompV.
//*/
//#if !defined(_COMPV_IMAGE_IMAGECONV_H_)
//#define _COMPV_IMAGE_IMAGECONV_H_
//
//#include "compv/compv_config.h"
//#include "compv/compv_obj.h"
//#include "compv/compv_buffer.h"
//#include "compv/compv_common.h"
//#include "compv/image/compv_image.h"
//#include "compv/parallel/compv_threaddisp.h"
//
//
//COMPV_NAMESPACE_BEGIN()
//
//class COMPV_API CompVImageConv
//{
//protected:
//	CompVImageConv();
//public:
//	virtual ~CompVImageConv();
//
//	static COMPV_ERROR_CODE rgbaToI420(const uint8_t* rgbaPtr, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* i420);
//	static COMPV_ERROR_CODE argbToI420(const uint8_t* argbPtr, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* i420);
//	static COMPV_ERROR_CODE i420ToRGBA(const uint8_t* yPtr, const uint8_t* uPtr, const uint8_t* vPtr, int32_t height, int32_t width, int32_t stride, CompVObjWrapper<CompVImage* >* rgba);
//
//private:
//	COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
//	COMPV_DISABLE_WARNINGS_END()
//};
//
//COMPV_NAMESPACE_END()
//
//#endif /* _COMPV_IMAGE_IMAGECONV_H_ */
