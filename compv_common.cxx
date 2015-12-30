/* Copyright (C) 2016 Doubango Telecom <https://www.doubango.org>
*
* This file is part of Open Source ComputerVision (a.k.a CompV) project.
* Source code hosted at https://github.com/DoubangoTelecom/compv
* Website hosted at http://compv.org
*
* CompV is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* CompV is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with CompV.
*/
#include "compv/compv_common.h"
#include "compv/compv_cpu.h"
#include "compv/compv_debug.h"

COMPV_NAMESPACE_BEGIN()

static bool s_bInitialized = false;

COMPV_ERROR_CODE CompVInit()
{
	COMPV_ERROR_CODE err_ = COMPV_ERROR_CODE_S_OK;

	if (s_bInitialized) {
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_DEBUG_INFO("Initializing engine (v %s)...", COMPV_VERSION_STRING);
#if defined(HAVE_GL_GLEW_H)
	GLenum glewErr = glewInit();
	if (GLEW_OK != glewErr) {
		COMPV_DEBUG_ERROR("Initializing GLEW failed (%d): %s", glewErr, glewGetErrorString(glewErr));
		COMPV_CHECK_CODE_RETURN(err_ = COMPV_ERROR_CODE_E_GLEW);
	}
#endif

	/* CPU features initialization */

	COMPV_CHECK_CODE_BAIL(err_ = CompVCpu::init());
	COMPV_DEBUG_INFO("CPU features: %s", CompVCpu::getFlagsAsString(CompVCpu::getFlags()));
	COMPV_DEBUG_INFO("CPU cores: #%d", CompVCpu::getCoresCount());

	COMPV_CHECK_CODE_BAIL(err_ = COMPV_ERROR_CODE_S_OK);

	COMPV_DEBUG_INFO("Engine initialized");

bail:
	s_bInitialized = COMPV_ERROR_CODE_IS_OK(err_);
	return err_;
}

bool CompVIsInitialized()
{
	return s_bInitialized;
}

// Private function used as extern
COMPV_API const char* CompVGetErrorString(COMPV_ERROR_CODE code)
{
	switch (code) {
	case COMPV_ERROR_CODE_S_OK:
		return "Success";
	case COMPV_ERROR_CODE_E_NOT_INITIALIZED:
		return "Not initialized";
	case COMPV_ERROR_CODE_E_INVALID_PARAMETER:
		return "Invalid parameter";
	case COMPV_ERROR_CODE_E_INVALID_STATE:
		return "Invalid state";
	default:
		return "Unknown code";
	}
}

COMPV_NAMESPACE_END()