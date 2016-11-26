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
	COMPV_CHECK_CODE_BAIL(err = CompVWindow::newObj(&window, 670, 580, "Hello world!"));

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
#if 0 // Multiple
	COMPV_ERROR_CODE err;
	CompVMatPtr mat[3];
	CompVMultiSurfaceLayerPtr multipleSurfaceLayer;
	CompVSurfacePtr surfaces[3];
	CompVViewportPtr ptrViewPort;
	CompVMVPPtr ptrMVP;
	static int count = 0;
	static uint64_t timeStart;
	char buff_[33] = { 0 };

	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("girl.jpg"), &mat[0]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("valve_original.jpg"), &mat[1]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("mandekalou.jpg"), &mat[2]));

	COMPV_CHECK_CODE_BAIL(err = window->addMultiLayerSurface(&multipleSurfaceLayer));
	COMPV_CHECK_CODE_BAIL(err = multipleSurfaceLayer->addSurface(&surfaces[0], window->width(), window->height()));
	COMPV_CHECK_CODE_BAIL(err = multipleSurfaceLayer->addSurface(&surfaces[1], window->width(), window->height()));
	COMPV_CHECK_CODE_BAIL(err = multipleSurfaceLayer->addSurface(&surfaces[2], window->width(), window->height()));

	COMPV_CHECK_CODE_BAIL(err = surfaces[0]->viewport()->reset(CompViewportSizeFlags::makeStatic(), 0, 0, 120, 120));
	COMPV_CHECK_CODE_BAIL(err = surfaces[1]->viewport()->reset(CompViewportSizeFlags::makeStatic(), 120, 0, 120, 120));
	COMPV_CHECK_CODE_BAIL(err = surfaces[2]->viewport()->reset(CompViewportSizeFlags::makeStatic(), 0, 120, 120, 120));

	timeStart = CompVTime::getNowMills();
	while (CompVDrawing::isLoopRunning()) {
		snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(count));
		std::string text = "Hello doubango telecom [" + std::string(buff_) + "]";
		COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
		//COMPV_CHECK_CODE_BAIL(err = surfaces[0]->drawText(text.c_str(), text.length(), 0, 0));
		COMPV_CHECK_CODE_BAIL(err = surfaces[0]->drawImage(mat[0/*(count + 0) % 3*/]));
		COMPV_CHECK_CODE_BAIL(err = surfaces[1]->drawImage(mat[1/*(count + 0) % 3*/]));
		COMPV_CHECK_CODE_BAIL(err = surfaces[2]->drawImage(mat[2/*(count + 0) % 3*/]));

		if (count == 1000) {
			uint64_t duration = (CompVTime::getNowMills() - timeStart);
			float fps = 1000.f / ((static_cast<float>(duration)) / 1000.f);
			COMPV_DEBUG_INFO("Elapsed time: %llu, fps=%f", duration, fps);
			count = 0;
			timeStart = CompVTime::getNowMills();
		}
		++count;

		COMPV_CHECK_CODE_BAIL(err = multipleSurfaceLayer->blit());
		COMPV_CHECK_CODE_BAIL(err = window->endDraw());
	}

bail:
	return NULL;

#elif 0 // Matching
	COMPV_ERROR_CODE err;
	CompVMatPtr mat[3];
	CompVMatchingSurfaceLayerPtr matchingSurfaceLayer;
	CompVViewportPtr ptrViewPort;
	CompVMVPPtr ptrMVP;
	static int count = 0;
	static uint64_t timeStart;
	char buff_[33] = { 0 };

	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("girl.jpg"), &mat[0]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("valve_original.jpg"), &mat[1]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("mandekalou.jpg"), &mat[2]));

	COMPV_CHECK_CODE_BAIL(err = window->addMatchingLayerSurface(&matchingSurfaceLayer));

	timeStart = CompVTime::getNowMills();
	while (CompVDrawing::isLoopRunning()) {
		snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(count));
		std::string text = "Hello doubango telecom [" + std::string(buff_) + "]";
		COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
		COMPV_CHECK_CODE_BAIL(err = matchingSurfaceLayer->drawMatches(mat[0/*(count + 0) % 3*/], mat[1/*(count + 0) % 3*/]));

		if (count == 1000) {
			uint64_t duration = (CompVTime::getNowMills() - timeStart);
			float fps = 1000.f / ((static_cast<float>(duration)) / 1000.f);
			COMPV_DEBUG_INFO("Elapsed time: %llu, fps=%f", duration, fps);
			count = 0;
			timeStart = CompVTime::getNowMills();
		}
		++count;

		COMPV_CHECK_CODE_BAIL(err = matchingSurfaceLayer->blit());
		COMPV_CHECK_CODE_BAIL(err = window->endDraw());
	}

