/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#include "compv/ds/compv_ds_camera.h"

COMPV_NAMESPACE_BEGIN()

CompVCameraDS::CompVCameraDS()
	: CompVCamera()
{

}

CompVCameraDS::~CompVCameraDS()
{

}

COMPV_ERROR_CODE CompVCameraDS::open(const std::string& name COMPV_DEFAULT("")) /* Overrides(CompVCamera) */
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCameraDS::close() /* Overrides(CompVCamera) */
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVCameraDS::newObj(CompVCameraDSPtrPtr camera)
{
	COMPV_CHECK_EXP_RETURN(!camera, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVCameraDSPtr camera_ = new CompVCameraDS();
	COMPV_CHECK_EXP_RETURN(!camera_, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);

	*camera = *camera_;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()