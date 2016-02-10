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
#if !defined(_COMPV_FEATURES_FAST_DETE_INTRIN_AVX2_H_)
#define _COMPV_FEATURES_FAST_DETE_INTRIN_AVX2_H_

#include "compv/compv_config.h"

#if defined(COMPV_ARCH_X86) && defined(COMPV_INTRINSIC)
#include "compv/compv_common.h"
#include "compv/image/compv_image.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

void FastData16Row_Intrin_AVX2(
	const uint8_t* IP,
	const uint8_t* IPprev,
	compv_scalar_t width,
	const compv_scalar_t(&pixels16)[16],
	compv_scalar_t N,
	compv_scalar_t threshold,
	COMPV_ALIGNED(AVX) compv_scalar_t(*pfdarkers16)[16],
	COMPV_ALIGNED(AVX) compv_scalar_t(*pfbrighters16)[16],
	COMPV_ALIGNED(AVX) uint8_t* ddarkers16x32,
	COMPV_ALIGNED(AVX) uint8_t* dbrighters16x32,
	compv_scalar_t* rd,
	compv_scalar_t* rb,
	compv_scalar_t* me);

COMPV_NAMESPACE_END()

#endif /* COMPV_ARCH_X86 */

#endif /* _COMPV_FEATURES_FAST_DETE_INTRIN_AVX2_H_ */
