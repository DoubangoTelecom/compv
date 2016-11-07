/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/skia/compv_canvas_skia.h"
#if HAVE_SKIA
#include "compv/drawing/opengl/compv_headers_gl.h"
#include "compv/base/compv_base.h"

#include <GrContext.h>
#include <SkCanvas.h>
#include <SkGraphics.h>
#include <SkSurface.h>
#include <gl/GrGLInterface.h>

COMPV_NAMESPACE_BEGIN()

CompVCanvasSkia::CompVCanvasSkia()
	: CompVCanvas()
{
}

CompVCanvasSkia::~CompVCanvasSkia()
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

COMPV_ERROR_CODE CompVCanvasSkia::test()
{
	//GrContext *sContext = NULL;
	SkSurface *sSurface = NULL;

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
	static const int kMsaaSampleCount = 0; //4;
#if COMPV_OS_ANDROID
	desc.fWidth = 1080;
	desc.fHeight = 1776;
#else
	desc.fWidth = 640;
	desc.fHeight = 480;
#endif
	desc.fConfig = kSkia8888_GrPixelConfig;
	desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
	desc.fSampleCnt = kMsaaSampleCount;
	desc.fStencilBits = kStencilBits;
	
	GLint buffer;
	glGetIntegerv(GLenum(GL_FRAMEBUFFER_BINDING), &buffer);
	if (buffer == 0) {
		COMPV_DEBUG_WARN("Drawing to system buffer");
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
	//canvas->clear(SK_ColorBLACK);

	SkPaint paint;
	paint.setFilterQuality(kLow_SkFilterQuality);
	paint.setColor(SK_ColorRED);
	//SkRandom rand;
	//paint.setColor(rand.nextU() | 0x44808080);
	paint.setTextSize(15.0f);
	paint.setAntiAlias(true);

	//canvas->clear(SK_ColorBLACK);
#if 1
	static int count = 0;
	char buff_[33] = { 0 };
	snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(++count));
	std::string outString = "Hello skia " + std::string(buff_);
	canvas->drawText(outString.c_str(), outString.length(), SkIntToScalar(50), SkIntToScalar(500), paint);
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
	
	//canvas->drawPath(create_star(), paint);
	
	SkRect r(SkRect::MakeWH(200, 200));
	paint.setARGB(200, 100, 100, 100);
	canvas->drawOval(r, paint);

#endif
	//glDisable(GL_BLEND);
	//glDisable(GL_VERTEX_PROGRAM_POINT_SIZE);
	//glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_ARB);
	//glDisable(GL_VERTEX_PROGRAM_POINT_SIZE_NV);
	//glDisable(GL_VERTEX_ATTRIB_ARRAY0_NV);
	//glDisable(GL_VERTEX_ATTRIB_ARRAY1_NV);
	/*glBindVertexArray(0);
	//glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
	glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_READ_FRAMEBUFFER_EXT, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_DRAW_FRAMEBUFFER_EXT, 0);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glBindFramebuffer(GL_FRAMEBUFFER_EXT, 0);
	glUseProgram(0);
	glUseProgramObjectARB(0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER_ARB, 0);*/
	//glDrawBuffer(GL_BACK);
	//glEnable(GL_DOUBLEBUFFER);
	//glEnable(GL_DITHER);
	//glEnable(GL_DEPTH_WRITEMASK);

	//canvas->clear(SK_ColorBLACK);

	//canvas->restore();
	canvas->flush();
	//sContext->resetContext();
#if 0
	// convert.exe -depth 8 -size 640x480 image.rgba image.png
	uint8_t* data = (uint8_t*)malloc(640 * 480 * 4);
	glReadBuffer(GL_COLOR_ATTACHMENT0);
	glReadPixels(0, 0, 640, 480, GL_RGBA, GL_UNSIGNED_BYTE, data);
	FILE* file = fopen("C:/Projects/image.rgba", "wb+");
	fwrite(data, 1, (640 * 480 * 4), file);
	fclose(file);
	free(data);
#elif 0
	SkBitmap tmpBitmap;
	tmpBitmap.allocPixels(SkImageInfo::MakeN32Premul(640, 480));
	//sContext->readSurfacePixels(sSurface->, 0, 0, 640, 480, GrPixelConfig::kBGRA_8888_GrPixelConfig, tmpBitmap.getPixels());
	//sContext->readRenderTargetPixels(fRenderTarget, 0, 0, 1000, 1000, GrPixelConfig::kBGRA_8888_GrPixelConfig, tmpBitmap.getPixels());
	sSurface->readPixels(SkImageInfo::MakeN32Premul(640, 480), tmpBitmap.getPixels(), 640, 0, 0);
	// SkString str;
	//bool ret = SkImageEncoder::EncodeFile("C:/Projects/image.png", tmpBitmap, SkImageEncoder::kPNG_Type, 1);

	//uint8_t* data = (uint8_t*)tmpBitmap.getPixels();
	//for (int i = 0; i < 640 * 480 * 4; ++i)data[i] = rand();

	FILE* file = fopen("C:/Projects/image.rgba", "wb+");
	fwrite(tmpBitmap.getPixels(), 1, (640 * 480 * 4), file);
	fclose(file);
#elif 0
	{
		int width = 640;
		int heigth = 480;
		float linewidth = 10.0f;

		SkImageInfo info = SkImageInfo::Make(
			width,
			heigth,
			SkColorType::kRGBA_8888_SkColorType,
			SkAlphaType::kOpaque_SkAlphaType
		);

		SkBitmap img;
		img.allocPixels(info);
		SkCanvas canvas(img);
		canvas.drawColor(SK_ColorBLACK);

		SkPaint paint;
		paint.setColor(SK_ColorWHITE);
		paint.setAlpha(255);
		paint.setAntiAlias(false);
		paint.setStrokeWidth(linewidth);
		paint.setStyle(SkPaint::kStroke_Style);

		canvas.drawCircle(500.0f, 500.0f, 100.0f, paint);

		bool success = SkImageEncoder::EncodeFile("C:\\Projects\\img.png", img,
			SkImageEncoder::kPNG_Type, 1);
		printf("success=%s", success ? "true" : "false");
	}
#endif

	if (sSurface) {
		delete sSurface;
		sSurface = NULL;
	}
	//if (sContext) {
	//	delete sContext;
	//	sContext = NULL;
	//}

	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCanvasSkia::newObj(CompVCanvasSkiaPtrPtr skiaCanvas)
{
	COMPV_CHECK_CODE_RETURN(CompVBase::init());
	COMPV_CHECK_EXP_RETURN(skiaCanvas == NULL, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	CompVCanvasSkiaPtr skiaCanvas_ = new CompVCanvasSkia();
	COMPV_CHECK_EXP_RETURN(!skiaCanvas_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*skiaCanvas = skiaCanvas_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()

#endif /* HAVE_SKIA */

