/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
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
//	CompVCanvas
//
COMPV_OBJECT_DECLARE_PTRS(Canvas)

class COMPV_BASE_API CompVCanvas : public CompVObj
{
protected:
    CompVCanvas();
public:
    virtual ~CompVCanvas();

	virtual COMPV_ERROR_CODE clear(const CompVDrawingOptions* options = nullptr) = 0;
	virtual COMPV_ERROR_CODE drawTexts(const CompVVecString& texts, const CompVPointFloat32Vector& positions, const CompVDrawingOptions* options = nullptr) = 0;
	virtual COMPV_ERROR_CODE drawLines(const CompVLineFloat32Vector& lines, const CompVDrawingOptions* options = nullptr) = 0;
	virtual COMPV_ERROR_CODE drawLines(const CompVPointFloat32Vector& points, const CompVDrawingOptions* options = nullptr) = 0;
	virtual COMPV_ERROR_CODE drawLines(const CompVMatPtr& points, const CompVDrawingOptions* options = nullptr) = 0;
	virtual COMPV_ERROR_CODE drawRectangles(const CompVRectFloat32Vector& rects, const CompVDrawingOptions* options = nullptr) = 0; // rectangle with square angles, for arbitrary angles use drawLines
	virtual COMPV_ERROR_CODE drawPoints(const CompVPointFloat32Vector& points, const CompVDrawingOptions* options = nullptr) = 0;
	virtual COMPV_ERROR_CODE drawPoints(const CompVMatPtr& points, const CompVDrawingOptions* options = nullptr) = 0;
	virtual COMPV_ERROR_CODE drawInterestPoints(const CompVInterestPointVector& interestPoints, const CompVDrawingOptions* options = nullptr) = 0;
	virtual bool haveDrawTexts()const = 0;
};

//
//	CompVCanvasFactory
//
struct COMPV_BASE_API CompVCanvasFactory {
public:
    const char* name;
    COMPV_ERROR_CODE(*newObjFuncPtr)(CompVCanvasPtrPtr canvas);
    static COMPV_ERROR_CODE set(const CompVCanvasFactory* inst) {
        COMPV_CHECK_EXP_RETURN(!inst, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        COMPV_DEBUG_INFO("Setting canvas factory: %s", inst->name);
        instance = inst;
        return COMPV_ERROR_CODE_S_OK;
    }
    static COMPV_ERROR_CODE newObj(CompVCanvasPtrPtr canvas) {
        COMPV_CHECK_EXP_RETURN(!canvas, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
        COMPV_CHECK_EXP_RETURN(!instance, COMPV_ERROR_CODE_E_NOT_INITIALIZED);
        COMPV_CHECK_CODE_RETURN(instance->newObjFuncPtr(canvas));
        return COMPV_ERROR_CODE_S_OK;
    }
private:
    static const CompVCanvasFactory* instance;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_DRAWING_CANVAS_H_ */
