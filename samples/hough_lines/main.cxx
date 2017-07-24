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

#define CANNY_LOW			0.83f
#define CANNY_HIGH			CANNY_LOW*2.f
#define CANNY_KERNEL_SIZE	3

#define HOUGH_ID							COMPV_HOUGHKHT_ID
#define HOUGH_RHO							1.0f // "rho-delta" (half-pixel for more accuracy - high cpu usage)
#define HOUGH_THETA							0.5f // "theta-delta" (half-radian for more accuracy - high cpu usage)
#define HOUGH_THRESHOLD						150 // minumum number of aligned points to form a line (also used in NMS)
#define HOUGH_MAXLINES						20 // maximum number of lines to retains (best) - use value <=0 to retain all
#define HOUGHKHT_THRESHOLD					1 // keep all votes and filter later using MAXLINES
#define HOUGHKHT_CLUSTER_MIN_DEVIATION		2.0f
#define HOUGHKHT_CLUSTER_MIN_SIZE			10
#define HOUGHKHT_KERNEL_MIN_HEIGTH			0.002f


#define TAG_SAMPLE			"Hough lines detector"

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
		if (CompVDrawing::isLoopRunning()) {
			CompVMatPtr imageGray, edges;
			CompVHoughLineVector linesPolor;
			CompVLineFloat32Vector linesCartesian;
#if 0
			COMPV_CHECK_CODE_RETURN(CompVImage::convertGrayscale(image, &imageGray));
			COMPV_CHECK_CODE_RETURN(m_ptrCanny->process(imageGray, &edges));
#elif 1
#	if COMPV_OS_ANDROID || TARGET_OS_IPHONE
			COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1020, 960, 1020, COMPV_PATH_FROM_NAME("road_binary1020x960_gray.yuv"), &edges));
#	else
			COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1020, 960, 1020, "C:/Projects/GitHub/data/hough/road_binary1020x960_gray.yuv", &edges));
#	endif
#elif 1
			COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1280, 738, 1280, "C:/Projects/GitHub/data/adas/vlcsnap-2016-07-13-22h51m40s373_1280x738_gray.yuv", &imageGray));
			COMPV_CHECK_CODE_RETURN(m_ptrCanny->process(imageGray, &edges));
#else
#			error "Not implemented"
#endif		
			COMPV_CHECK_CODE_RETURN(m_ptrHough->process(edges, linesPolor));
			COMPV_CHECK_CODE_RETURN(m_ptrHough->toCartesian(edges->cols(), edges->rows(), linesPolor, linesCartesian));
			
			COMPV_CHECK_CODE_BAIL(err = m_ptrWindow->beginDraw());
			COMPV_CHECK_CODE_BAIL(err = m_ptrSingleSurfaceLayer->surface()->drawImage(edges));
			COMPV_CHECK_CODE_BAIL(err = m_ptrSingleSurfaceLayer->surface()->renderer()->canvas()->drawLines(linesCartesian, &m_DrawingOptions));
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
		CompVMyCameraListenerPtr listener_ = new CompVMyCameraListener(ptrWindow, ptrSingleSurfaceLayer);
		COMPV_CHECK_EXP_RETURN(!listener_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
		COMPV_CHECK_CODE_RETURN(CompVEdgeDete::newObj(&listener_->m_ptrCanny, COMPV_CANNY_ID, CANNY_LOW, CANNY_HIGH, CANNY_KERNEL_SIZE));
		COMPV_CHECK_CODE_RETURN(CompVHough::newObj(&listener_->m_ptrHough, HOUGH_ID, 
			(HOUGH_ID == COMPV_HOUGHKHT_ID) ? HOUGH_RHO : 1.f, // SHT doesn't support fractional rho values (only 1.f is supported)
			HOUGH_THETA, 
			(HOUGH_ID == COMPV_HOUGHKHT_ID) ? HOUGHKHT_THRESHOLD : HOUGH_THRESHOLD));
		COMPV_CHECK_CODE_RETURN(listener_->m_ptrHough->setInt(COMPV_HOUGH_SET_INT_MAXLINES, HOUGH_MAXLINES));
		if (HOUGH_ID == COMPV_HOUGHKHT_ID) {
			COMPV_CHECK_CODE_RETURN(listener_->m_ptrHough->setFloat32(COMPV_HOUGHKHT_SET_FLT32_CLUSTER_MIN_DEVIATION, HOUGHKHT_CLUSTER_MIN_DEVIATION));
			COMPV_CHECK_CODE_RETURN(listener_->m_ptrHough->setInt(COMPV_HOUGHKHT_SET_INT_CLUSTER_MIN_SIZE, HOUGHKHT_CLUSTER_MIN_SIZE));
			COMPV_CHECK_CODE_RETURN(listener_->m_ptrHough->setFloat32(COMPV_HOUGHKHT_SET_FLT32_KERNEL_MIN_HEIGTH, HOUGHKHT_KERNEL_MIN_HEIGTH));
		}
		listener_->m_DrawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_STATIC;
		listener_->m_DrawingOptions.color[0] = 1.f;
		listener_->m_DrawingOptions.color[1] = 1.f;
		listener_->m_DrawingOptions.color[2] = 0.f;
		listener_->m_DrawingOptions.color[3] = 1.0f;
		listener_->m_DrawingOptions.lineWidth = 1.5f;

		*listener = listener_;
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	CompVWindowPtr m_ptrWindow;
	CompVSingleSurfaceLayerPtr m_ptrSingleSurfaceLayer;
	CompVEdgeDetePtr m_ptrCanny;
	CompVHoughPtr m_ptrHough;
	CompVDrawingOptions m_DrawingOptions;
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
