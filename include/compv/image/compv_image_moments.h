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
#if !defined(_COMPV_IMAGE_IMAGE_MOMENTS_H_)
#define _COMPV_IMAGE_IMAGE_MOMENTS_H_

#include "compv/compv_config.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

class CompVImageMoments
{

public:
    static double cirPQ(const uint8_t* ptr, int patch_diameter, int center_x, int center_y, int img_width, int img_stride, int img_height, int p, int q);
	static void cirM01M10(const uint8_t* ptr, int patch_diameter, const int* patch_max_abscissas, int center_x, int center_y, int img_width, int img_stride, int img_height, double* m01, double* m10);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_IMAGE_IMAGE_MOMENTS_H_ */
