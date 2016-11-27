/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_FACTORY_H_)
#define _COMPV_DRAWING_FACTORY_H_

#include "compv/drawing/compv_config.h"
#include "compv/drawing/compv_common.h"
#include "compv/base/drawing/compv_canvas.h"
#include "compv/base/drawing/compv_mvp.h"
#include "compv/base/drawing/compv_window.h"

COMPV_NAMESPACE_BEGIN()

//
//	CompVDrawingMVP
//
class COMPV_DRAWING_API CompVDrawingMVP
{
public:
	static COMPV_ERROR_CODE newObj2D(CompVMVPPtrPtr mvp);
	static COMPV_ERROR_CODE newObj3D(CompVMVPPtrPtr mvp);
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_FACTORY_H_ */
