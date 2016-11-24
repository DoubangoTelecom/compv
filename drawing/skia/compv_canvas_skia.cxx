/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/skia/compv_canvas_skia.h"
#if HAVE_SKIA
#include "compv/gl/compv_gl_headers.h"
#include "compv/gl/compv_gl_func.h"
#include "compv/base/compv_base.h"

#include <GrContext.h>
#include <SkCanvas.h>
#include <SkGraphics.h>
#include <SkSurface.h>
#include <gl/GrGLInterface.h>

// FIXME: create surface once and must be associated to a context
// FIXME: use 'drawPoints' for multiple points and for multiple lines

COMPV_NAMESPACE_BEGIN()

CompVCanvasImplSkia::CompVCanvasImplSkia()
	: CompVCanvasImpl()
{
}

CompVCanvasImplSkia::~CompVCanvasImplSkia()
{
	
}


// https://github.com/google/skia/blob/master/example/SkiaSDLExample.cpp
// https://github.com/google/skia/blob/master/experimental/GLFWTest/glfw_main.cpp
// https://groups.google.com/forum/#!topic/skia-discuss/P4GO92rxIaM
// http://stackoverflow.com/questions/12157646/how-to-render-offscreen-on-opengl
// https://developer.apple.com/library/content/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_offscreen/opengl_offscreen.html

static SkPath create_star() {
	static const int kNumPoints = 5;
	SkPath concavePath;
	SkPoint points[kNumPoints] = { { 0, SkIntToScalar(-50) } };
	SkMatrix rot;
	rot.setRotate(SkIntToScalar(360) / kNumPoints);
	for (int i = 1; i < kNumPoints; ++i) {
		rot.mapPoints(points + i, points + i - 1, 1);
	}
	concavePath.moveTo(points[0]);
	for (int i = 0; i < kNumPoints; ++i) {
		concavePath.lineTo(points[(2 * i) % kNumPoints]);
	}
	concavePath.setFillType(SkPath::kEvenOdd_FillType);
	SkASSERT(!concavePath.isConvex());
	concavePath.close();
	return concavePath;
}

COMPV_ERROR_CODE CompVCanvasImplSkia::drawText(const void* textPtr, size_t textLengthInBytes, int x, int y)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	GLint bufferWidth = 0, bufferHeight = 0;
	//GrContext *sContext = NULL;
	SkSurface *sSurface = NULL;

	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &bufferWidth);
	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &bufferHeight);
	COMPV_CHECK_EXP_RETURN(!bufferWidth || !bufferHeight, COMPV_ERROR_CODE_E_GL);

	// setup GrContext
	SkAutoTUnref<const GrGLInterface> interf(GrGLCreateNativeInterface());

	// To use NVPR, comment this out
	interf.reset(GrGLInterfaceRemoveNVPR(interf));
	SkASSERT(interf);

	//SkAutoTUnref<const GrGLInterface> interface(GrGLCreateNativeInterface());

	// To use NVPR, comment this out
	//interface.reset(GrGLInterfaceRemoveNVPR(interface));
	//SkASSERT(interface);

	// setup contexts
	SkAutoTUnref<GrContext> grContext(GrContext::Create(kOpenGL_GrBackend,
		(GrBackendContext)interf.get()));

	//if (!sContext) {
	//	sContext = GrContext::Create(kOpenGL_GrBackend, 0);
	//}

#if 0
	GrBackendRenderTargetDesc desc;
	desc.fWidth = 640;
	desc.fHeight = 480;
	desc.fConfig = kSkia8888_GrPixelConfig;
	desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
	desc.fSampleCnt = 1;
	desc.fStencilBits = 0;
	desc.fRenderTargetHandle = 0;  // assume default framebuffer
#elif 0
	GLuint bufferID = 0;
	GrBackendRenderTargetDesc desc;
	desc.fWidth = SkScalarRoundToInt(640);
	desc.fHeight = SkScalarRoundToInt(480);
	desc.fConfig = kSkia8888_GrPixelConfig;
	desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
	desc.fSampleCnt = 0;
	desc.fStencilBits = 8;
	desc.fRenderTargetHandle = bufferID;
