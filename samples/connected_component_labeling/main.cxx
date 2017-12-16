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

#define CCL_ID				COMPV_PLSL_ID // Parallel Light Speed Labeling

static const compv_float32x4_t __color_red = { 1.f, 0.f, 0.f, 1.f };
static const compv_float32x4_t __color_green = { 0.f, 1.f, 0.f, 1.f };
static const compv_float32x4_t __color_blue = { 0.f, 0.f, 1.f, 1.f };
static const compv_float32x4_t __color_yellow = { 1.f, 1.f, 0.f, 1.f };
static const compv_float32x4_t __color_tomato = { 1.f, .388f, .278f, 1.f };
static const compv_float32x4_t __color_violet = { .933f, .509f, .933f, 1.f };
static const compv_float32x4_t* __colors[] = { &__color_red , &__color_green , &__color_blue, &__color_yellow, &__color_tomato, &__color_violet };
static const size_t __colors_count = sizeof(__colors) / sizeof(__colors[0]);

#define TAG_SAMPLE			"Connected Component Labeling"

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
		CompVDrawingOptions drawingOptions;
		CompVMatPtr imageBinar, blob;
		CompVConnectedComponentLabelingResultPtr ccl_result;
		CompVConnectedComponentLabelingPtr ccl_obj;
		CompVCameraCallbackOnNewFrame funcPtrOnNewFrame;
		double threshold = 0.0;

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Set drawing options
		drawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_STATIC;
		drawingOptions.lineWidth = 1.f;
		drawingOptions.fontSize = 12;
		drawingOptions.pointSize = 1.f;

		// Create window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));

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

		// Create the ccl and set default settings
		COMPV_CHECK_CODE_RETURN(CompVConnectedComponentLabeling::newObj(&ccl_obj, CCL_ID));
		if (CCL_ID == COMPV_PLSL_ID) {
			COMPV_CHECK_CODE_RETURN(ccl_obj->setInt(COMPV_PLSL_SET_INT_TYPE, COMPV_PLSL_TYPE_STD));
		}

		// Add 'OnNewFrame' callback to the camera
		funcPtrOnNewFrame = [&](const CompVMatPtr& image) -> COMPV_ERROR_CODE {
			COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
			CompVCanvasPtr canvas;
			CompVMatPtrVector points;
			size_t color_index = 0;
			if (CompVDrawing::isLoopRunning()) {
				COMPV_CHECK_CODE_RETURN(CompVImage::convertGrayscale(image, &imageBinar));
				//COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1285, 1285, 1285, "C:/Projects/GitHub/data/morpho/diffract_1285x1285_gray.yuv", &imageBinar));
				COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 1285, 803, 1285, "C:/Projects/GitHub/data/morpho/dummy_1285x803_gray.yuv", &imageBinar));
				//COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 800, 600, 800, "C:/Projects/GitHub/data/morpho/labyrinth_800x600_gray.yuv", &imageBinar));
				//COMPV_CHECK_CODE_RETURN(CompVImage::readPixels(COMPV_SUBTYPE_PIXELS_Y, 800, 600, 800, "C:/Projects/GitHub/data/morpho/checker_800x600_gray.yuv", &imageBinar));
				COMPV_CHECK_CODE_RETURN(CompVImageThreshold::otsu(imageBinar, threshold, &imageBinar));
				COMPV_CHECK_CODE_RETURN(ccl_obj->process(imageBinar, &ccl_result));
				COMPV_CHECK_CODE_RETURN(ccl_result->extract(points, COMPV_CCL_EXTRACT_TYPE_CONTOUR));

				COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->cover()->drawImage(imageBinar));
				canvas = singleSurfaceLayer->cover()->renderer()->canvas();
				// FIXME(dmi): draw all the vector once
				COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Draw all points (vector) once");
				for (CompVMatPtrVector::const_iterator i = points.begin(); i < points.end(); ++i) {
					drawingOptions.setColor(*__colors[color_index++ % __colors_count]);
					COMPV_CHECK_CODE_BAIL(err = canvas->drawPoints(*i, &drawingOptions));
				}
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->blit());
			bail:
				COMPV_CHECK_CODE_NOP(err = window->endDraw()); // Make sure 'endDraw()' will be called regardless the result
			}
			//getchar();
			return err;
		};
		COMPV_CHECK_CODE_BAIL(err = camera->setCallbackOnNewFrame(funcPtrOnNewFrame));

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
