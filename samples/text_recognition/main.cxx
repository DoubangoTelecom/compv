#include <compv/compv_api.h>

using namespace compv;

#if COMPV_OS_WINDOWS
#	define IMAGE_FOLDER				"C:/Projects/GitHub/data/morpho"
#elif COMPV_OS_OSX
#	define IMAGE_FOLDER				"/Users/mamadou/Projects/GitHub/data/morpho"
#else
#	define IMAGE_FOLDER				NULL
#endif

#define WINDOW_WIDTH				1280
#define WINDOW_HEIGHT				720

#define IMAGE_FILE_NAME				"text_1122x1182_gray.yuv"
#define IMAGE_WIDTH					1122
#define IMAGE_HEIGHT				1182

#define THRESHOLDING_KERNEL_SIZE	5
#define THRESHOLDING_DELTA			+4

#define MORPH_STREL_SIZE			CompVSizeSz(3, 3)
#define MORPH_STREL_TYPE			COMPV_MATH_MORPH_STREL_TYPE_DIAMOND
#define MORPH_BORDER_TYPE			COMPV_BORDER_TYPE_REPLICATE
#define MORPH_OPERATOR				COMPV_MATH_MORPH_OP_TYPE_CLOSE

#define TAG_SAMPLE			"Text recognition"

typedef std::function<COMPV_ERROR_CODE()> FuncPtr;

/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err;
		CompVWindowPtr window;
		CompVSingleSurfaceLayerPtr singleSurfaceLayer;
		std::string path;
		CompVMatPtr image, structuringElement;
		FuncPtr funcPtrProcessStart, funcPtrProcessStop;

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Create "Hello world!" window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));

		// Read image from file
		path = COMPV_PATH_FROM_NAME(IMAGE_FILE_NAME); // path from android's assets, iOS' bundle....
		// The path isn't correct when the binary is loaded from another process(e.g. when Intel VTune is used)
		if (!CompVFileUtils::exists(path.c_str())) {
			path = std::string(IMAGE_FOLDER) + std::string("/") + std::string(IMAGE_FILE_NAME);
		}
		COMPV_CHECK_CODE_BAIL(err = CompVImage::readPixels(
			COMPV_SUBTYPE_PIXELS_Y,
			IMAGE_WIDTH,
			IMAGE_HEIGHT,
			IMAGE_WIDTH,
			path.c_str(),
			&image
		));
		
		// Inversed adaptive thresholding. Inversed to swap back<->white (text became white and background back)
		COMPV_CHECK_CODE_BAIL(err = CompVImageThreshold::adaptive(image, &image, THRESHOLDING_KERNEL_SIZE, THRESHOLDING_DELTA, 255, true));

		// Composite morphological operation: close
		COMPV_CHECK_CODE_BAIL(err = CompVMathMorph::buildStructuringElement(&structuringElement, MORPH_STREL_SIZE, MORPH_STREL_TYPE));
		COMPV_CHECK_CODE_RETURN(CompVMathMorph::process(image, structuringElement, &image, MORPH_OPERATOR, MORPH_BORDER_TYPE));

		funcPtrProcessStart = [&]() -> COMPV_ERROR_CODE {
			COMPV_ERROR_CODE err = COMPV_ERROR_CODE_S_OK;
			double delta = 0;
			
			while (CompVDrawing::isLoopRunning()) {
				// Put here something you want to before drawing each frame to the screen

				// Drawing
				COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->cover()->drawImage(image));
				COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->blit());
			bail:
				COMPV_CHECK_CODE_NOP(err = window->endDraw()); // Make sure 'endDraw()' will be called regardless the result
			}
			return err;
		};

		funcPtrProcessStop = [&]() -> COMPV_ERROR_CODE {
			return COMPV_ERROR_CODE_S_OK;
		};

		/* Start ui runloop */
		// Setting a listener is optional but used here to show how to handle Android onStart, onPause, onResume.... activity states
		COMPV_CHECK_CODE_BAIL(err = CompVDrawing::runLoop([&](const COMPV_RUNLOOP_STATE& newState) -> COMPV_ERROR_CODE {
			COMPV_DEBUG_INFO_EX(TAG_SAMPLE, "RunLoop onStateChanged(%d)", newState);
			switch (newState) {
			case COMPV_RUNLOOP_STATE_LOOP_STARTED:
			default:
				return COMPV_ERROR_CODE_S_OK;
			case COMPV_RUNLOOP_STATE_ANIMATION_STARTED:
			case COMPV_RUNLOOP_STATE_ANIMATION_RESUMED:
				return funcPtrProcessStart();
			case COMPV_RUNLOOP_STATE_ANIMATION_PAUSED:
			case COMPV_RUNLOOP_STATE_LOOP_STOPPED:
			case COMPV_RUNLOOP_STATE_ANIMATION_STOPPED:
				return funcPtrProcessStop();
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
