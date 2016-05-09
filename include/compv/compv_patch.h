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
#if !defined(_COMPV_PATCH_H_)
#define _COMPV_PATCH_H_

#include "compv/compv_config.h"
#include "compv/compv_obj.h"
#include "compv/compv_common.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_API CompVPatch : public CompVObj
{
protected:
	CompVPatch();
public:
	virtual ~CompVPatch();
	virtual COMPV_INLINE const char* getObjectId() {
		return "CompVPatch";
	};
	void moments0110(const uint8_t* ptr, int center_x, int center_y, int img_width, int img_stride, int img_height, int* m01, int* m10);
	static COMPV_ERROR_CODE newObj(CompVPtr<CompVPatch* >* patch, int diameter);

private:
	void initXYMax();

private:
	int m_nRadius;
	int16_t* m_pMaxAbscissas;
	int16_t* m_pX;
	int16_t* m_pY;
	uint8_t* m_pTop;
	uint8_t* m_pBottom;
	size_t m_nCount;
	size_t m_nStride;
	void(*m_Moments0110)(const uint8_t* top, const uint8_t* bottom, const int16_t* x, const int16_t* y, compv_scalar_t count, compv_scalar_t* s01, compv_scalar_t* s10);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_PATCH_H_ */
