/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_CCL_LMSER_RESULT_H_)
#define _COMPV_CORE_CCL_LMSER_RESULT_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_memz.h"
#include "compv/base/compv_ccl.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(ConnectedComponentLabelingResultLMSERImpl);

class ConnectedComponentLabelingResultLMSERImpl : public CompVConnectedComponentLabelingResultLMSER
{
protected:
	ConnectedComponentLabelingResultLMSERImpl();
public:
	virtual ~ConnectedComponentLabelingResultLMSERImpl();
	COMPV_OBJECT_GET_ID(ConnectedComponentLabelingResultLMSERImpl);

	virtual size_t labelsCount() const override;
	virtual COMPV_ERROR_CODE debugFlatten(CompVMatPtrPtr ptr32sLabels) const override;
	virtual COMPV_ERROR_CODE extract(CompVConnectedComponentPointsVector& points, COMPV_CCL_EXTRACT_TYPE type = COMPV_CCL_EXTRACT_TYPE_BLOB) const override;

	virtual COMPV_ERROR_CODE boundingBoxes(CompVConnectedComponentBoundingBoxesVector& boxes) const override;
	virtual COMPV_ERROR_CODE moments(CompVConnectedComponentMoments& moments) const override;

	COMPV_ERROR_CODE reset();

	static COMPV_ERROR_CODE newObj(CompVConnectedComponentLabelingResultLMSERImplPtrPtr result);

private:

private:
	size_t m_nCount; // final number of absolute labels
	CompVSizeSz m_szInput;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CCL_LMSER_RESULT_H_ */
