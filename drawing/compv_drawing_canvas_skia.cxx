/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/drawing/compv_drawing_canvas_skia.h"
#if HAVE_SKIA
#include "compv/gl/compv_gl_headers.h"
#include "compv/gl/compv_gl_func.h"
#include "compv/gl/compv_gl_utils.h"
#include "compv/base/compv_base.h"

// Link input: $(OutDir)CompVBase.lib;$(OutDir)CompVGL.lib;skia_core.lib;skia_effects.lib;skia_images.lib;skia_opts.lib;skia_opts_hsw.lib;skia_opts_sse41.lib;skia_opts_sse42.lib;skia_opts_ssse3.lib;skia_opts_avx.lib;skia_ports.lib;skia_sfnt.lib;skia_utils.lib;skia_skgpu.lib;skia_codec.lib;libjpeg-turbo.lib;libpng_static.lib;libwebp_dec.lib;libwebp_dsp.lib;libwebp_demux.lib;libwebp_utils.lib;raw_codec.lib;dng_sdk.lib;piex.lib;giflib.lib;zlib.lib;libSkKTX.lib;libetc1.lib;glew32s.lib;OpenGL32.lib;Glu32.lib;SDL2.lib;Winmm.lib;imm32.lib;version.lib;%(AdditionalDependencies)

// FIXME: create surface once and must be associated to a context
// FIXME: use 'drawPoints' for multiple points and for multiple lines

// https://github.com/google/skia/blob/master/example/SkiaSDLExample.cpp
// https://github.com/google/skia/blob/master/experimental/GLFWTest/glfw_main.cpp
// https://groups.google.com/forum/#!topic/skia-discuss/P4GO92rxIaM
// http://stackoverflow.com/questions/12157646/how-to-render-offscreen-on-opengl
// https://developer.apple.com/library/content/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_offscreen/opengl_offscreen.html

#define COMPV_THIS_CLASSNAME	"CompVCanvasImplSkia"

COMPV_NAMESPACE_BEGIN()

static COMPV_ERROR_CODE CompVCanvasFactorySkia_newObj(CompVCanvasImplPtrPtr canvasImpl)
{
    CompVCanvasImplSkiaPtr canvasImplSkia;
    COMPV_CHECK_CODE_RETURN(CompVCanvasImplSkia::newObj(&canvasImplSkia));
    *canvasImpl = *canvasImplSkia;
    return COMPV_ERROR_CODE_S_OK;
}

const CompVCanvasFactory CompVCanvasFactorySkia = {
    "SKia",
    CompVCanvasFactorySkia_newObj
};

CompVCanvasImplSkia::CompVCanvasImplSkia()
    : CompVCanvasImpl()
	, m_pContextSkia(NULL)
	, m_pSurfaceSkia(NULL)
	, m_pcContextGL(NULL)
	, m_bInitialized(false)
{
}

CompVCanvasImplSkia::~CompVCanvasImplSkia()
{
	COMPV_CHECK_CODE_NOP(deInit());
}

bool CompVCanvasImplSkia::isContextGLChanged()const
{
	return CompVGLUtils::currentContext() != m_pcContextGL;
}

COMPV_ERROR_CODE CompVCanvasImplSkia::init()
{
	COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
	COMPV_CHECK_EXP_BAIL(!CompVGLUtils::currentContext(), (err = COMPV_ERROR_CODE_E_GL_NO_CONTEXT), "No OpenGL context");
	if (isContextGLChanged()) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "GL Context changed");
		COMPV_CHECK_CODE_BAIL(err = deInit());
	}
	if (!m_bInitialized) {
		COMPV_DEBUG_INFO_EX(COMPV_THIS_CLASSNAME, "Initializing...");

		GLint bufferWidth = 0, bufferHeight = 0, bufferName = 0;
		GrBackendRenderTargetDesc desc;

		COMPV_glGetIntegerv(GLenum(GL_FRAMEBUFFER_BINDING), &bufferName);
		COMPV_CHECK_EXP_BAIL((bufferName == kCompVGLNameInvalid), (err = COMPV_ERROR_CODE_E_SKIA), "bufferName is invalid");
		COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &bufferWidth);
		COMPV_glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &bufferHeight);
		COMPV_CHECK_EXP_BAIL(!bufferWidth || !bufferHeight, (err = COMPV_ERROR_CODE_E_SKIA), "bufferWidth or bufferHeight is equal to zero");

		SkAutoTUnref<const GrGLInterface> interf(GrGLCreateNativeInterface());
		SkASSERT(interf);
		//interf.reset(GrGLInterfaceRemoveNVPR(interf)); // remove GL_NV_path_rendering ?
		
		m_pContextSkia = GrContext::Create(kOpenGL_GrBackend, (GrBackendContext)interf.get());
		COMPV_CHECK_EXP_BAIL(!m_pContextSkia, (err = COMPV_ERROR_CODE_E_SKIA), "GrContext::Create(kOpenGL_GrBackend) failed");

		SkSurfaceProps props(SkSurfaceProps::kLegacyFontHost_InitType);
		desc.fWidth = bufferWidth;
		desc.fHeight = bufferHeight;
		desc.fConfig = kSkia8888_GrPixelConfig;
		desc.fOrigin = kBottomLeft_GrSurfaceOrigin;
		desc.fSampleCnt = 0;
		desc.fStencilBits = 8; // required
		desc.fRenderTargetHandle = bufferName;
		m_pSurfaceSkia = SkSurface::MakeFromBackendRenderTarget(m_pContextSkia, desc, &props).release();
		COMPV_CHECK_EXP_BAIL(!m_pSurfaceSkia, (err = COMPV_ERROR_CODE_E_SKIA), "SkSurface::MakeFromBackendRenderTarget failed");
		
		// update context associated to this surface
		m_pcContextGL = CompVGLUtils::currentContext();

		m_bInitialized = true;
	}

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_CHECK_CODE_NOP(deInit());
	}
	return err;
}

COMPV_ERROR_CODE CompVCanvasImplSkia::deInit()
{
	if (m_pSurfaceSkia) {
		delete m_pSurfaceSkia;
		m_pSurfaceSkia = NULL;
	}
	if (m_pContextSkia) {
		delete m_pContextSkia;
		m_pContextSkia = NULL;
	}
	m_bInitialized = false;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCanvasImplSkia::drawText(const void* textPtr, size_t textLengthInBytes, int x, int y) /*Overrides(CompVCanvasInterface)*/
{
#if 1
	// Only first call works
	//deInit();
	COMPV_CHECK_CODE_RETURN(init());
	SkCanvas* canvas = m_pSurfaceSkia->getCanvas(); // We don't manage this pointer's lifetime
	SkPaint paint;
	paint.setFilterQuality(kLow_SkFilterQuality);
	paint.setColor(SK_ColorRED);
	//SkRandom rand;
	//paint.setColor(rand.nextU() | 0x44808080);
	paint.setTextSize(15.0f);
	paint.setAntiAlias(true);
	canvas->drawText(textPtr, textLengthInBytes, SkIntToScalar(x), SkIntToScalar(y), paint);
	canvas->flush();
	return COMPV_ERROR_CODE_S_OK;
#else
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
#endif
}

COMPV_ERROR_CODE CompVCanvasImplSkia::drawLine(int x0, int y0, int x1, int y1) /*Overrides(CompVCanvasInterface)*/
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

COMPV_ERROR_CODE CompVCanvasImplSkia::close() /*Overrides(CompVCanvasImpl)*/
{
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