#else
	// Wrap the frame buffer object attached to the screen in a Skia render target so Skia can
	// render to it
	GrBackendRenderTargetDesc desc;
	static const int kStencilBits = 8;  // Skia needs 8 stencil bits
	static const int kMsaaSampleCount = 0;
	desc.fWidth = bufferWidth;
	desc.fHeight = bufferHeight;
	desc.fConfig = kSkia8888_GrPixelConfig;
	desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
	desc.fSampleCnt = kMsaaSampleCount;
	desc.fStencilBits = kStencilBits;
	
	GLint buffer;
	COMPV_glGetIntegerv(GLenum(GL_FRAMEBUFFER_BINDING), &buffer);
	if (!buffer) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED_GPU();
	}
	// GrGLint buffer;
	//GR_GL_GetIntegerv(interf, GR_GL_FRAMEBUFFER_BINDING, &buffer);
	desc.fRenderTargetHandle = buffer;
#endif

	// setup SkSurface
	// To use distance field text, use commented out SkSurfaceProps instead
	// SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
	//                      SkSurfaceProps::kLegacyFontHost_InitType);
	SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

	if (!sSurface) {
		sSurface = SkSurface::MakeFromBackendRenderTarget(grContext, desc, &props).release();
	}
	SkCanvas* canvas = NULL;
	if (sSurface) {
		canvas = sSurface->getCanvas();   // We don't manage this pointer's lifetime.
	}

#if 1
	//canvas->clear(rand()&1?SK_ColorWHITE: SK_ColorRED);

	SkPaint paint;
	paint.setFilterQuality(kLow_SkFilterQuality);
	paint.setColor(SK_ColorRED);
	//SkRandom rand;
	//paint.setColor(rand.nextU() | 0x44808080);
	paint.setTextSize(15.0f);
	paint.setAntiAlias(true);

	//canvas->clear(SK_ColorBLACK);
#if 1
	canvas->drawText(textPtr, textLengthInBytes, SkIntToScalar(422), SkIntToScalar(102), paint);
	//canvas->drawPoint();
	canvas->drawCircle(SkIntToScalar(x), SkIntToScalar(y), SkIntToScalar(5), paint);

	canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(50), SkIntToScalar(50), paint);
	canvas->drawCircle(SkIntToScalar(bufferWidth - 50), SkIntToScalar(50), SkIntToScalar(50), paint);
	canvas->drawCircle(SkIntToScalar(50), SkIntToScalar(bufferHeight - 50), SkIntToScalar(50), paint);
	canvas->drawCircle(SkIntToScalar(bufferWidth - 50), SkIntToScalar(bufferHeight - 50), SkIntToScalar(50), paint);
	//canvas->drawLine(SkIntToScalar(0), SkIntToScalar(0), SkIntToScalar(500), SkIntToScalar(500), paint);
#else
	std::string outString = "Hello skia";
	canvas->drawText(outString.c_str(),outString.length(), SkIntToScalar(100), SkIntToScalar(100), paint);
#endif

	//paint.reset();
	//paint.setAntiAlias(true);
	//paint.setColor(SK_ColorWHITE);
	const SkScalar scale = 256.0f;
	const SkScalar R = 0.45f * scale;
	const SkScalar TAU = 6.2831853f;
	SkPath path;
	path.moveTo(R, 0.0f);
	for (int i = 1; i < 7; ++i) {
		SkScalar theta = 3 * i * TAU / 7;
		path.lineTo(R * cos(theta), R * sin(theta));
	}
	path.close();
	canvas->translate(0.5f * scale, 0.5f * scale);
	canvas->drawPath(path, paint);
	
	SkRect r(SkRect::MakeWH(200, 200));
	paint.setARGB(200, 100, 100, 100);
	canvas->drawOval(r, paint);

