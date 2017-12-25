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
#include "compv/base/compv_memz.h"
#include "compv/base/compv_ccl.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

static const int32_t kCompVConnectedComponentLabelingLSLBachgroundLabel = 0; // Must be zero because of calloc()

typedef std::vector<CompVRangeInt16 > compv_ccl_rlc_t;
struct compv_ccl_range_t {
	int32_t a; // = A[ERA[er]] within [1, na]. /!\ IMPORTANT: not zero-based
	int16_t start; // RLC[er - 1]
	int16_t end; // RLC[er]
};
typedef std::vector<compv_ccl_range_t, CompVAllocatorNoDefaultConstruct<compv_ccl_range_t> > compv_ccl_lea_1_t;
typedef std::vector<compv_ccl_lea_1_t > compv_ccl_lea_n_t;


COMPV_OBJECT_DECLARE_PTRS(ConnectedComponentLabelingResultLSLImpl);

class CompVConnectedComponentLabelingResultLSLImpl : public CompVConnectedComponentLabelingResultLSL
{
protected:
	CompVConnectedComponentLabelingResultLSLImpl();
public:
	virtual ~CompVConnectedComponentLabelingResultLSLImpl();
	COMPV_OBJECT_GET_ID(CompVConnectedComponentLabelingResultLSLImpl);

	virtual int32_t backgroundLabelId() const override { return kCompVConnectedComponentLabelingLSLBachgroundLabel; }
	virtual size_t labelsCount() const override;
	virtual COMPV_ERROR_CODE debugFlatten(CompVMatPtrPtr ptr32sLabels) const override;
	virtual COMPV_ERROR_CODE extract(CompVConnectedComponentPointsVector& points, COMPV_CCL_EXTRACT_TYPE type = COMPV_CCL_EXTRACT_TYPE_BLOB) const override;

	virtual COMPV_ERROR_CODE boundingBoxes() const override;
	virtual COMPV_ERROR_CODE firstOrderMoment() const override;

	COMPV_INLINE int32_t& na1() { return m_nNa1; }
	COMPV_INLINE compv_ccl_lea_n_t& vecLEA() { return m_vecLEA; }
	COMPV_INLINE CompVSizeSz& szInput() { return m_szInput; }

	COMPV_ERROR_CODE reset();

	static COMPV_ERROR_CODE newObj(CompVConnectedComponentLabelingResultLSLImplPtrPtr result);

private:
	int32_t m_nNa1; // final number of absolute labels
	compv_ccl_lea_n_t m_vecLEA; // an associative table holding the association between er and ea: ea = ERAi[er]
	CompVSizeSz m_szInput;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CCL_LSL_RESULT_H_ */
