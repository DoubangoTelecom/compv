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

#if COMPV_OS_WINDOWS
typedef LONG compv_ccl_accumulator_t; /* InterlockedDecrement/InterlockedIncrement requires LONG */
#else
typedef int compv_ccl_accumulator_t;
#endif
typedef CompVMemZero<compv_ccl_accumulator_t > CompVMemZeroLockedCount;
typedef CompVPtr<CompVMemZeroLockedCount *> CompVMemZeroLockedCountPtr;

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
	virtual COMPV_ERROR_CODE extract(std::vector<CompVMatPtr>& points) const override;

	virtual COMPV_ERROR_CODE boundingBoxes() const override;
	virtual COMPV_ERROR_CODE firstOrderMoment() const override;

	COMPV_INLINE CompVMemZeroLockedCountPtr& ptrxNa() { return m_ptrxNa; }
	COMPV_INLINE CompVMemZero32sPtr& ptr32sERA() { return m_ptr32sERA; }
	COMPV_INLINE CompVMatPtr& ptr32sA() { return m_ptr32sA; }
	COMPV_INLINE CompVMatPtr& ptr16sRLC() { return m_ptr16sRLC; }
	COMPV_INLINE CompVMatPtr& ptr16sNer() { return m_ptr16sNer; }
	COMPV_INLINE CompVSizeSz& szInput() { return m_szInput; }

	static COMPV_ERROR_CODE newObj(CompVConnectedComponentLabelingResultLSLImplPtrPtr result);

private:
	CompVMemZeroLockedCountPtr m_ptrxNa; // final number of absolute labels (only needed to extract points)
	CompVMemZero32sPtr m_ptr32sERA; // an associative table holding the association between er and ea: ea = ERAi[er]
	CompVMatPtr m_ptr32sA; // the associative table of ancestors
	CompVMatPtr m_ptr16sRLC; // a table holding the run length coding of segments of the line Xi, RLCi-1 is the similar memorization of the previous line.
	CompVMatPtr m_ptr16sNer; // the number of segments of ERi - black + white -
	CompVSizeSz m_szInput;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CCL_LSL_RESULT_H_ */
