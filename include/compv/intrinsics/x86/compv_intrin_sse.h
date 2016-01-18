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
#if !defined(_COMPV_INTRIN_SSE_H_)
#define _COMPV_INTRIN_SSE_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

/*
Macro used to convert 3x16RGB to 4x16RGBA samples
*/
#define COMPV_3RGB_TO_4RGBA_SSSE3(rgbaPtr_, rgbPtr_, xmm0_, xmm1_, xmmMaskRgbToRgba_) \
	/* RGBA0 = Convert(RGB0) -> 4RGBAs which means we used 4RGBs = 12bytes and lost 4bytes from RGB0 */ \
	/* RGBA1 = Convert(ALIGN(RGB0, RGB1, 12)) -> we used 4bytes from RGB0 and 8bytes from RGB1 = 12bytes RGB = 16bytes RGBA -> lost 12bytes from RGB1 */ \
	/* RGBA2 = Convert(ALIGN(RGB1, RGB2, 8)) -> we used 8bytes from RGB1 and 4bytes from RGB2 = 12bytes RGB = 16bytes RGBA -> lost 12bytes from RGB2 */ \
	/* RGBA3 = Convert(ALIGN(RGB2, RGB2, 4)) -> used 12bytes from RGB2 = 12bytes RGB = 16bytes RGBA */ \
	_mm_store_si128(&xmm0_, _mm_load_si128((__m128i*)rgbPtr_)); /* load first 16 samples */ \
	_mm_store_si128(&xmm1_, _mm_load_si128((__m128i*)(rgbPtr_ + 16))); /* load next 16 samples */ \
	_mm_store_si128(&((__m128i*)rgbaPtr_)[0], _mm_shuffle_epi8(xmm0_, xmmMaskRgbToRgba_)); \
	_mm_store_si128(&((__m128i*)rgbaPtr_)[1], _mm_shuffle_epi8(_mm_alignr_epi8(xmm1_, xmm0_, 12), xmmMaskRgbToRgba_)); \
	_mm_store_si128(&xmm0_, _mm_load_si128((__m128i*)(rgbPtr_ + 32))); /* load next 16 samples */ \
	_mm_store_si128(&((__m128i*)rgbaPtr_)[2], _mm_shuffle_epi8(_mm_alignr_epi8(xmm0_, xmm1_, 8), xmmMaskRgbToRgba_)); \
	_mm_store_si128(&((__m128i*)rgbaPtr_)[3], _mm_shuffle_epi8(_mm_alignr_epi8(xmm0_, xmm0_, 4), xmmMaskRgbToRgba_)); \

COMPV_NAMESPACE_END()

#endif /* _COMPV_INTRIN_SSE_H_ */