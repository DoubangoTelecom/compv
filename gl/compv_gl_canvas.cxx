/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/gl/compv_gl_canvas.h"
#include "compv/gl/compv_gl_headers.h"
#if defined(HAVE_OPENGL) || defined(HAVE_OPENGLES)
#include "compv/gl/compv_gl_func.h"

COMPV_NAMESPACE_BEGIN()

CompVGLCanvas::CompVGLCanvas(CompVGLFboPtr ptrFBO)
    : CompVCanvas()
    , m_bEmpty(true)
    , m_ptrFBO(ptrFBO)
{

}

CompVGLCanvas::~CompVGLCanvas()
{

}

COMPV_ERROR_CODE CompVGLCanvas::clear(const CompVDrawingOptions* options COMPV_DEFAULT(nullptr)) /*Overrides(CompVCanvas)*/
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_CODE_BAIL(err = m_ptrFBO->bind());

	if (options) {
		COMPV_glClearColor(options->color[0], options->color[1], options->color[2], options->color[3]);
	}
	else {
		COMPV_glClearColor(0.f, 0.f, 0.f, 1.f);
	}
	COMPV_glClearStencil(0);
	COMPV_glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

bail:
	COMPV_CHECK_CODE_NOP(m_ptrFBO->unbind());
	unMakeEmpty();
	return err;
}

// Public override
COMPV_ERROR_CODE CompVGLCanvas::drawTexts(const CompVStringVector& texts, const CompVPointFloat32Vector& positions, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr)) /*Overrides(CompVCanvasInterface)*/
{
	if (positions.empty()) {
		return COMPV_ERROR_CODE_S_OK;
	}
    COMPV_CHECK_EXP_RETURN(texts.size() != positions.size(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	if (!m_ptrDrawTexts) {
		COMPV_CHECK_CODE_RETURN(CompVGLDrawTexts::newObj(&m_ptrDrawTexts));
	}
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_CODE_BAIL(err = m_ptrFBO->bind());
	COMPV_CHECK_CODE_BAIL(err = m_ptrDrawTexts->texts(texts, positions, options));

bail:
	COMPV_CHECK_CODE_NOP(m_ptrFBO->unbind());
	unMakeEmpty();
	return err;
}

// Public override
COMPV_ERROR_CODE CompVGLCanvas::drawLines(const CompVLineFloat32Vector& lines, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr)) /*Overrides(CompVCanvasInterface)*/
{   
	if (lines.empty()) {
		return COMPV_ERROR_CODE_S_OK;
	}
	CompVMatPtr glPoints2D;
	COMPV_CHECK_CODE_RETURN(CompVGLCanvas::linesBuild(&glPoints2D, lines));
	COMPV_CHECK_CODE_RETURN(CompVGLCanvas::linesApplyOptions(glPoints2D, options));
	COMPV_CHECK_CODE_RETURN(drawLinesGL(glPoints2D->ptr<const CompVGLPoint2D>(), glPoints2D->cols(), options));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvas::drawLines(const CompVPointFloat32Vector& points, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr)) /*Overrides(CompVCanvas)*/
{
	if (points.empty()) {
		return COMPV_ERROR_CODE_S_OK;
	}
	CompVMatPtr glMemPoints;
	COMPV_CHECK_CODE_RETURN(CompVGLCanvas::pointsBuild(&glMemPoints, points, options));
	COMPV_CHECK_CODE_RETURN(drawLinesGL(glMemPoints->ptr<const CompVGLPoint2D>(), glMemPoints->cols(), options, true));
	return COMPV_ERROR_CODE_S_OK;
}

// rectangle with square angles, for arbitrary angles use drawLines
// Public override
COMPV_ERROR_CODE CompVGLCanvas::drawRectangles(const CompVRectFloat32Vector& rects, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr)) /*Overrides(CompVCanvas)*/
{
	if (rects.empty()) {
		return COMPV_ERROR_CODE_S_OK;
	}

	const size_t countRects = rects.size();
	if (countRects == 1) { // drawing a single rectangle is very common
		const CompVRectFloat32& roi = rects[0];
		const compv_float32_t x0[4] = { roi.left, roi.right, roi.right, roi.left };
		const compv_float32_t y0[4] = { roi.top, roi.top, roi.bottom, roi.bottom };
		const compv_float32_t x1[4] = { roi.right, roi.right, roi.left, roi.left };
		const compv_float32_t y1[4] = { roi.top, roi.bottom, roi.bottom, roi.top };
		COMPV_CHECK_CODE_RETURN(drawLinesGL(x0, y0, x1, y1, 4, options));
	}
	else {
		const size_t countLines = (countRects << 2);
		CompVMatPtr points;
		COMPV_CHECK_CODE_RETURN((CompVMat::newObj<compv_float32_t>(&points, 4, countLines, 1)));
		compv_float32_t *px0 = points->ptr<compv_float32_t>(0);
		compv_float32_t *py0 = points->ptr<compv_float32_t>(1);
		compv_float32_t *px1 = points->ptr<compv_float32_t>(2);
		compv_float32_t *py1 = points->ptr<compv_float32_t>(3);
		for (CompVRectFloat32Vector::const_iterator rect = rects.begin(); rect < rects.end(); ++rect) {
			px0[0] = rect->left, px0[1] = rect->right, px0[2] = rect->right, px0[3] = rect->left, px0 += 4;
			py0[0] = rect->top, py0[1] = rect->top, py0[2] = rect->bottom, py0[3] = rect->bottom, py0 += 4;
			px1[0] = rect->right, px1[1] = rect->right, px1[2] = rect->left, px1[3] = rect->left, px1 += 4;
			py1[0] = rect->top, py1[1] = rect->bottom, py1[2] = rect->bottom, py1[3] = rect->top, py1 += 4;
		}
		COMPV_CHECK_CODE_RETURN(drawLinesGL(
			points->ptr<const compv_float32_t>(0), points->ptr<const compv_float32_t>(1),
			points->ptr<const compv_float32_t>(2), points->ptr<const compv_float32_t>(3),
			countLines, options
		));
	}
	return COMPV_ERROR_CODE_S_OK;
}

