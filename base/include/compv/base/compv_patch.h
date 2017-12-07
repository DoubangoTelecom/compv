/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_PATCH_H_)
#define _COMPV_BASE_PATCH_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_debug.h"
#include "compv/base/compv_obj.h"
#include "compv/base/parallel/compv_mutex.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(Patch);

class COMPV_BASE_API CompVPatch : public CompVObj
{
protected:
	CompVPatch();
public:
	virtual ~CompVPatch();
	COMPV_OBJECT_GET_ID(CompVPatch);
	COMPV_ERROR_CODE moments0110(const uint8_t* ptr, int center_x, int center_y, size_t img_width, size_t img_height, size_t img_stride, int* m01, int* m10);
	static COMPV_ERROR_CODE newObj(CompVPatchPtrPtr patch, int diameter);

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
	void(*m_Moments0110)(const uint8_t* top, const uint8_t* bottom, const int16_t* x, const int16_t* y, compv_uscalar_t count, compv_scalar_t* s01, compv_scalar_t* s10);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_PATCH_H_ */
