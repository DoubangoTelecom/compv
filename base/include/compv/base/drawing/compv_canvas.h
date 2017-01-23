/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_DRAWING_CANVAS_H_)
#define _COMPV_BASE_DRAWING_CANVAS_H_

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
	virtual COMPV_ERROR_CODE drawInterestPoints(const std::vector<CompVInterestPoint >& interestPoints) = 0;
};

//
//	CompVCanvasImpl
//
COMPV_OBJECT_DECLARE_PTRS(CanvasImpl)

class COMPV_BASE_API CompVCanvasImpl : public CompVObj, public CompVCanvasInterface
{
protected:
    CompVCanvasImpl();
public:
    virtual ~CompVCanvasImpl();
	virtual COMPV_ERROR_CODE close() = 0;
};

//
//	CompVCanvas
//
COMPV_OBJECT_DECLARE_PTRS(Canvas)

class COMPV_BASE_API CompVCanvas : public CompVObj, public CompVCanvasInterface
{
protected:
    CompVCanvas();
public:
    virtual ~CompVCanvas();
};

//
//	CompVCanvasFactory
//
struct COMPV_BASE_API CompVCanvasFactory {
public:
    const char* name;
    COMPV_ERROR_CODE(*newObjFuncPtr)(CompVCanvasImplPtrPtr canvasImpl);
    static COMPV_ERROR_CODE set(const CompVCanvasFactory* inst) {
        COMPV_CHECK_EXP_RETURN(!inst, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        COMPV_DEBUG_INFO("Setting canvas factory: %s", inst->name);
        instance = inst;
        return COMPV_ERROR_CODE_S_OK;
    }
    static COMPV_ERROR_CODE newObj(CompVCanvasImplPtrPtr canvasImpl) {
        COMPV_CHECK_EXP_RETURN(!canvasImpl, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        COMPV_CHECK_EXP_RETURN(!instance, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
        COMPV_CHECK_CODE_RETURN(instance->newObjFuncPtr(canvasImpl));
        return COMPV_ERROR_CODE_S_OK;
    }
private:
    static const CompVCanvasFactory* instance;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_DRAWING_CANVAS_H_ */