// Public override
COMPV_ERROR_CODE CompVGLCanvas::drawPoints(const CompVPointFloat32Vector& points, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr)) /*Overrides(CompVCanvas)*/
{
	if (points.empty()) {
		return COMPV_ERROR_CODE_S_OK;
	}
	if (!m_ptrDrawPoints) {
		COMPV_CHECK_CODE_RETURN(CompVGLDrawPoints::newObj(&m_ptrDrawPoints));
	}

	CompVMatPtr glMemPoints;
	COMPV_CHECK_CODE_RETURN(CompVGLCanvas::pointsBuild(&glMemPoints, points, options));

	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_CODE_BAIL(err = m_ptrFBO->bind());
	COMPV_CHECK_CODE_BAIL(err = m_ptrDrawPoints->points(glMemPoints->ptr<CompVGLPoint2D>(), static_cast<GLsizei>(points.size()), options));

bail:
	COMPV_CHECK_CODE_NOP(m_ptrFBO->unbind());
	unMakeEmpty();
	return err;
}

// Public override
COMPV_ERROR_CODE CompVGLCanvas::drawInterestPoints(const CompVInterestPointVector& interestPoints, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr)) /*Overrides(CompVCanvasInterface)*/
{
	if (interestPoints.empty()) {
		return COMPV_ERROR_CODE_S_OK;
	}
	// TODO(dmi): we could for example have the point size depending on the strength, this is why we have 'drawPoints()' and 'drawInterestPoints()' for the future
	CompVPointFloat32Vector points;
	points.resize(interestPoints.size());
	CompVPointFloat32Vector::iterator i = points.begin();
	std::for_each(interestPoints.begin(), interestPoints.end(), [&i](const CompVInterestPoint &p) {
		i->x = p.x;
		i++->y = p.y;
	});
	COMPV_CHECK_CODE_RETURN(drawPoints(points, options));
	return COMPV_ERROR_CODE_S_OK;
}

