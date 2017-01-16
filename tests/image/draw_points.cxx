#include "../tests_common.h"

#define TAG_TEST									"TestImageDraw"
#if COMPV_OS_WINDOWS
#	define COMPV_TEST_IMAGE_FOLDER					"C:/Projects/GitHub/data/colorspace"
#elif COMPV_OS_OSX
#	define COMPV_TEST_IMAGE_FOLDER					"/Users/mamadou/Projects/GitHub/data/colorspace"
#else
#	define COMPV_TEST_IMAGE_FOLDER					NULL
#endif
#define COMPV_TEST_IMAGE_PATH_TO_FILE(filename)		compv_tests_path_from_file(filename, COMPV_TEST_IMAGE_FOLDER)

#define COMPV_TEST_IMAGE_FILENAME					"equirectangular_1282x720_nv12.yuv"
#define COMPV_TEST_IMAGE_SUBTYPE					COMPV_SUBTYPE_PIXELS_NV12	
#define COMPV_TEST_IMAGE_WIDTH						1282
#define COMPV_TEST_IMAGE_HEIGHT						720
#define COMPV_TEST_IMAGE_STRIDE						1282

#define COMPV_TEST_IMAGE_WINDOW_WIDTH				1280
#define COMPV_TEST_IMAGE_WINDOW_HEIGHT				720

COMPV_OBJECT_DECLARE_PTRS(MyRunLoopListener2)
class CompVMyRunLoopListener2 : public CompVRunLoopListener
{
protected:
	CompVMyRunLoopListener2(CompVWindowPtr window, CompVMatPtr image) : m_ptrWindow(window), m_ptrImage(image) { }
public:
	virtual ~CompVMyRunLoopListener2() {}
	virtual COMPV_ERROR_CODE onStateChanged(COMPV_RUNLOOP_STATE newState) override /*Overrides(CompVRunLoopListener)*/;
	static void* COMPV_STDCALL threadDrawingGPU(void* arg);
	static COMPV_ERROR_CODE newObj(CompVMyRunLoopListener2PtrPtr listener, CompVWindowPtr window, CompVMatPtr image);
private:
	CompVWindowPtr m_ptrWindow;
	CompVMatPtr m_ptrImage;
	CompVSingleSurfaceLayerPtr m_ptrSingleSurfaceLayer;
	CompVThreadPtr m_ptrThreadDrawingGPU;
	bool m_bAnimating;
};

COMPV_ERROR_CODE draw_points()
{
	CompVWindowPtr window;
	CompVMatPtr image;
	CompVMyRunLoopListener2Ptr listener;
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_TEST_IMAGE_SUBTYPE, COMPV_TEST_IMAGE_WIDTH, COMPV_TEST_IMAGE_HEIGHT, COMPV_TEST_IMAGE_STRIDE, COMPV_TEST_IMAGE_PATH_TO_FILE(COMPV_TEST_IMAGE_FILENAME).c_str(), &image));
	COMPV_CHECK_CODE_RETURN(CompVWindow::newObj(&window, COMPV_TEST_IMAGE_WINDOW_WIDTH, COMPV_TEST_IMAGE_WINDOW_HEIGHT, "Test draw points!"));
	COMPV_CHECK_CODE_RETURN(CompVMyRunLoopListener2::newObj(&listener, window, image));
	COMPV_CHECK_CODE_RETURN(CompVDrawing::runLoop(*listener));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMyRunLoopListener2::onStateChanged(COMPV_RUNLOOP_STATE newState) /*Overrides(CompVRunLoopListener)*/
{
	switch (newState) {
	case COMPV_RUNLOOP_STATE_LOOP_STARTED:
		COMPV_CHECK_CODE_RETURN(m_ptrWindow->addSingleLayerSurface(&m_ptrSingleSurfaceLayer));
		return COMPV_ERROR_CODE_S_OK;

	case COMPV_RUNLOOP_STATE_ANIMATION_STARTED:
	case COMPV_RUNLOOP_STATE_ANIMATION_RESUMED:
		if (!m_bAnimating) {
			m_bAnimating = true;
			COMPV_CHECK_CODE_RETURN(CompVThread::newObj(&m_ptrThreadDrawingGPU, threadDrawingGPU, this));
		}
		return COMPV_ERROR_CODE_S_OK;

	case COMPV_RUNLOOP_STATE_ANIMATION_PAUSED:
	case COMPV_RUNLOOP_STATE_LOOP_STOPPED:
	case COMPV_RUNLOOP_STATE_ANIMATION_STOPPED:
		if (m_bAnimating) {
			m_bAnimating = false;
			if (m_ptrThreadDrawingGPU) {
				COMPV_CHECK_CODE_RETURN(m_ptrThreadDrawingGPU->join());
				m_ptrThreadDrawingGPU = NULL;
			}
		}
		return COMPV_ERROR_CODE_S_OK;

	default:
		return COMPV_ERROR_CODE_S_OK;
	}
}

void* COMPV_STDCALL CompVMyRunLoopListener2::threadDrawingGPU(void* arg)
{
	CompVMyRunLoopListener2Ptr ptrThis = reinterpret_cast<CompVMyRunLoopListener2*>(arg);
	size_t count = 0;
	while (ptrThis->m_bAnimating && CompVDrawing::isLoopRunning() /*&& !ptrWindow->isClosed()*/) { // EGL: window is created with closed state then, opened with beginDraw
		COMPV_CHECK_CODE_BAIL(ptrThis->m_ptrWindow->beginDraw());
		COMPV_CHECK_CODE_BAIL(ptrThis->m_ptrSingleSurfaceLayer->surface()->drawImage(ptrThis->m_ptrImage));
		std::string text = "Hello doubango telecom [" + CompVBase::to_string(count++) + "]";
		COMPV_CHECK_CODE_BAIL(ptrThis->m_ptrSingleSurfaceLayer->surface()->renderer()->canvas()->drawText(text.c_str(), text.length(), 463, 86)); // FIXME: canvas should be at surface()
		COMPV_CHECK_CODE_BAIL(ptrThis->m_ptrSingleSurfaceLayer->blit());
		COMPV_CHECK_CODE_BAIL(ptrThis->m_ptrWindow->endDraw());
	}

bail:
	return NULL;
}

COMPV_ERROR_CODE CompVMyRunLoopListener2::newObj(CompVMyRunLoopListener2PtrPtr listener, CompVWindowPtr window, CompVMatPtr image)
{
	COMPV_CHECK_EXP_RETURN(!listener || !window || !image, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!(*listener = new CompVMyRunLoopListener2(window, image)), COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	return COMPV_ERROR_CODE_S_OK;
}

