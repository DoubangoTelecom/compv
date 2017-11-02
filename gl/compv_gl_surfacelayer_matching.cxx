/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_surfacelayer_matching.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_func.h"
#include "compv/base/math/compv_math.h"

#define COMPV_THIS_CLASSNAME	"CompVGLMatchingSurfaceLayer"

COMPV_NAMESPACE_BEGIN()

CompVGLMatchingSurfaceLayer::CompVGLMatchingSurfaceLayer()
{

}

CompVGLMatchingSurfaceLayer::~CompVGLMatchingSurfaceLayer()
{

}

COMPV_ERROR_CODE CompVGLMatchingSurfaceLayer::drawMatches(const CompVMatPtr& trainImage, const CompVMatPtr& trainGoodMatches, const CompVMatPtr& queryImage, const CompVMatPtr& queryGoodMatches, const CompVDrawingOptions* options COMPV_DEFAULT(NULL)) /*Overrides(CompVMatchingSurfaceLayer)*/
{
	COMPV_CHECK_EXP_RETURN(!trainImage || trainImage->isEmpty() || !queryImage || queryImage->isEmpty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	size_t coverWidth = trainImage->cols() + COMPV_DRAWING_MATCHES_TRAIN_QUERY_XOFFSET + queryImage->cols();
	size_t coverHeight = COMPV_MATH_MAX(trainImage->rows(), queryImage->rows());
	CompVGLFboPtr fboCover = NULL;

	// Create/FBO used in the cover surface
	COMPV_CHECK_CODE_RETURN(m_ptrCoverSurfaceGL->blitter()->requestFBO(coverWidth, coverHeight));

	if (m_ptrTrainSurfaceGL->isActive() && m_ptrQuerySurfaceGL->isActive() && m_ptrCoverSurfaceGL->isActive()) {
		fboCover = m_ptrCoverSurfaceGL->blitter()->fbo();
		const int queryOffsetX = static_cast<int>(COMPV_DRAWING_MATCHES_TRAIN_QUERY_XOFFSET + trainImage->cols());
		const int trainTop = static_cast<int>(CompVViewport::yFromBottomLeftToTopLeft(static_cast<int>(fboCover->height()), static_cast<int>(trainImage->rows()), 0));

		// Clear the FBO (this is a slave fbo and will not be cleared in surface begin)
		if (COMPV_ERROR_CODE_IS_OK(fboCover->bind())) {
			COMPV_glClearColor(0.f, 0.f, 0.f, 1.f);
			COMPV_glClearStencil(0);
			COMPV_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
			COMPV_CHECK_CODE_NOP(fboCover->unbind());
		}

		// Draw Train points
		COMPV_CHECK_CODE_RETURN(m_ptrTrainSurfaceGL->viewport()->reset(CompViewportSizeFlags::makeStatic(), 0, trainTop, static_cast<int>(trainImage->cols()), static_cast<int>(trainImage->rows())));
		COMPV_CHECK_CODE_RETURN(m_ptrTrainSurfaceGL->drawImage(trainImage));
		COMPV_CHECK_CODE_RETURN(m_ptrTrainSurfaceGL->blitRenderer(fboCover));

		// Draw Query points
		COMPV_CHECK_CODE_RETURN(m_ptrQuerySurfaceGL->viewport()->reset(CompViewportSizeFlags::makeStatic(), queryOffsetX, 0, static_cast<int>(queryImage->cols()), static_cast<int>(queryImage->rows())));
		COMPV_CHECK_CODE_RETURN(m_ptrQuerySurfaceGL->drawImage(queryImage));
		COMPV_CHECK_CODE_RETURN(m_ptrQuerySurfaceGL->blitRenderer(fboCover));

		if (trainGoodMatches && !trainGoodMatches->isEmpty() && queryGoodMatches && !queryGoodMatches->isEmpty()) {
			COMPV_CHECK_EXP_RETURN(trainGoodMatches->cols() != trainGoodMatches->cols()
				|| trainGoodMatches->rows() != trainGoodMatches->rows()
				|| trainGoodMatches->subType() != trainGoodMatches->subType(), COMPV_ERROR_CODE_E_INVALID_PARAMETER, "Query and Train matches must have same type and size");
			switch (trainGoodMatches->subType()) {
			case COMPV_SUBTYPE_RAW_FLOAT32:
				COMPV_CHECK_CODE_RETURN(drawMatches(trainGoodMatches->ptr<compv_float32_t>(0), trainGoodMatches->ptr<compv_float32_t>(1), queryGoodMatches->ptr<compv_float32_t>(0), queryGoodMatches->ptr<compv_float32_t>(1), queryGoodMatches->cols(), static_cast<size_t>(queryOffsetX), options));
				break;
			case COMPV_SUBTYPE_RAW_FLOAT64:
				COMPV_CHECK_CODE_RETURN(drawMatches(trainGoodMatches->ptr<compv_float64_t>(0), trainGoodMatches->ptr<compv_float64_t>(1), queryGoodMatches->ptr<compv_float64_t>(0), queryGoodMatches->ptr<compv_float64_t>(1), queryGoodMatches->cols(), static_cast<size_t>(queryOffsetX), options));
				break;
			default:
				COMPV_DEBUG_ERROR_EX(COMPV_THIS_CLASSNAME, "Expected COMPV_SUBTYPE_RAW_FLOAT32 or COMPV_SUBTYPE_RAW_FLOAT64 as subtype but found %s", CompVGetSubtypeString(trainGoodMatches->subType()));
				return COMPV_ERROR_CODE_E_INVALID_SUBTYPE;
			}
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

CompVSurfacePtr CompVGLMatchingSurfaceLayer::surface() /*Overrides(CompVMatchingSurfaceLayer)*/
{
	if (m_ptrCoverSurfaceGL) {
		return *m_ptrCoverSurfaceGL;
	}
	return NULL;
}

CompVSurfacePtr CompVGLMatchingSurfaceLayer::surfaceTrain() /*Overrides(CompVMatchingSurfaceLayer)*/
{
	if (m_ptrTrainSurfaceGL) {
		return *m_ptrTrainSurfaceGL;
	}
	return NULL;
}

CompVSurfacePtr CompVGLMatchingSurfaceLayer::surfaceQuery() /*Overrides(CompVMatchingSurfaceLayer)*/
{
	if (m_ptrQuerySurfaceGL) {
		return *m_ptrQuerySurfaceGL;
	}
	return NULL;
}

COMPV_ERROR_CODE CompVGLMatchingSurfaceLayer::drawMatches(const compv_float32_t* trainX, const compv_float32_t* trainY, const compv_float32_t* queryX, const compv_float32_t* queryY, size_t count, size_t queryOffsetx, const CompVDrawingOptions* options COMPV_DEFAULT(NULL)) /* Private */
{
	COMPV_CHECK_EXP_RETURN(!trainX || !trainY || !queryX || !queryY || !count, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	compv_float32_t *queryX_ = NULL;
	CompVDrawingOptions matchOptions = CompVDrawingOptions::clone(options);
	if (queryOffsetx) {
		compv_float32_t offset = static_cast<compv_float32_t>(queryOffsetx);
		queryX_ = reinterpret_cast<compv_float32_t*>(CompVMem::malloc(sizeof(compv_float32_t) * count));
		COMPV_CHECK_EXP_BAIL(!queryX_, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
		for (size_t i = 0; i < count; ++i) {
			queryX_[i] = queryX[i] + offset;
		}
	}
	
	matchOptions.lineType = COMPV_DRAWING_LINE_TYPE_MATCH;
	COMPV_CHECK_CODE_RETURN(m_ptrCoverSurfaceGL->canvasGL()->drawLinesGL(trainX, trainY, queryX_ ? queryX_ : queryX, queryY, count, &matchOptions));
	
bail:
	CompVMem::free(reinterpret_cast<void**>(&queryX_));
	return err;
}

COMPV_ERROR_CODE CompVGLMatchingSurfaceLayer::drawMatches(const compv_float64_t* trainX, const compv_float64_t* trainY, const compv_float64_t* queryX, const compv_float64_t* queryY, size_t count, size_t queryOffsetx, const CompVDrawingOptions* options COMPV_DEFAULT(NULL)) /* Private */
{
	COMPV_CHECK_EXP_RETURN(!trainX || !trainY || !queryX || !queryY || !count, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED_GPU("No SIMD or GPU implementation found"); // Add SIMD for "Float64 -> FLoat32 convertion" in CompVMathUtils::static_cast

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	compv_float32_t *trainX_ = NULL, *trainY_ = NULL, *queryX_ = NULL, *queryY_ = NULL;
	compv_float32_t offset = static_cast<compv_float32_t>(queryOffsetx);

	trainX_ = reinterpret_cast<compv_float32_t*>(CompVMem::malloc(sizeof(compv_float32_t) * count));
	COMPV_CHECK_EXP_BAIL(!trainX_, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
	trainY_ = reinterpret_cast<compv_float32_t*>(CompVMem::malloc(sizeof(compv_float32_t) * count));
	COMPV_CHECK_EXP_BAIL(!trainY_, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
	queryX_ = reinterpret_cast<compv_float32_t*>(CompVMem::malloc(sizeof(compv_float32_t) * count));
	COMPV_CHECK_EXP_BAIL(!queryX_, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));
	queryY_ = reinterpret_cast<compv_float32_t*>(CompVMem::malloc(sizeof(compv_float32_t) * count));
	COMPV_CHECK_EXP_BAIL(!queryY_, (err = COMPV_ERROR_CODE_E_OUT_OF_MEMORY));

	for (size_t i = 0; i < count; ++i) {
		trainX_[i] = static_cast<compv_float32_t>(trainX[i]);
		trainY_[i] = static_cast<compv_float32_t>(trainY[i]);
		queryX_[i] = static_cast<compv_float32_t>(queryX[i]) + offset;
		queryY_[i] = static_cast<compv_float32_t>(queryY[i]);
	}

	COMPV_CHECK_CODE_BAIL(err = drawMatches(trainX_, trainY_, queryX_, queryY_, count, 0, options)); // set offset to zero to avoid adding again in the float32 implementation

bail:
	CompVMem::free(reinterpret_cast<void**>(&trainX_));
	CompVMem::free(reinterpret_cast<void**>(&trainY_));
	CompVMem::free(reinterpret_cast<void**>(&queryX_));
	CompVMem::free(reinterpret_cast<void**>(&queryY_));
	return err;
}

CompVSurfacePtr CompVGLMatchingSurfaceLayer::cover() /*Overries(CompVSurfaceLayer)*/
{
	return *m_ptrCoverSurfaceGL;
}
										 
COMPV_ERROR_CODE CompVGLMatchingSurfaceLayer::blit() /*Overries(CompVSurfaceLayer)*/
{
    if (m_ptrCoverSurfaceGL && m_ptrCoverSurfaceGL->isActive()) {
        CompVGLFboPtr fboCover = m_ptrCoverSurfaceGL->blitter()->fbo();
		if (fboCover) {
			COMPV_CHECK_CODE_RETURN(m_ptrCoverSurfaceGL->blit(fboCover, kCompVGLPtrSystemFrameBuffer));
		}
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMatchingSurfaceLayer::updateSize(size_t newWidth, size_t newHeight)
{
    COMPV_CHECK_EXP_RETURN(!m_ptrCoverSurfaceGL, COMPV_ERROR_CODE_E_INVALID_STATE);
    COMPV_CHECK_CODE_RETURN(m_ptrCoverSurfaceGL->updateSize(newWidth, newHeight));
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMatchingSurfaceLayer::close()
{
    if (m_ptrCoverSurfaceGL) {
        COMPV_CHECK_CODE_NOP(m_ptrCoverSurfaceGL->close());
    }
    if (m_ptrTrainSurfaceGL) {
        COMPV_CHECK_CODE_NOP(m_ptrTrainSurfaceGL->close());
    }
    if (m_ptrQuerySurfaceGL) {
        COMPV_CHECK_CODE_NOP(m_ptrQuerySurfaceGL->close());
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLMatchingSurfaceLayer::newObj(CompVGLMatchingSurfaceLayerPtrPtr layer, size_t width, size_t height)
{
    COMPV_CHECK_EXP_RETURN(!layer || !width || !height, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLMatchingSurfaceLayerPtr layer_;
    layer_ = new CompVGLMatchingSurfaceLayer();
    COMPV_CHECK_EXP_RETURN(!layer_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    COMPV_CHECK_CODE_RETURN(CompVGLSurface::newObj(&layer_->m_ptrTrainSurfaceGL, width, height));
    COMPV_CHECK_CODE_RETURN(CompVGLSurface::newObj(&layer_->m_ptrQuerySurfaceGL, width, height));
    COMPV_CHECK_CODE_RETURN(CompVGLSurface::newObj(&layer_->m_ptrCoverSurfaceGL, width, height));

    *layer = layer_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
