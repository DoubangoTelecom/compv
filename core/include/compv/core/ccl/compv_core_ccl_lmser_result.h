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
#include "compv/base/compv_box.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

struct CompVConnectedComponentLmser {
	struct CompVConnectedComponentLmser* sister;
	struct CompVConnectedComponentLmser* child;
	struct CompVConnectedComponentLmser* parent;
	double variation;
	int8_t stable;
	int16_t greyLevel;
	CompVConnectedComponentLabelingRegionMser region;
	CompVConnectedComponentLmser(const int16_t greyLevel_ = 0)
		: greyLevel(greyLevel_) {
	}
	virtual ~CompVConnectedComponentLmser() {
	}
};
typedef CompVConnectedComponentLmser* CompVConnectedComponentLmserRef;
typedef std::vector<CompVConnectedComponentLmser, CompVAllocatorNoDefaultConstruct<CompVConnectedComponentLmser> > CompVConnectedComponentLmserVector;
typedef std::vector<CompVConnectedComponentLmserRef, CompVAllocatorNoDefaultConstruct<CompVConnectedComponentLmserRef> > CompVConnectedComponentLmserRefVector;

typedef CompVMemZero<CompVConnectedComponentLmser> CompVMemZeroCompVConnectedComponentLmser;
typedef CompVPtr<CompVMemZeroCompVConnectedComponentLmser *> CompVMemZeroCompVConnectedComponentLmserPtr;

static const bool CompVMemZeroCompVConnectedComponentLmserUseLegacyCalloc = false; // use libc's calloc or tbbCalloc?

struct CompVConnectedComponentLabelingLMSERStackMem
{
	friend class CompVConnectedComponentLabelingResultLMSERImpl;
public:
	CompVConnectedComponentLabelingLMSERStackMem() {
		m_nItemIdx = 0;
		m_nLastVecSize = 0;
	}
	virtual ~CompVConnectedComponentLabelingLMSERStackMem() {
		COMPV_CHECK_CODE_ASSERT(relase());
	}
	// https://en.wikipedia.org/wiki/Placement_syntax
	COMPV_INLINE COMPV_ERROR_CODE requestNewItem(CompVConnectedComponentLmserRef* item, const int16_t greyLevel_ = 0) {
		if (m_nItemIdx >= m_nLastVecSize) {
			COMPV_CHECK_CODE_RETURN(pushNewMem());
		}
		*item = new(&m_ptrMem[m_nItemIdx++])CompVConnectedComponentLmser(greyLevel_);
		return COMPV_ERROR_CODE_S_OK;
	}
private:
	COMPV_INLINE COMPV_ERROR_CODE relase() {
#if 1
		if (!m_vecMem.empty()) {
			for (std::vector<CompVMemZeroCompVConnectedComponentLmserPtr>::iterator i = m_vecMem.begin(); i < m_vecMem.end(); ++i) {
				const size_t count = (i == (m_vecMem.end() - 1)) ? m_nItemIdx : (*i)->cols();
				CompVConnectedComponentLmserRef ptrB = (*i)->ptr();
				const CompVConnectedComponentLmserRef ptrE = ptrB + count;
				while (ptrB < ptrE) {
					ptrB->~CompVConnectedComponentLmser();
					++ptrB;
				}
			}
		}
#endif
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_INLINE COMPV_ERROR_CODE reset() {
		COMPV_CHECK_CODE_RETURN(relase());
		if (!m_vecMem.empty()) {
			// For next time -> alloc right size'd mem
			size_t total = m_nItemIdx; // back
			for (std::vector<CompVMemZeroCompVConnectedComponentLmserPtr>::const_iterator i = m_vecMem.begin(); i < m_vecMem.end() - 1; ++i) {
				total += (*i)->cols();
			}
			m_vecMem.clear();
			if (total) {
				COMPV_CHECK_CODE_RETURN(pushNewMem(total));
			}
		}
		else {
			m_nItemIdx = 0;
			m_nLastVecSize = 0;
		}
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_INLINE COMPV_ERROR_CODE pushNewMem(size_t count = s_nMemGrowthAmount) {
		CompVMemZeroCompVConnectedComponentLmserPtr memz;
		COMPV_CHECK_CODE_RETURN(CompVMemZeroCompVConnectedComponentLmser::newObj(&memz, 1, count, 0, CompVMemZeroCompVConnectedComponentLmserUseLegacyCalloc));
		m_ptrMem = memz->ptr();
		m_nItemIdx = 0;
		m_nLastVecSize = memz->cols();
		m_vecMem.push_back(memz);
		return COMPV_ERROR_CODE_S_OK;
	}
	std::vector<CompVMemZeroCompVConnectedComponentLmserPtr> m_vecMem;
	CompVConnectedComponentLmserRef m_ptrMem;
	size_t m_nItemIdx;
	size_t m_nLastVecSize;
	static const size_t s_nMemGrowthAmount = 8192;
	 
};

COMPV_OBJECT_DECLARE_PTRS(ConnectedComponentLabelingResultLMSERImpl);

class CompVConnectedComponentLabelingResultLMSERImpl : public CompVConnectedComponentLabelingResultLMSER
{
protected:
	CompVConnectedComponentLabelingResultLMSERImpl();
public:
	virtual ~CompVConnectedComponentLabelingResultLMSERImpl();
	COMPV_OBJECT_GET_ID(CompVConnectedComponentLabelingResultLMSERImpl);

	virtual size_t labelsCount() const override;
	virtual COMPV_ERROR_CODE debugFlatten(CompVMatPtrPtr ptr32sLabels) const override;
	virtual COMPV_ERROR_CODE extract(CompVConnectedComponentPointsVector& points, COMPV_CCL_EXTRACT_TYPE type = COMPV_CCL_EXTRACT_TYPE_BLOB) const override;

	virtual const CompVConnectedComponentLabelingRegionMserRefsVector& points() const override;
	virtual const CompVConnectedComponentLabelingRegionMserRefsVector& boundingBoxes() const override;

	COMPV_ERROR_CODE reset();

	COMPV_INLINE CompVConnectedComponentLabelingRegionMserRefsVector& vecRegions() {
		return m_vecRegions;
	}
	COMPV_INLINE CompVConnectedComponentLabelingLMSERStackMem& stackMem() {
		return m_StackMem;
	}

	static COMPV_ERROR_CODE newObj(CompVConnectedComponentLabelingResultLMSERImplPtrPtr result);

private:

private:
	CompVConnectedComponentLabelingRegionMserRefsVector m_vecRegions;
	CompVConnectedComponentLabelingLMSERStackMem m_StackMem;
	bool m_bBoundingBoxesComputed;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CCL_LMSER_RESULT_H_ */
