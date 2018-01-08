/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/

/* @description
This class implement LMSER (Linear Time Maximally Stable Extremal Regions) algorithm.
Some literature about MSER:
- Linear Time Maximally Stable Extremal Regions: https://github.com/Stanley/043/blob/master/docs/bibl/Linear%20Time%20Maximally%20Stable%20Extremal%20Regions/53030183.pdf
*/

#include "compv/core/ccl/compv_core_ccl_lmser.h"
#include "compv/core/ccl/compv_core_ccl_lmser_result.h"
#include "compv/core/compv_core.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_memz.h"

#define COMPV_THIS_CLASSNAME	"CompVConnectedComponentLabelingLMSER"

COMPV_NAMESPACE_BEGIN()

CompVConnectedComponentLabelingLMSER::CompVConnectedComponentLabelingLMSER()
	: CompVConnectedComponentLabeling(COMPV_LMSER_ID)
{

}

CompVConnectedComponentLabelingLMSER::~CompVConnectedComponentLabelingLMSER()
{

}

COMPV_ERROR_CODE CompVConnectedComponentLabelingLMSER::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVConnectedComponentLabeling)*/
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingLMSER::process(const CompVMatPtr& binar, CompVConnectedComponentLabelingResultPtrPtr result) /*Overrides(CompVConnectedComponentLabeling)*/
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingLMSER::newObj(CompVConnectedComponentLabelingPtrPtr ccl)
{
	COMPV_CHECK_CODE_RETURN(COMPV_ERROR_CODE_E_NOT_IMPLEMENTED);
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
