#include <compv/compv_api.h>

using namespace compv;

#define CAMERA_IDX			0
#define CAMERA_WIDTH		1280
#define CAMERA_HEIGHT		720
#define CAMERA_FPS			25
#define CAMERA_SUBTYPE		COMPV_SUBTYPE_PIXELS_YUY2
#define CAMERA_AUTOFOCUS	true

#define WINDOW_WIDTH		1280
#define WINDOW_HEIGHT		720

#define FAST_NONMAXIMA		true
#define FAST_THRESHOLD		20
#define FAST_TYPE			COMPV_FAST_TYPE_9
#define FAST_MAXFEATURES	-1 // <= 0 means all points (do not retain best)

#define TAG_SAMPLE			"Features Detection App"

/* My runloop listener (optional) */
COMPV_OBJECT_DECLARE_PTRS(MyRunLoopListener)
class CompVMyRunLoopListener : public CompVRunLoopListener
{
protected:
	CompVMyRunLoopListener(CompVCameraPtr camera, const std::string& cameraId) : m_ptrCamera(camera), m_strCameraId(cameraId) { }
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
			return m_ptrCamera->start(m_strCameraId);
		case COMPV_RUNLOOP_STATE_ANIMATION_PAUSED:
		case COMPV_RUNLOOP_STATE_LOOP_STOPPED:
		case COMPV_RUNLOOP_STATE_ANIMATION_STOPPED:
			return m_ptrCamera->stop();
		}
	}
	static COMPV_ERROR_CODE newObj(CompVMyRunLoopListenerPtrPtr listener, CompVCameraPtr camera, const std::string& cameraId) {
		COMPV_CHECK_EXP_RETURN(!listener || !camera, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		*listener = new CompVMyRunLoopListener(camera, cameraId);
		COMPV_CHECK_EXP_RETURN(!*listener, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		return COMPV_ERROR_CODE_S_OK;
	}
private:
	CompVCameraPtr m_ptrCamera;
	const std::string m_strCameraId;
};

/* My camera listener */
COMPV_OBJECT_DECLARE_PTRS(MyCameraListener)
class CompVMyCameraListener : public CompVCameraListener
{
protected:
	CompVMyCameraListener(CompVWindowPtr ptrWindow, CompVSingleSurfaceLayerPtr ptrSingleSurfaceLayer)
		: m_ptrWindow(ptrWindow), m_ptrSingleSurfaceLayer(ptrSingleSurfaceLayer) { }
public:
	virtual ~CompVMyCameraListener() { }

	virtual COMPV_ERROR_CODE onNewFrame(const CompVMatPtr& image) override {
		COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
		// TODO(dmi): 'drawImage' is a pure GPU function while 'FAST->process' is a pure CPU function -> can be done on parallel
		if (CompVDrawing::isLoopRunning()) {
			COMPV_CHECK_CODE_BAIL(err = m_ptrWindow->beginDraw());
			COMPV_CHECK_CODE_BAIL(err = m_ptrSingleSurfaceLayer->surface()->drawImage(image));
			COMPV_CHECK_CODE_BAIL(err = m_ptrFAST->process(image, m_vecInterestPoints));
			COMPV_CHECK_CODE_BAIL(err = m_ptrSingleSurfaceLayer->surface()->renderer()->canvas()->drawInterestPoints(m_vecInterestPoints)); // TOD(dmi): canvas should be at surface()
			COMPV_CHECK_CODE_BAIL(err = m_ptrSingleSurfaceLayer->blit());
		bail:
			COMPV_CHECK_CODE_NOP(err = m_ptrWindow->endDraw()); // Make sure 'endDraw()' will be called regardless the result
		}
		return err;
	}

	virtual COMPV_ERROR_CODE onError(const std::string& message) override {
		COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "Camera error: %s", message.c_str()); // probably a disconnect
		return COMPV_ERROR_CODE_S_OK;
	}

	static COMPV_ERROR_CODE newObj(CompVMyCameraListenerPtrPtr listener, CompVWindowPtr ptrWindow, CompVSingleSurfaceLayerPtr ptrSingleSurfaceLayer) {
		COMPV_CHECK_EXP_RETURN(!listener || !ptrWindow || !ptrSingleSurfaceLayer, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
		*listener = new CompVMyCameraListener(ptrWindow, ptrSingleSurfaceLayer);
		COMPV_CHECK_EXP_RETURN(!*listener, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

		COMPV_CHECK_CODE_RETURN(CompVCornerDete::newObj(&(*listener)->m_ptrFAST, COMPV_FAST_ID));
		COMPV_CHECK_CODE_RETURN((*listener)->m_ptrFAST->setInt(COMPV_FAST_SET_INT_THRESHOLD, FAST_THRESHOLD));
		COMPV_CHECK_CODE_RETURN((*listener)->m_ptrFAST->setInt(COMPV_FAST_SET_INT_FAST_TYPE, FAST_TYPE));
		COMPV_CHECK_CODE_RETURN((*listener)->m_ptrFAST->setInt(COMPV_FAST_SET_INT_MAX_FEATURES, FAST_MAXFEATURES));
		COMPV_CHECK_CODE_RETURN((*listener)->m_ptrFAST->setBool(COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP, FAST_NONMAXIMA));
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	CompVWindowPtr m_ptrWindow;
	CompVSingleSurfaceLayerPtr m_ptrSingleSurfaceLayer;
	CompVCornerDetePtr m_ptrFAST;
	CompVInterestPointVector m_vecInterestPoints;
};

/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err;
		CompVWindowPtr window;
		CompVMyRunLoopListenerPtr runloopListener;
		CompVSingleSurfaceLayerPtr singleSurfaceLayer;
		CompVCameraPtr camera;
		CompVMyCameraListenerPtr cameraListener;
		CompVCameraDeviceInfoList devices;
		std::string cameraId = ""; // empty string means default

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Create "Hello world!" window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));

		// Create my camera and add a listener to it 
		COMPV_CHECK_CODE_BAIL(err = CompVCamera::newObj(&camera));

		COMPV_CHECK_CODE_BAIL(err = CompVMyCameraListener::newObj(&cameraListener, window, *singleSurfaceLayer));
		COMPV_CHECK_CODE_BAIL(err = camera->setListener(*cameraListener));
		// Get list of devices/cameras and print them to the screen (optional)
		COMPV_CHECK_CODE_BAIL(err = camera->devices(devices));
		COMPV_CHECK_EXP_BAIL(devices.empty(), err = COMPV_ERROR_CODE_E_NOT_FOUND, "No camera device found");
		for (CompVCameraDeviceInfoList::iterator it = devices.begin(); it != devices.end(); ++it) {
			COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "Camera device: %s -> %s, %s", it->id.c_str(), it->name.c_str(), it->description.c_str());
		}
		// Set camera parameters (optional)
		cameraId = devices[COMPV_MATH_MIN(CAMERA_IDX, (devices.size() - 1))].id;
		COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_WIDTH, CAMERA_WIDTH));
		COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_HEIGHT, CAMERA_HEIGHT));
		COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_FPS, CAMERA_FPS));
		COMPV_CHECK_CODE_BAIL(err = camera->setInt(COMPV_CAMERA_CAP_INT_SUBTYPE, CAMERA_SUBTYPE));
		COMPV_CHECK_CODE_BAIL(err = camera->setBool(COMPV_CAMERA_CAP_BOOL_AUTOFOCUS, CAMERA_AUTOFOCUS));
		COMPV_CHECK_CODE_BAIL(err = camera->start(cameraId)); // use no parameter ('star()') to use default camera device

		// Start ui runloop
		// Setting a listener is optional but used here to show how to handle Android onStart, onPause, onResume.... activity states
		COMPV_CHECK_CODE_BAIL(err = CompVMyRunLoopListener::newObj(&runloopListener, camera, cameraId));
		COMPV_CHECK_CODE_BAIL(err = CompVDrawing::runLoop(*runloopListener));

	bail:
		if (COMPV_ERROR_CODE_IS_NOK(err)) {
			COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "Something went wrong!!");
		}
	}

	COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS();

	compv_main_return(0);
}
