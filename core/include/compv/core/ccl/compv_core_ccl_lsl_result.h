/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_CCL_LSL_RESULT_H_)
#define _COMPV_CORE_CCL_LSL_RESULT_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_ccl.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(ConnectedComponentLabelingResultLSLImpl);

class CompVConnectedComponentLabelingResultLSLImpl : public CompVConnectedComponentLabelingResultLSL
{
protected:
	CompVConnectedComponentLabelingResultLSLImpl();
public:
	virtual ~CompVConnectedComponentLabelingResultLSLImpl();
	COMPV_OBJECT_GET_ID(CompVConnectedComponentLabelingResultLSLImpl);

	virtual size_t labelsCount() const override;
	virtual COMPV_ERROR_CODE debugFlatten(CompVMatPtrPtr labels) const override;
	virtual COMPV_ERROR_CODE extract(std::vector<CompVMatPtr>& points) const override;

	virtual COMPV_ERROR_CODE boundingBoxes() const override;
	virtual COMPV_ERROR_CODE firstOrderMoment() const override;

	static COMPV_ERROR_CODE newObj(CompVConnectedComponentLabelingResultLSLImplPtrPtr result);

private:
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CCL_LSL_RESULT_H_ */
