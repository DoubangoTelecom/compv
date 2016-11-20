/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_DRAWING_CANVAS_H_)
#define _COMPV_DRAWING_CANVAS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_mat.h"

#include <string>

COMPV_NAMESPACE_BEGIN()

//
//	CompVCanvasImpl
//
class CompVCanvasImpl;
typedef CompVPtr<CompVCanvasImpl* > CompVCanvasImplPtr;
typedef CompVCanvasImplPtr* CompVCanvasImplPtrPtr;

class CompVCanvasImpl : public CompVObj
{
protected:
	CompVCanvasImpl();
public:
	virtual ~CompVCanvasImpl();

	virtual COMPV_ERROR_CODE drawText(const void* textPtr, size_t textLengthInBytes, int x, int y) = 0;
	virtual COMPV_ERROR_CODE drawLine(int x0, int y0, int x1, int y1) = 0;

	static COMPV_ERROR_CODE newObj(CompVCanvasImplPtrPtr canvasImpl);

protected:

private:
};

//
//	CompVCanvas
//

class COMPV_DRAWING_API CompVCanvas
{
protected:
	CompVCanvas();
public:
	virtual ~CompVCanvas();

	COMPV_ERROR_CODE drawText(const void* textPtr, size_t textLengthInBytes, int x, int y);
	COMPV_ERROR_CODE drawLine(int x0, int y0, int x1, int y1);
	
protected:
	// FIXME: ugly
	virtual COMPV_ERROR_CODE canvasBind() = 0;
	virtual COMPV_ERROR_CODE canvasUnbind() = 0;

private:
	COMPV_ERROR_CODE init();
	COMPV_ERROR_CODE deInit();

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	CompVCanvasImplPtr m_ptrImpl;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_DRAWING_CANVAS_H_ */
