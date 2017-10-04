/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_IMAGE_REMAP_H_)
#define _COMPV_BASE_IMAGE_REMAP_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

class COMPV_BASE_API CompVImageRemap
{
public:
	static COMPV_ERROR_CODE remap(const CompVMatPtr& input, CompVMatPtrPtr output, const CompVMatPtr& map, COMPV_INTERPOLATION_TYPE interType = COMPV_INTERPOLATION_TYPE_BILINEAR, const CompVSizeSz* outputSize = nullptr, const CompVRectFloat32* mapROI = nullptr);
	
private:
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_IMAGE_REMAP_H_ */
