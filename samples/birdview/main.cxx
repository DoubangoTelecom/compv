#include <compv/compv_api.h>

using namespace compv;

#define WINDOW_WIDTH			1280
#define WINDOW_HEIGHT			720

#if COMPV_OS_WINDOWS
#	define COMPV_SAMPLE_IMAGE_FOLDER			"C:/Projects/GitHub/data/adas"
#elif COMPV_OS_OSX
#	define COMPV_SAMPLE_IMAGE_FOLDER			"/Users/mamadou/Projects/GitHub/data/adas"
#else
#	define COMPV_SAMPLE_IMAGE_FOLDER			NULL
#endif

#define IMAGE_FILE_NAME			"birdview_1920x1090_gray.yuv"
#define IMAGE_WIDTH				1920
#define IMAGE_HEIGHT			1090
#define IMAGE_STRIDE			1920

#define TAG_SAMPLE				"Birdview"

/* IWindowSizeChanged */
class IWindowSizeChanged {
public:
	virtual COMPV_ERROR_CODE onWindowSizeChanged(const size_t newWidth, const size_t newHeight) = 0;
};

/* My window listener */
COMPV_OBJECT_DECLARE_PTRS(MyWindowListener)
class CompVMyWindowListener : public CompVWindowListener {
protected:
	CompVMyWindowListener(const IWindowSizeChanged* pcIWindowSizeChanged) : m_pcIWindowSizeChanged(pcIWindowSizeChanged) { }
public:
	virtual ~CompVMyWindowListener() {

	}
	static COMPV_ERROR_CODE newObj(CompVMyWindowListenerPtrPtr listener, const IWindowSizeChanged* pcIWindowSizeChanged) {
		COMPV_CHECK_EXP_RETURN(!listener || !pcIWindowSizeChanged, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		*listener = new CompVMyWindowListener(pcIWindowSizeChanged);
		COMPV_CHECK_EXP_RETURN(!*listener, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		return COMPV_ERROR_CODE_S_OK;
	}
	virtual COMPV_ERROR_CODE onSizeChanged(size_t newWidth, size_t newHeight) override {
		COMPV_CHECK_CODE_RETURN(const_cast<IWindowSizeChanged*>(m_pcIWindowSizeChanged)->onWindowSizeChanged(newWidth, newHeight));
		return COMPV_ERROR_CODE_S_OK;
	};
	virtual COMPV_ERROR_CODE onStateChanged(COMPV_WINDOW_STATE newState) override {
		return COMPV_ERROR_CODE_S_OK;
	};
private:
	const IWindowSizeChanged* m_pcIWindowSizeChanged;
};

/* My runloop listener (optional) */
COMPV_OBJECT_DECLARE_PTRS(MyRunLoopListener)
class CompVMyRunLoopListener : public CompVRunLoopListener, public IWindowSizeChanged, public CompVLock
{
protected:
	CompVMyRunLoopListener(CompVWindowPtr ptrWindow) : m_ptrWindow(ptrWindow), m_bRunning(false) { }
public:
	virtual ~CompVMyRunLoopListener() { }
	virtual COMPV_ERROR_CODE onStateChanged(COMPV_RUNLOOP_STATE newState) override {
		COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "RunLoop onStateChanged(%d)", newState);
		switch (newState) {
		case COMPV_RUNLOOP_STATE_LOOP_STARTED:
		default:
			return COMPV_ERROR_CODE_S_OK;
		case COMPV_RUNLOOP_STATE_ANIMATION_STARTED:
		case COMPV_RUNLOOP_STATE_ANIMATION_RESUMED:
			m_bRunning = true;
			COMPV_CHECK_CODE_RETURN(drawImage());
			return COMPV_ERROR_CODE_S_OK;
		case COMPV_RUNLOOP_STATE_ANIMATION_PAUSED:
		case COMPV_RUNLOOP_STATE_LOOP_STOPPED:
		case COMPV_RUNLOOP_STATE_ANIMATION_STOPPED:
			m_bRunning = false;
			return COMPV_ERROR_CODE_S_OK;
		}
	}
	static COMPV_ERROR_CODE newObj(CompVMyRunLoopListenerPtrPtr listener, CompVWindowPtr ptrWindow) {
		COMPV_CHECK_EXP_RETURN(!listener || !ptrWindow, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		CompVMyRunLoopListenerPtr runLoopListener_ = new CompVMyRunLoopListener(ptrWindow);
		COMPV_CHECK_EXP_RETURN(!runLoopListener_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		COMPV_CHECK_CODE_RETURN(CompVMyWindowListener::newObj(&runLoopListener_->m_ptrWindowListener, *runLoopListener_));
		COMPV_CHECK_CODE_RETURN(runLoopListener_->m_ptrWindow->addListener(*runLoopListener_->m_ptrWindowListener));
		*listener = runLoopListener_;
		return COMPV_ERROR_CODE_S_OK;
	}
	virtual COMPV_ERROR_CODE onWindowSizeChanged(const size_t newWidth, const size_t newHeight) override {
		COMPV_CHECK_CODE_RETURN(drawImage());
		return COMPV_ERROR_CODE_S_OK;
	}
private:
	COMPV_ERROR_CODE drawImage() {
		CompVAutoLock<CompVMyRunLoopListener>(this);
		if (m_bRunning) {
			if (!m_ptrImage) {
				std::string path = COMPV_PATH_FROM_NAME(IMAGE_FILE_NAME); // path from android's assets, iOS' bundle....
				// The path isn't correct when the binary is loaded from another process(e.g. when Intel VTune is used)
				if (!CompVFileUtils::exists(path.c_str())) {
					path = std::string(COMPV_SAMPLE_IMAGE_FOLDER) + std::string("/") + std::string(IMAGE_FILE_NAME);
				}
				// Build interest points and descriptions
				COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, IMAGE_WIDTH, IMAGE_HEIGHT, IMAGE_STRIDE, path.c_str(), &m_ptrImage));
			}
			if (!m_ptrSurfaceLayer) {
				COMPV_CHECK_CODE_RETURN(m_ptrWindow->addSingleLayerSurface(&m_ptrSurfaceLayer));
			}
			COMPV_ERROR_CODE err;
			COMPV_CHECK_CODE_BAIL(err = m_ptrWindow->beginDraw());
			COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceLayer->surface()->drawImage(m_ptrImage));
			COMPV_CHECK_CODE_BAIL(err = m_ptrSurfaceLayer->blit());
		bail:
			COMPV_CHECK_CODE_NOP(err = m_ptrWindow->endDraw()); // Make sure 'endDraw()' will be called regardless the result
		}
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_ERROR_CODE computeHomography() {
		if (!m_ptrHomography) {

		}
		return COMPV_ERROR_CODE_S_OK;
	}
private:
	bool m_bRunning;
	CompVWindowPtr m_ptrWindow;
	CompVMyWindowListenerPtr m_ptrWindowListener;
	CompVMatPtr m_ptrImage;
	CompVMatPtr m_ptrHomography;
	CompVSingleSurfaceLayerPtr m_ptrSurfaceLayer;
};

/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err;
		CompVMyRunLoopListenerPtr runloopListener;
		CompVWindowPtr m_ptrWindow;

		 // Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Create window
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&m_ptrWindow, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));

		// Start ui runloop
		// Setting a listener is optional but used here to show how to handle Android onStart, onPause, onResume.... activity states
		COMPV_CHECK_CODE_BAIL(err = CompVMyRunLoopListener::newObj(&runloopListener, m_ptrWindow));
		COMPV_CHECK_CODE_BAIL(err = CompVDrawing::runLoop(*runloopListener));

	bail:
		if (COMPV_ERROR_CODE_IS_NOK(err)) {
			COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "Something went wrong!!");
		}
	}

	COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS();

	compv_main_return(0);
}