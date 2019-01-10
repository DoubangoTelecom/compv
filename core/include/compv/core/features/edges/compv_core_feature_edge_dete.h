/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_FEATURES_EDGE_DETE_H_)
#define _COMPV_CORE_FEATURES_EDGE_DETE_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_features.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(CornerDeteEdgeBase);

class CompVCornerDeteEdgeBase : public CompVEdgeDete
{
protected:
	CompVCornerDeteEdgeBase(int id, const int16_t* kernelPtrVt, const int16_t* kernelPtrHz, size_t kernelSize);
public:
	virtual ~CompVCornerDeteEdgeBase();
	COMPV_OBJECT_GET_ID(CompVCornerDeteEdgeBase);

	virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
	virtual COMPV_ERROR_CODE process(const CompVMatPtr& image, CompVMatPtrPtr edges, CompVMatPtrPtr directions = NULL) override /*Overrides(CompVEdgeDete)*/;

	static COMPV_ERROR_CODE newObjSobel(CompVEdgeDetePtrPtr dete, float tLow = 0.68f, float tHigh = 0.68f*2.f, size_t kernSize = 3);
	static COMPV_ERROR_CODE newObjScharr(CompVEdgeDetePtrPtr dete, float tLow = 0.68f, float tHigh = 0.68f*2.f, size_t kernSize = 3);
	static COMPV_ERROR_CODE newObjPrewitt(CompVEdgeDetePtrPtr dete, float tLow = 0.68f, float tHigh = 0.68f*2.f, size_t kernSize = 3);

private:
	static COMPV_ERROR_CODE newObj(CompVEdgeDetePtrPtr dete, int id, float tLow = 0.68f, float tHigh = 0.68f*2.f, size_t kernSize = 3);
	const int16_t* m_pcKernelVt;
	const int16_t* m_pcKernelHz;
	size_t m_nKernelSize;
	size_t m_nImageWidth;
	size_t m_nImageHeight;
	size_t m_nImageStride;
	int16_t* m_pGx;
	int16_t* m_pGy;
	uint16_t* m_pG;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_FEATURES_EDGE_DETE_H_ */
