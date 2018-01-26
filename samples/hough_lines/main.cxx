#include <compv/compv_api.h>

using namespace compv;

#define CAMERA_IDX				0
#define CAMERA_WIDTH			1280
#define CAMERA_HEIGHT			720
#define CAMERA_FPS				25
#define CAMERA_SUBTYPE			COMPV_SUBTYPE_PIXELS_YUY2
#define CAMERA_AUTOFOCUS		true

#define WINDOW_WIDTH			1280
#define WINDOW_HEIGHT			720

#define CANNY_THRESHOLD_TYPE	COMPV_CANNY_THRESHOLD_TYPE_COMPARE_TO_GRADIENT

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

/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err;
		CompVWindowPtr window;
		CompVSingleSurfaceLayerPtr singleSurfaceLayer;
		CompVCameraPtr camera;
		CompVCameraDeviceInfoList devices;
		CompVEdgeDetePtr ptrCanny;
		CompVHoughPtr ptrHough;
		CompVDrawingOptions drawingOptions;
		std::string cameraId = ""; // empty string means default

		CompVMatPtr imageGray, edges;
		CompVHoughLineVector linesPolor;
		CompVLineFloat32Vector linesCartesian;
		double threshold;

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Create "Hello world!" window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));

		// Create Canny edge detector
		COMPV_CHECK_CODE_BAIL(err = CompVEdgeDete::newObj(&ptrCanny, COMPV_CANNY_ID));
		COMPV_CHECK_CODE_BAIL(err = ptrCanny->setInt(COMPV_CANNY_SET_INT_THRESHOLD_TYPE, CANNY_THRESHOLD_TYPE));

		// Create HoughLines processor and set options
		COMPV_CHECK_CODE_BAIL(err = CompVHough::newObj(&ptrHough, HOUGH_ID,
			(HOUGH_ID == COMPV_HOUGHKHT_ID) ? HOUGH_RHO : 1.f, // SHT doesn't support fractional rho values (only 1.f is supported)
			HOUGH_THETA,
			(HOUGH_ID == COMPV_HOUGHKHT_ID) ? HOUGHKHT_THRESHOLD : HOUGH_THRESHOLD));
		COMPV_CHECK_CODE_BAIL(err = ptrHough->setInt(COMPV_HOUGH_SET_INT_MAXLINES, HOUGH_MAXLINES));
		if (HOUGH_ID == COMPV_HOUGHKHT_ID) {
			COMPV_CHECK_CODE_BAIL(err = ptrHough->setFloat32(COMPV_HOUGHKHT_SET_FLT32_CLUSTER_MIN_DEVIATION, HOUGHKHT_CLUSTER_MIN_DEVIATION));
			COMPV_CHECK_CODE_BAIL(err = ptrHough->setInt(COMPV_HOUGHKHT_SET_INT_CLUSTER_MIN_SIZE, HOUGHKHT_CLUSTER_MIN_SIZE));
			COMPV_CHECK_CODE_BAIL(err = ptrHough->setFloat32(COMPV_HOUGHKHT_SET_FLT32_KERNEL_MIN_HEIGTH, HOUGHKHT_KERNEL_MIN_HEIGTH));
		}

		// Set Drawing options
		drawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_STATIC;
		drawingOptions.color[0] = 1.f;
		drawingOptions.color[1] = 1.f;
		drawingOptions.color[2] = 0.f;
		drawingOptions.color[3] = 1.0f;
		drawingOptions.lineWidth = 1.5f;

		// Create my camera and add a listener to it 
		COMPV_CHECK_CODE_BAIL(err = CompVCamera::newObj(&camera));
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

		// Add 'OnNewFrame' callback to the camera
		COMPV_CHECK_CODE_BAIL(err = camera->setCallbackOnNewFrame([&](const CompVMatPtr& image) -> COMPV_ERROR_CODE {
			COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
			if (CompVDrawing::isLoopRunning()) {
				COMPV_CHECK_CODE_RETURN(CompVImage::convertGrayscale(image, &imageGray));
				COMPV_CHECK_CODE_RETURN(CompVImage::thresholdOtsu(imageGray, threshold));
				COMPV_CHECK_CODE_RETURN(ptrCanny->setFloat32(COMPV_CANNY_SET_FLT32_THRESHOLD_LOW, static_cast<compv_float32_t>(threshold * 0.5)));
				COMPV_CHECK_CODE_RETURN(ptrCanny->setFloat32(COMPV_CANNY_SET_FLT32_THRESHOLD_HIGH, static_cast<compv_float32_t>(threshold)));
				COMPV_CHECK_CODE_RETURN(ptrCanny->process(imageGray, &edges));

				COMPV_CHECK_CODE_RETURN(ptrHough->process(edges, linesPolor));
				COMPV_CHECK_CODE_RETURN(ptrHough->toCartesian(edges->cols(), edges->rows(), linesPolor, linesCartesian));

				// Drawing
				COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->cover()->drawImage(edges));
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->cover()->renderer()->canvas()->drawLines(linesCartesian, &drawingOptions));
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->blit());
			bail:
				COMPV_CHECK_CODE_NOP(err = window->endDraw()); // Make sure 'endDraw()' will be called regardless the result
			}
			return err;
		}));

		// Add 'OnError' callback to the camera
		COMPV_CHECK_CODE_BAIL(err = camera->setCallbackOnError([&](const std::string& message) -> COMPV_ERROR_CODE {
			COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "Camera error: %s", message.c_str()); // probably a disconnect
			return COMPV_ERROR_CODE_S_OK;
		}));

		// Start ui runloop
		// Setting a listener is optional but used here to show how to handle Android onStart, onPause, onResume.... activity states
		COMPV_CHECK_CODE_BAIL(err = CompVDrawing::runLoop([&](const COMPV_RUNLOOP_STATE& newState) -> COMPV_ERROR_CODE {
			COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "RunLoop onStateChanged(%d)", newState);
			switch (newState) {
			case COMPV_RUNLOOP_STATE_LOOP_STARTED:
			default:
				return COMPV_ERROR_CODE_S_OK;
			case COMPV_RUNLOOP_STATE_ANIMATION_STARTED:
			case COMPV_RUNLOOP_STATE_ANIMATION_RESUMED:
				return camera->start(cameraId); // start camera
			case COMPV_RUNLOOP_STATE_ANIMATION_PAUSED:
			case COMPV_RUNLOOP_STATE_LOOP_STOPPED:
			case COMPV_RUNLOOP_STATE_ANIMATION_STOPPED:
				return camera->stop(); // stop camera
			}
		}));

	bail:
		if (COMPV_ERROR_CODE_IS_NOK(err)) {
			COMPV_DEBUG_ERROR_EX(TAG_SAMPLE, "Something went wrong!!");
		}
	}

	COMPV_DEBUG_CHECK_FOR_MEMORY_LEAKS();

	compv_main_return(0);
}
