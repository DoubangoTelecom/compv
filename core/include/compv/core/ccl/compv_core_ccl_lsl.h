/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_CCL_LSL_H_)
#define _COMPV_CORE_CCL_LSL_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_ccl.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(ConnectedComponentLabelingLSL);

class CompVConnectedComponentLabelingLSL : public CompVConnectedComponentLabeling
{
protected:
	CompVConnectedComponentLabelingLSL();
public:
	virtual ~CompVConnectedComponentLabelingLSL();
	COMPV_OBJECT_GET_ID(CompVConnectedComponentLabelingLSL);

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& binar, CompVConnectedComponentLabelingResultPtrPtr result) const override /*Overrides(CompVConnectedComponentLabeling)*/;

	static COMPV_ERROR_CODE newObj(CompVConnectedComponentLabelingPtrPtr ccl);

private:
	int m_nType;
	bool m_bSortSegments;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CCL_LSL_H_ */
