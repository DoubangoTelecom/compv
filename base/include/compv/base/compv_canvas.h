/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_CANVAS_H_)
#define _COMPV_BASE_CANVAS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"

#include <string>

COMPV_NAMESPACE_BEGIN()

//
//	CompVCanvasInterface
//
class COMPV_BASE_API CompVCanvasInterface
{
public:
	virtual COMPV_ERROR_CODE drawText(const void* textPtr, size_t textLengthInBytes, int x, int y) = 0;
	virtual COMPV_ERROR_CODE drawLine(int x0, int y0, int x1, int y1) = 0;
};

//
//	CompVCanvasImpl
//
class CompVCanvasImpl;
typedef CompVPtr<CompVCanvasImpl* > CompVCanvasImplPtr;
typedef CompVCanvasImplPtr* CompVCanvasImplPtrPtr;

class COMPV_BASE_API CompVCanvasImpl : public CompVObj, public CompVCanvasInterface
{
protected:
	CompVCanvasImpl();
public:
	virtual ~CompVCanvasImpl();
};

//
//	CompVCanvas
//
class CompVCanvas;
typedef CompVPtr<CompVCanvas* > CompVCanvasPtr;
typedef CompVCanvasPtr* CompVCanvasPtrPtr;

class COMPV_BASE_API CompVCanvas : public CompVObj, public CompVCanvasInterface
{
protected:
	CompVCanvas();
public:
	virtual ~CompVCanvas();
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_CANVAS_H_ */
