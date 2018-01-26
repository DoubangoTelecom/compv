#include <compv/compv_api.h>

using namespace compv;

#define CAMERA_IDX			0
#define CAMERA_WIDTH		1280
#define CAMERA_HEIGHT		720
#define CAMERA_FPS			30
#define CAMERA_SUBTYPE		COMPV_SUBTYPE_PIXELS_YUY2
#define CAMERA_AUTOFOCUS	true

#define WINDOW_WIDTH		1280
#define WINDOW_HEIGHT		720

#define FAST_NONMAXIMA		true
#define FAST_THRESHOLD		20
#define FAST_TYPE			COMPV_FAST_TYPE_9
#define FAST_MAXFEATURES	2000 // use negative value to retain all features (more cpu usage!!)

#define TAG_SAMPLE			"Features Detection App"

/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err;
		CompVWindowPtr window;
		CompVSingleSurfaceLayerPtr singleSurfaceLayer;
		CompVCameraPtr camera;
		CompVCameraDeviceInfoList devices;
		std::string cameraId = ""; // empty string means default
		CompVCornerDetePtr ptrFAST;
		CompVInterestPointVector vecInterestPoints;
		CompVMatPtr imageGray;

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Create "Hello world!" window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));

		// Create FAST feature detector and set the options
		COMPV_CHECK_CODE_BAIL(err = CompVCornerDete::newObj(&ptrFAST, COMPV_FAST_ID));
		COMPV_CHECK_CODE_BAIL(err = ptrFAST->setInt(COMPV_FAST_SET_INT_THRESHOLD, FAST_THRESHOLD));
		COMPV_CHECK_CODE_BAIL(err = ptrFAST->setInt(COMPV_FAST_SET_INT_FAST_TYPE, FAST_TYPE));
		COMPV_CHECK_CODE_BAIL(err = ptrFAST->setInt(COMPV_FAST_SET_INT_MAX_FEATURES, FAST_MAXFEATURES));
		COMPV_CHECK_CODE_BAIL(err = ptrFAST->setBool(COMPV_FAST_SET_BOOL_NON_MAXIMA_SUPP, FAST_NONMAXIMA));

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
			// TODO(dmi): 'drawImage' is a pure GPU function while 'FAST->process' is a pure CPU function -> can be done on parallel
			if (CompVDrawing::isLoopRunning()) {
				COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->cover()->drawImage(image));
				COMPV_CHECK_CODE_BAIL(err = CompVImage::convertGrayscale(image, &imageGray));
				COMPV_CHECK_CODE_BAIL(err = ptrFAST->process(imageGray, vecInterestPoints));
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->cover()->renderer()->canvas()->drawInterestPoints(vecInterestPoints)); // TODO(dmi): canvas should be at surface()
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
		// Setting a state listener is optional but used here to show how to handle Android onStart, onPause, onResume.... activity states
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
