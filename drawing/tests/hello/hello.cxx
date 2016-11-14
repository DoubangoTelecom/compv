#include <compv/compv_api.h>

using namespace compv;

CompVWindowPtr window;

static void* COMPV_STDCALL WorkerThread(void* arg);

compv_main()
{
	COMPV_ERROR_CODE err;

	// Change debug level to INFO before starting
	CompVDebugMgr::setLevel(COMPV_DEBUG_LEVEL_INFO);
	
	// Init the modules
	COMPV_CHECK_CODE_BAIL(err = CompVInit());

	// Create "Hello world!" window
	COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, 640, 480, "Hello world!"));

	// Start ui runloop
	COMPV_CHECK_CODE_BAIL(err = CompVDrawing::runLoop(WorkerThread));

bail:
	if (COMPV_ERROR_CODE_IS_NOK(err)) {
		COMPV_DEBUG_ERROR("Something went wrong!!");
	}

	window = NULL;
	
	// DeInit the modules
	COMPV_CHECK_CODE_ASSERT(err = CompVDeInit());
	// Make sure we freed all allocated memory
	COMPV_ASSERT(CompVMem::isEmpty());
	// Make sure we freed all allocated objects
	COMPV_ASSERT(CompVObj::isEmpty());

	compv_main_return(0);
}

static void* COMPV_STDCALL WorkerThread(void* arg)
{
	COMPV_ERROR_CODE err;
	CompVMatPtr mat[3];
	static int count = 0;
	char buff_[33] = { 0 };
	
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("girl.jpg"), &mat[0]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("valve_original.jpg"), &mat[1]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("mandekalou.jpg"), &mat[2]));

	COMPV_CHECK_CODE_BAIL(err = window->addSurface());
	//COMPV_CHECK_CODE_BAIL(err = window->addSurface());
	//COMPV_CHECK_CODE_BAIL(err = window->addSurface());

	COMPV_CHECK_CODE_BAIL(err = window->surface(0)->MVP()->view()->setUpPos(CompVDrawingVec3f(0.f, 1.f, 0.f)));
	// COMPV_CHECK_CODE_BAIL(err = window->surface(0)->MVP()->model()->matrix()->translate(CompVDrawingVec3f(1.0f, 0.f, 0.f)));

	while (CompVDrawing::isLoopRunning()) {
		snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(count));
		std::string text = "Hello Doubango " + std::string(buff_);
		COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
		COMPV_CHECK_CODE_BAIL(err = window->surface(0)->drawImage(mat[(count + 0) % 3]));
		COMPV_CHECK_CODE_BAIL(err = window->surface(0)->drawText(text.c_str(), text.length()));
		//COMPV_CHECK_CODE_BAIL(err = window->surface(1)->drawImage(mat[(count + 1) % 3]));
		//COMPV_CHECK_CODE_BAIL(err = window->surface(2)->drawImage(mat[(count + 2) % 3]));
		COMPV_CHECK_CODE_BAIL(err = window->endDraw());

		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->beginDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->surface()->drawImage(mat[count % 3])) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->surface()->drawText(text.c_str(), text.length())) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->endDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);

		++count;
		
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->beginDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->drawImage(mat[/*index % 3*/0])) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->drawText(text.c_str(), text.length())) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->endDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window1->test(mat[/*index % 3*/0])) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window2->draw(mat[/*index++ % 3*/0])) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//if(count==1)break;
		//break;
	}

bail:
	return NULL;
}
