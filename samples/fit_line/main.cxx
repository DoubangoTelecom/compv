#include <compv/compv_api.h>

using namespace compv;

#define WINDOW_WIDTH		1280
#define WINDOW_HEIGHT		720

static const compv_float32x4_t __color_yellow = { 1.f, 1.f, 0.f, 1.f };
static const compv_float32x4_t __color_blue = { 0.f, 0.f, 1.f, 1.f };

#define TAG_SAMPLE	"Fit line"

/* Entry point function */
compv_main()
{
	{
		COMPV_ERROR_CODE err;
		CompVWindowPtr window;
		CompVSingleSurfaceLayerPtr singleSurfaceLayer;
		CompVMatPtr imageFake;
		CompVRunnablePtr runnable;
		CompVRunnableCallbackOnRunning onRunning;
		CompVDrawingOptions drawingOptions;
		CompVCanvasPtr canvas;

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Drawing options
		drawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_STATIC;
		drawingOptions.pointSize = 7.f;
		drawingOptions.fontSize = 12;
		drawingOptions.lineWidth = 3.f;

		// Create window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));

		// Create Runnable and execute the task (lane detection)
		onRunning = [&]() -> COMPV_ERROR_CODE {
			while (CompVDrawing::isLoopRunning()) {
				CompVLineFloat32 line;
				line.a.x = 0.f;
				line.a.y = 100;
				line.b.x = 1000.f;
				line.b.y = 200;
				CompVLineFloat32Vector lines;
				lines.push_back(line);
				COMPV_CHECK_CODE_BAIL(window->beginDraw());
				COMPV_CHECK_EXP_BAIL(!(canvas = singleSurfaceLayer->cover()->requestCanvas()), COMPV_ERROR_CODE_E_INVALID_CALL, "Cannot create a canvas for the cover");
				COMPV_CHECK_CODE_BAIL(canvas->drawLines(lines, &drawingOptions));
				COMPV_CHECK_CODE_BAIL(singleSurfaceLayer->blit());
			bail:
				COMPV_CHECK_CODE_NOP(window->endDraw()); // Make sure 'endDraw()' will be called regardless the result
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		COMPV_CHECK_CODE_BAIL(err = CompVRunnable::newObj(&runnable, onRunning));

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
				return runnable->start();
			case COMPV_RUNLOOP_STATE_ANIMATION_PAUSED:
			case COMPV_RUNLOOP_STATE_LOOP_STOPPED:
			case COMPV_RUNLOOP_STATE_ANIMATION_STOPPED:
				return runnable->stop();
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