#endif
	

	//canvas->restore();
	canvas->flush();


	if (sSurface) {
		delete sSurface;
		sSurface = NULL;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCanvasImplSkia::drawLine(int x0, int y0, int x1, int y1)
{
	COMPV_DEBUG_INFO_CODE_FOR_TESTING();
	GLint bufferWidth = 0, bufferHeight = 0;
	SkSurface *sSurface = NULL;

	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &bufferWidth);
	COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &bufferHeight);
	COMPV_CHECK_EXP_RETURN(!bufferWidth || !bufferHeight, COMPV_ERROR_CODE_E_GL);

	// setup GrContext
	SkAutoTUnref<const GrGLInterface> interf(GrGLCreateNativeInterface());

	// To use NVPR, comment this out
	interf.reset(GrGLInterfaceRemoveNVPR(interf));
	SkASSERT(interf);

	// setup contexts
	SkAutoTUnref<GrContext> grContext(GrContext::Create(kOpenGL_GrBackend,
		(GrBackendContext)interf.get()));


	// Wrap the frame buffer object attached to the screen in a Skia render target so Skia can
	// render to it
	GrBackendRenderTargetDesc desc;
	static const int kStencilBits = 8;  // Skia needs 8 stencil bits
	static const int kMsaaSampleCount = 0;
	desc.fWidth = bufferWidth;
	desc.fHeight = bufferHeight;
	desc.fConfig = kSkia8888_GrPixelConfig;
	desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
	desc.fSampleCnt = kMsaaSampleCount;
	desc.fStencilBits = kStencilBits;

	GLint buffer;
	COMPV_glGetIntegerv(GLenum(GL_FRAMEBUFFER_BINDING), &buffer);
	if (!buffer) {
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED_GPU();
	}
	// GrGLint buffer;
	//GR_GL_GetIntegerv(interf, GR_GL_FRAMEBUFFER_BINDING, &buffer);
	desc.fRenderTargetHandle = buffer;

	// setup SkSurface
	// To use distance field text, use commented out SkSurfaceProps instead
	SkSurfaceProps props(SkSurfaceProps::kUseDeviceIndependentFonts_Flag,
	                      SkSurfaceProps::kLegacyFontHost_InitType);
	//SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);

	if (!sSurface) {
		sSurface = SkSurface::MakeFromBackendRenderTarget(grContext, desc, &props).release();
	}
	SkCanvas* canvas = NULL;
	if (sSurface) {
		canvas = sSurface->getCanvas();   // We don't manage this pointer's lifetime.
	}

	SkPaint paint;
	paint.setFilterQuality(kLow_SkFilterQuality);
	paint.setColor(SK_ColorRED);
	//paint.setAutohinted(true);
	//paint.setBlendMode(SkBlendMode::kColor);
	//SkRandom rand;
	//paint.setColor(rand.nextU() | 0x44808080);
	//paint.setTextSize(SkIntToScalar(40));
	paint.setAntiAlias(false);
	paint.setStyle(SkPaint::kStroke_Style);
	paint.setStrokeWidth(2);
	//canvas->drawLine(SkIntToScalar(0), SkIntToScalar(0), SkIntToScalar(500), SkIntToScalar(500), paint);
	canvas->drawLine(SkIntToScalar(x0), SkIntToScalar(y0), SkIntToScalar(x1), SkIntToScalar(y1), paint);

	canvas->flush();

	if (sSurface) {
		delete sSurface;
		sSurface = NULL;
	}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCanvasImplSkia::newObj(CompVCanvasImplSkiaPtrPtr skiaCanvas)
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_EXP_RETURN(!skiaCanvas, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVCanvasImplSkiaPtr skiaCanvas_ = new CompVCanvasImplSkia();
	COMPV_CHECK_EXP_RETURN(!skiaCanvas_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*skiaCanvas = skiaCanvas_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* HAVE_SKIA */