bail:
	return NULL;
#elif 1 // Single
	COMPV_ERROR_CODE err;
	CompVMatPtr mat[3];
	CompVSingleSurfaceLayerPtr singleSurfaceLayer;
	CompVViewportPtr ptrViewPort;
	CompVMVPPtr ptrMVP;
	static int count = 0;
	static uint64_t timeStart;
	char buff_[33] = { 0 };
	
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("girl.jpg"), &mat[0]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("valve_original.jpg"), &mat[1]));
	COMPV_CHECK_CODE_BAIL(err = CompVImageDecoder::decodeFile(COMPV_PATH_FROM_NAME("mandekalou.jpg"), &mat[2]));

	COMPV_CHECK_CODE_BAIL(err = window->addSingleLayerSurface(&singleSurfaceLayer));
	/*
	// Set viewport
	COMPV_CHECK_CODE_BAIL(err = CompVViewport::newObj(&ptrViewPort, CompViewportSizeFlags::makeDynamicAspectRatio()));
	COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->setViewport(ptrViewPort));

	// Set MVP
	COMPV_CHECK_CODE_BAIL(err = CompVMVP::newObjProjection2D(&ptrMVP));
	COMPV_CHECK_CODE_BAIL(err = ptrMVP->model()->matrix()->scale(CompVVec3f(1.f, 1.f, 1.f)));
	COMPV_CHECK_CODE_BAIL(err = ptrMVP->view()->setCamera(CompVVec3f(0.f, 0.f, 1.f), CompVVec3f(0.f, 0.f, 0.f), CompVVec3f(0.f, 1.f, 0.f)));
	COMPV_CHECK_CODE_BAIL(err = ptrMVP->projection2D()->setOrtho(-1.f, 1.f, -1.f, 1.f, -1.f, 1.f));
	COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->setMVP(ptrMVP));*/

	timeStart = CompVTime::getNowMills();
	while (CompVDrawing::isLoopRunning()) {
		snprintf(buff_, sizeof(buff_), "%d", static_cast<int>(count));
		std::string text = "Hello doubango telecom [" + std::string(buff_) + "]";
		COMPV_CHECK_CODE_BAIL(err = window->beginDraw());
		COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->drawImage(mat[0/*(count + 0) % 3*/]));
		//COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->drawText(text.c_str(), text.length(), 463, 86));
		COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->surface()->renderer()->canvas()->drawText(text.c_str(), text.length(), 463, 86));

		if (count == 1000) {
			uint64_t duration = (CompVTime::getNowMills() - timeStart);
			float fps = 1000.f / ((static_cast<float>(duration)) / 1000.f);
			COMPV_DEBUG_INFO("Elapsed time: %llu, fps=%f", duration, fps);
			count = 0;
			timeStart = CompVTime::getNowMills();
		}
		++count;

		COMPV_CHECK_CODE_BAIL(err = singleSurfaceLayer->blit());
		COMPV_CHECK_CODE_BAIL(err = window->endDraw());

		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->beginDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->surface()->drawImage(mat[count % 3])) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->surface()->drawText(text.c_str(), text.length())) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);
		//COMPV_CHECK_EXP_BAIL(COMPV_ERROR_CODE_IS_NOK(err = window->endDraw()) && err != COMPV_ERROR_CODE_W_WINDOW_CLOSED, err);

		++count;
	}

bail:
	return NULL;
#endif
}
