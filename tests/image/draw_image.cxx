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

COMPV_OBJECT_DECLARE_PTRS(MyRunLoopListener)
class CompVMyRunLoopListener : public CompVRunLoopListener, public CompVRunnable
{
protected:
	CompVMyRunLoopListener(CompVWindowPtr window, CompVMatPtr image) : m_ptrWindow(window), m_ptrImage(image) { }
public:
	virtual ~CompVMyRunLoopListener() {}
	virtual COMPV_ERROR_CODE onStateChanged(COMPV_RUNLOOP_STATE newState) override /*Overrides(CompVRunLoopListener)*/;
	virtual COMPV_ERROR_CODE running() override /*Overrides(CompVRunnable)*/;
	static COMPV_ERROR_CODE newObj(CompVMyRunLoopListenerPtrPtr listener, CompVWindowPtr window, CompVMatPtr image); 
private:
	CompVWindowPtr m_ptrWindow;
	CompVMatPtr m_ptrImage;
	CompVSingleSurfaceLayerPtr m_ptrSingleSurfaceLayer;
};

COMPV_ERROR_CODE draw_image()
{
	CompVWindowPtr window;
	CompVMatPtr image;
	CompVMyRunLoopListenerPtr listener;
	COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_TEST_IMAGE_SUBTYPE, COMPV_TEST_IMAGE_WIDTH, COMPV_TEST_IMAGE_HEIGHT, COMPV_TEST_IMAGE_STRIDE, COMPV_TEST_IMAGE_PATH_TO_FILE(COMPV_TEST_IMAGE_FILENAME).c_str(), &image));
	COMPV_CHECK_CODE_RETURN(CompVWindow::newObj(&window, COMPV_TEST_IMAGE_WINDOW_WIDTH, COMPV_TEST_IMAGE_WINDOW_HEIGHT, "Test draw image!"));
	COMPV_CHECK_CODE_RETURN(CompVMyRunLoopListener::newObj(&listener, window, image));
	COMPV_CHECK_CODE_RETURN(CompVDrawing::runLoop(*listener));
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMyRunLoopListener::onStateChanged(COMPV_RUNLOOP_STATE newState) /*Overrides(CompVRunLoopListener)*/
{
	switch (newState) {
	case compv::COMPV_RUNLOOP_STATE_LOOP_STARTED:
		COMPV_CHECK_CODE_RETURN(m_ptrWindow->addSingleLayerSurface(&m_ptrSingleSurfaceLayer));
		COMPV_CHECK_CODE_RETURN(CompVRunnable::start()); // call base class
		break;
	case compv::COMPV_RUNLOOP_STATE_ANIMATION_STARTED:
		break;
	case compv::COMPV_RUNLOOP_STATE_ANIMATION_PAUSED:
		break;
	case compv::COMPV_RUNLOOP_STATE_ANIMATION_RESUMED:
		break;
	case compv::COMPV_RUNLOOP_STATE_ANIMATION_STOPPED:
		break;
	case compv::COMPV_RUNLOOP_STATE_LOOP_STOPPED:
		break;
	default:
		break;
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMyRunLoopListener::running() /*Overrides(CompVRunnable)*/
{
	while (CompVDrawing::isLoopRunning() && !m_ptrWindow->isClosed()) {
		COMPV_CHECK_CODE_RETURN(m_ptrWindow->beginDraw());
		COMPV_CHECK_CODE_RETURN(m_ptrSingleSurfaceLayer->surface()->drawImage(m_ptrImage));
		COMPV_CHECK_CODE_RETURN(m_ptrSingleSurfaceLayer->blit());
		COMPV_CHECK_CODE_RETURN(m_ptrWindow->endDraw());
	}
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVMyRunLoopListener::newObj(CompVMyRunLoopListenerPtrPtr listener, CompVWindowPtr window, CompVMatPtr image)
{
	COMPV_CHECK_EXP_RETURN(!listener || !window || !image, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	COMPV_CHECK_EXP_RETURN(!(*listener = new CompVMyRunLoopListener(window, image)), COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	return COMPV_ERROR_CODE_S_OK;
}

