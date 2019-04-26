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

#define DELTA			2
#define MIN_AREA		(0.0055 * 0.0055)
#define MAX_AREA		(0.8 * 0.15)
#define MAX_VARIATION	0.3
#define MIN_DIVERSITY	0.2
#define CONNECTIVITY	8

#define TAG_SAMPLE			"MSER Detection App"

static COMPV_ERROR_CODE drawBoundingBoxes(const CompVConnectedComponentLabelingResultPtr ccl_result, CompVCanvasPtr canvas);

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
		CompVMatPtr imageGray;
		CompVConnectedComponentLabelingPtr ccl_obj;
		CompVConnectedComponentLabelingResultPtr ccl_result;

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Create "Hello world!" window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));

		// Create MSER feature detector and set the options
		COMPV_CHECK_CODE_BAIL(err = CompVConnectedComponentLabeling::newObj(&ccl_obj, COMPV_LMSER_ID,
			DELTA,
			MIN_AREA,
			MAX_AREA,
			MAX_VARIATION,
			MIN_DIVERSITY,
			CONNECTIVITY)
		);

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
			// TODO(dmi): 'drawImage' is a pure GPU function while 'MSER->process' is a pure CPU function -> can be done on parallel
			if (CompVDrawing::isLoopRunning()) {
				COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->cover()->drawImage(image));
				COMPV_CHECK_CODE_BAIL(err = CompVImage::convertGrayscale(image, &imageGray));
				COMPV_CHECK_CODE_BAIL(err = ccl_obj->process(imageGray, &ccl_result));
				COMPV_CHECK_CODE_BAIL(err = drawBoundingBoxes(ccl_result, singleSurfaceLayer->cover()->renderer()->canvas()));
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

static COMPV_ERROR_CODE drawBoundingBoxes(const CompVConnectedComponentLabelingResultPtr ccl_result, CompVCanvasPtr canvas)
{
	const CompVConnectedComponentLabelingResultLMSER* result_lmser =
		CompVConnectedComponentLabeling::reinterpret_castr<CompVConnectedComponentLabelingResultLMSER>(ccl_result);
	const CompVConnectedComponentLabelingRegionMserVector& regions = result_lmser->boundingBoxes();
	const size_t count = regions.size();
	if (count) {
		CompVRectFloat32Vector boxes(count);
		auto funcPtr = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
			for (size_t ii = start; ii < end; ++ii) {
				CompVRectFloat32* bf = &boxes[ii];
				const CompVConnectedComponentBoundingBox* bi = &regions[ii].boundingBox;
				bf->bottom = static_cast<compv_float32_t>(bi->bottom);
				bf->top = static_cast<compv_float32_t>(bi->top);
				bf->left = static_cast<compv_float32_t>(bi->left);
				bf->right = static_cast<compv_float32_t>(bi->right);
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
			funcPtr,
			1,
			count,
			100
		));
		COMPV_CHECK_CODE_RETURN(canvas->drawRectangles(boxes));
	}
	return COMPV_ERROR_CODE_S_OK;
}
