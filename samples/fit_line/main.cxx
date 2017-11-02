#include <compv/compv_api.h>
#include <random>

using namespace compv;

#define WINDOW_WIDTH		1280
#define WINDOW_HEIGHT		720

#define NUM_NOISY_POINTS	1300
#define NUM_OUTLIERS		200

static const compv_float32x4_t __color_red = { 1.f, 0.f, 0.f, 1.f };
static const compv_float32x4_t __color_black = { 0.f, 0.f, 0.f, 1.f };
static const compv_float32x4_t __color_yellow = { 1.f, 1.f, 0.f, 1.f };
static const compv_float32x4_t __color_blue = { 0.f, 0.f, 1.f, 1.f };

#define __color_background		__color_black
#define __color_points			__color_blue
#define __color_line_perfect	__color_yellow
#define __color_line_fitted		__color_red

#define TAG_SAMPLE	"Fit line"

static void __build_random_points(
	const  compv_float32_t window_width, const  compv_float32_t window_height, 
	const compv_float32_t rho, const compv_float32_t theta_deg,
	CompVLineFloat32& linePerfect,
	CompVPointFloat32Vector& pointsNoisy
) 
{
	const compv_float32_t theta_rad = COMPV_MATH_DEGREE_TO_RADIAN_FLOAT(theta_deg);
	if (theta_rad == 0.f) {
		linePerfect.a.x = 0;
		linePerfect.a.y = rho;
		linePerfect.b.x = window_width;
		linePerfect.b.y = -rho;
	}
	else {
		const compv_float32_t a = (std::cos(theta_rad) * window_width);
		const compv_float32_t b = (1.f / std::sin(theta_rad));
		linePerfect.a.x = 0;
		linePerfect.a.y = ((rho + a) * b) + window_height;
		linePerfect.b.x = window_width;
		linePerfect.b.y = ((rho - a) * b) + window_height;
	}

	pointsNoisy.clear();

	const compv_float32_t slope = (linePerfect.b.y - linePerfect.a.y) / (linePerfect.b.x - linePerfect.a.x);
	const compv_float32_t intercept = linePerfect.a.y - (slope * linePerfect.a.x);
	const compv_float32_t slope_scale = 1.f / slope;

	const int window_height_int = static_cast<int>(window_height);
	const int window_width_int = static_cast<int>(window_width);

	// Perfect points
	pointsNoisy.resize(NUM_NOISY_POINTS + NUM_OUTLIERS);
	for (CompVPointFloat32Vector::iterator i = pointsNoisy.begin(); i < pointsNoisy.end(); ++i) {
		i->y = static_cast<compv_float32_t>(rand() % window_height_int);
		i->x = (i->y - intercept) * slope_scale;
	}

	// Add gaussian noise
	const compv_float32_t mean = 0.f;
	const compv_float32_t stddev = 8.3f;
	std::default_random_engine generator;
	std::normal_distribution<compv_float32_t> dist(mean, stddev);
	for (CompVPointFloat32Vector::iterator i = pointsNoisy.begin(); i < pointsNoisy.end(); ++i) {
		i->x += dist(generator);
		i->y += dist(generator);
	}

	// Now add 100 outliers
	for (CompVPointFloat32Vector::iterator i = (pointsNoisy.begin() + NUM_NOISY_POINTS); i < pointsNoisy.end(); ++i) {
		i->x += static_cast<compv_float32_t>((rand() % window_width_int) * ((rand() & 1) ? -1 : 1));
		i->y += static_cast<compv_float32_t>((rand() % window_height_int) * ((rand() & 1) ? -1 : 1));
	}
}

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
		CompVPointFloat32Vector pointsNoisy;
		CompVLineFloat32Vector linesPerfect(1);
		CompVLineFloat32Vector linesFitted(1);

		// Change debug level to INFO before starting
		CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);

		// Init the modules
		COMPV_CHECK_CODE_BAIL(err = CompVInit());

		// Drawing options
		drawingOptions.colorType = COMPV_DRAWING_COLOR_TYPE_STATIC;
		drawingOptions.pointSize = 2.f;
		drawingOptions.fontSize = 12;
		drawingOptions.lineWidth = 4.f;

		// Create window and add a surface for drawing
		COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, WINDOW_WIDTH, WINDOW_HEIGHT, TAG_SAMPLE));
		COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));

		// Create Runnable and execute the task (lane detection)
		onRunning = [&]() -> COMPV_ERROR_CODE {
			compv_float32_t theta = 0.f;
			compv_float32_t rho = 0.f;
			while (CompVDrawing::isLoopRunning()) {
				__build_random_points(
					static_cast<compv_float32_t>(window->width()), static_cast<compv_float32_t>(window->height()),
					std::fmodf((rho += 0.5f), static_cast<compv_float32_t>(window->width())) * ((rand() & 1) ? -1.f : 1.f), 
					std::fmodf((theta += 0.1f), 360.f) * ((rand() & 1) ? -1.f : 1.f),
					linesPerfect[0],
					pointsNoisy
				);
				COMPV_CHECK_CODE_BAIL(window->beginDraw());
				COMPV_CHECK_EXP_BAIL(!(canvas = singleSurfaceLayer->cover()->requestCanvas()), COMPV_ERROR_CODE_E_INVALID_CALL, "Cannot create a canvas for the cover");
				drawingOptions.setColor(__color_background);
				COMPV_CHECK_CODE_BAIL(canvas->clear(&drawingOptions));
				drawingOptions.setColor(__color_line_perfect);
				COMPV_CHECK_CODE_BAIL(canvas->drawLines(linesPerfect, &drawingOptions));
				drawingOptions.setColor(__color_line_fitted);
				COMPV_CHECK_CODE_BAIL(canvas->drawLines(linesFitted, &drawingOptions));
				drawingOptions.setColor(__color_points);
				COMPV_CHECK_CODE_BAIL(canvas->drawPoints(pointsNoisy, &drawingOptions));
				COMPV_CHECK_CODE_BAIL(singleSurfaceLayer->blit());
			bail:
				COMPV_CHECK_CODE_NOP(window->endDraw()); // Make sure 'endDraw()' will be called regardless the result
				COMPV_DEBUG_INFO_CODE_FOR_TESTING("Remove the sleep function");
				CompVThread::sleep(100); // FIXME(dmi): remove
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