// Internal implementation
COMPV_ERROR_CODE CompVGLCanvas::drawLinesGL(const compv_float32_t* x0, const compv_float32_t* y0, const compv_float32_t* x1, const compv_float32_t* y1, const size_t count, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr), bool connected COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!x0 || !y0 || !x1 || !y1 || !count, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVMatPtr glPoints2D;
	COMPV_CHECK_CODE_RETURN(CompVGLCanvas::linesBuild(&glPoints2D, x0, y0, x1, y1, count));
	COMPV_CHECK_CODE_RETURN(CompVGLCanvas::linesApplyOptions(glPoints2D, options));
	COMPV_CHECK_CODE_RETURN(drawLinesGL(glPoints2D->ptr<const CompVGLPoint2D>(), glPoints2D->cols(), options, connected));
	return COMPV_ERROR_CODE_S_OK;
}

// Internal implemenation
COMPV_ERROR_CODE CompVGLCanvas::drawLinesGL(const CompVGLPoint2D* lines, const size_t count, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr), bool loop COMPV_DEFAULT(false))
{
	COMPV_CHECK_EXP_RETURN(!lines || !count, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	if (!m_ptrDrawLines) {
		COMPV_CHECK_CODE_RETURN(CompVGLDrawLines::newObj(&m_ptrDrawLines));
	}
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_CODE_BAIL(err = m_ptrFBO->bind());
	COMPV_CHECK_CODE_BAIL(err = m_ptrDrawLines->lines(lines, static_cast<GLsizei>(count), options, loop));

bail:
	COMPV_CHECK_CODE_NOP(m_ptrFBO->unbind());
	unMakeEmpty();
	return err;
}

// Public override
bool CompVGLCanvas::haveDrawTexts() const /*Overrides(CompVCanvas)*/
{
#if HAVE_FREETYPE
	return true;
#else
	return false;
#endif
}

COMPV_ERROR_CODE CompVGLCanvas::close()
{
	// Function always called with valid GL context -> good time to destroy drawers and release GL allocated objects
	m_ptrDrawPoints = nullptr;
	m_ptrDrawLines = nullptr;
	m_ptrDrawTexts = nullptr;
    if (m_ptrFBO) {
		COMPV_CHECK_CODE_NOP(m_ptrFBO->close());
    }
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvas::linesBuild(CompVMatPtrPtr glPoints2D, const CompVLineFloat32Vector& lines)
{
	COMPV_CHECK_EXP_RETURN(!glPoints2D || lines.empty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	const size_t count = lines.size();
	COMPV_CHECK_CODE_RETURN((CompVMat::newObj<CompVGLPoint2D, COMPV_MAT_TYPE_STRUCT>(glPoints2D, 1, (count << 1), 1)));
	CompVGLPoint2D* glMemLine_ = (*glPoints2D)->ptr<CompVGLPoint2D>();
	// x0, y0, x1, y1 : (x0, y0) -> (x1, y1)
	for (CompVLineFloat32Vector::const_iterator i = lines.begin(); i < lines.end(); ++i, glMemLine_ += 2) {
		glMemLine_->position[0] = static_cast<GLfloat>(i->a.x);
		glMemLine_->position[1] = static_cast<GLfloat>(i->a.y);
		(glMemLine_ + 1)->position[0] = static_cast<GLfloat>(i->b.x);
		(glMemLine_ + 1)->position[1] = static_cast<GLfloat>(i->b.y);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvas::linesBuild(CompVMatPtrPtr glPoints2D, const compv_float32_t* x0, const compv_float32_t* y0, const compv_float32_t* x1, const compv_float32_t* y1, size_t count)
{
	COMPV_CHECK_EXP_RETURN(!x0 || !y0 || !x1 || !y1 || !count, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_CODE_RETURN((CompVMat::newObj<CompVGLPoint2D, COMPV_MAT_TYPE_STRUCT>(glPoints2D, 1, (count << 1), 1)));
	CompVGLPoint2D* glMemLine_ = (*glPoints2D)->ptr<CompVGLPoint2D>();
	// x0, y0, x1, y1 : (x0, y0) -> (x1, y1)
	for (size_t i = 0; i < count; ++i, glMemLine_ += 2) {
		glMemLine_->position[0] = static_cast<GLfloat>(x0[i]);
		glMemLine_->position[1] = static_cast<GLfloat>(y0[i]);
		(glMemLine_ + 1)->position[0] = static_cast<GLfloat>(x1[i]);
		(glMemLine_ + 1)->position[1] = static_cast<GLfloat>(y1[i]);
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvas::linesApplyOptions(CompVMatPtr& glPoints2D, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr))
{
	COMPV_CHECK_EXP_RETURN(!glPoints2D, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	bool randomColors = (!options || options->colorType == COMPV_DRAWING_COLOR_TYPE_RANDOM);
	const size_t count = glPoints2D->cols();
	COMPV_ASSERT(!(count & 1));
	CompVGLPoint2D* glMemLines_ = glPoints2D->ptr<CompVGLPoint2D>();
	CompVGLPoint2D *a = glMemLines_, *b = a + 1;
	if (randomColors) {
		const GLfloat(*color)[3];
		for (size_t i = 0; i < count; i += 2, a+=2, b +=2) {
			color = &kCompVGLRandomColors[rand() % kCompVGLRandomColorsCount];
			a->color[0] = b->color[0] = (*color)[0];
			a->color[1] = b->color[1] = (*color)[1];
			a->color[2] = b->color[2] = (*color)[2];
			a->color[3] = b->color[3] = 1.f; // alpha
		}
	}
	else {
		for (size_t i = 0; i < count; i += 2, a += 2, b += 2) {
			a->color[0] = b->color[0] = options->color[0];
			a->color[1] = b->color[1] = options->color[1];
			a->color[2] = b->color[2] = options->color[2];
			a->color[3] = b->color[3] = options->color[3];
		}
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvas::pointsBuild(CompVMatPtrPtr glPoints2D, const CompVPointFloat32Vector& points, const CompVDrawingOptions* options COMPV_DEFAULT(nullptr))
{
	COMPV_CHECK_EXP_RETURN(!glPoints2D || points.empty(), COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVGLPoint2D* glMemPoint_;
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	const GLfloat(*color)[3];

	COMPV_CHECK_CODE_RETURN((CompVMat::newObj<CompVGLPoint2D, COMPV_MAT_TYPE_STRUCT>(glPoints2D, 1, points.size(), 1)));

	CompVPointFloat32Vector::const_iterator it;
	bool randomColors = (!options || options->colorType == COMPV_DRAWING_COLOR_TYPE_RANDOM);
	if (randomColors) {
		for (it = points.begin(), glMemPoint_ = (*glPoints2D)->ptr<CompVGLPoint2D>(); it != points.end(); ++it, ++glMemPoint_) {
			// x, y
			glMemPoint_->position[0] = static_cast<GLfloat>((*it).x);
			glMemPoint_->position[1] = static_cast<GLfloat>((*it).y);
			// r, g, b
			color = &kCompVGLRandomColors[rand() % kCompVGLRandomColorsCount];
			glMemPoint_->color[0] = (*color)[0];
			glMemPoint_->color[1] = (*color)[1];
			glMemPoint_->color[2] = (*color)[2];
			glMemPoint_->color[3] = 1.f; // alpha
		}
	}
	else {
		for (it = points.begin(), glMemPoint_ = (*glPoints2D)->ptr<CompVGLPoint2D>(); it != points.end(); ++it, ++glMemPoint_) {
			// x, y
			glMemPoint_->position[0] = static_cast<GLfloat>((*it).x);
			glMemPoint_->position[1] = static_cast<GLfloat>((*it).y);
			// r, g, b
			glMemPoint_->color[0] = options->color[0];
			glMemPoint_->color[1] = options->color[1];
			glMemPoint_->color[2] = options->color[2];
			glMemPoint_->color[3] = options->color[3];
		}
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVGLCanvas::newObj(CompVGLCanvasPtrPtr canvas, CompVGLFboPtr ptrFBO)
{
    COMPV_CHECK_EXP_RETURN(!canvas || !ptrFBO, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
    CompVGLCanvasPtr canvas_ = new CompVGLCanvas(ptrFBO);
    COMPV_CHECK_EXP_RETURN(!canvas_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
    *canvas = canvas_;
    return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* defined(HAVE_OPENGL) || defined(HAVE_OPENGLES) */
