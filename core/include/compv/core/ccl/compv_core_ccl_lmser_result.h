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
#include "compv/base/parallel/compv_parallel.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

#define COMPV_CORE_LMSER_RESULT_DELETE_PTR_SAMPLES_PER_THREAD				(1024)
#define COMPV_CORE_LMSER_RESULT_COMPUTE_VARIATION_SAMPLES_PER_THREAD		(256)
#define COMPV_CORE_LMSER_RESULT_COMPUTE_STABILITY_SAMPLES_PER_THREAD		(256)

struct CompVConnectedComponentLmserLinkedListNodePixelIdx {
	int32_t data;
	struct CompVConnectedComponentLmserLinkedListNodePixelIdx* next;
};
struct CompVConnectedComponentLmserLinkedListPixelIdx {
	CompVConnectedComponentLmserLinkedListPixelIdx()
		: head(nullptr)
	{ }
	COMPV_ALWAYS_INLINE void push_front(CompVConnectedComponentLmserLinkedListNodePixelIdx* node) {
		node->next = head, head = node;
	}
	CompVConnectedComponentLmserLinkedListNodePixelIdx* head;
};

typedef const struct CompVConnectedComponentLmser* CompVConnectedComponentLmserNode;
typedef std::vector<CompVConnectedComponentLmserNode, CompVAllocatorNoDefaultConstruct<CompVConnectedComponentLmserNode> > CompVConnectedComponentLmserNodesVector;

struct CompVConnectedComponentLmser {
	friend struct CompVConnectedComponentLabelingLMSERStackMem;
	struct CompVConnectedComponentLmser* sister;
	struct CompVConnectedComponentLmser* child;
	struct CompVConnectedComponentLmser* parent;
	double variation;
	int8_t stable;
	int16_t greyLevel; // int16_t instead of uint8_t because the highest value is #256
	int area;
	CompVConnectedComponentLmserNodesVector merge_nodes;
	CompVConnectedComponentLmserLinkedListPixelIdx points;

	CompVConnectedComponentLmser(const int16_t greyLevel_ = 0)
		: greyLevel(greyLevel_) {
	}

	virtual ~CompVConnectedComponentLmser() {
	}

	COMPV_INLINE void merge(struct CompVConnectedComponentLmser* b) {
		area += b->area;
		b->sister = this->child, this->child = b, b->parent = this;
		merge_nodes.push_back(b);
	}

private:
	COMPV_INLINE void computeVariation(const int& delta) {
		const int deltaPlus = (greyLevel + delta);
		struct CompVConnectedComponentLmser* parent_;
		for (parent_ = this; parent_->parent && (parent_->parent->greyLevel <= deltaPlus); parent_ = parent_->parent)
			/* do nothing */;

		variation = (parent_->area - area) / static_cast<double>(area);
	}

	COMPV_INLINE void computeStability(const int& min_area, const int& max_area, const double& max_variation) {
		const int8_t stable_ = (!parent || (parent->variation >= variation)) &&
			(variation <= max_variation) && (min_area <= area && area <= max_area);
		struct CompVConnectedComponentLmser* child_ = child;
		if (child_) {
			if (stable_) {
				do {
					if (variation < child_->variation) {
						stable = 1;
						break;
					}
				} while ((child_ = child_->sister));
			}
		}
		else {
			stable = stable_;
		}
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
		m_vecMem.clear();
	}
	
	COMPV_INLINE size_t size() const {
		size_t total = m_nItemIdx; // back
		for (std::vector<CompVMemZeroCompVConnectedComponentLmserPtr>::const_iterator i = m_vecMem.begin(); i < m_vecMem.end() - 1; ++i) {
			total += (*i)->cols();
		}
		return total;
	}
	
	// https://en.wikipedia.org/wiki/Placement_syntax
	COMPV_INLINE COMPV_ERROR_CODE requestNewItem(CompVConnectedComponentLmserRef* item, const int16_t greyLevel_ = 0) {
		if (m_nItemIdx >= m_nLastVecSize) {
			COMPV_CHECK_CODE_RETURN(pushNewMem());
		}
		*item = new(&m_ptrMem[m_nItemIdx++])CompVConnectedComponentLmser(greyLevel_);
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE computeVariation(const int& delta) {
		for (std::vector<CompVMemZeroCompVConnectedComponentLmserPtr>::const_iterator i = m_vecMem.begin(); i < m_vecMem.end(); ++i) {
			const size_t count = (i == (m_vecMem.end() - 1)) ? m_nItemIdx : (*i)->cols();
			CompVConnectedComponentLmserRef ptr = (*i)->ptr();
			auto funcPtrComputeVariation = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
				for (size_t ii = start; ii < end; ++ii) {
					ptr[ii].computeVariation(delta);
				}
				return COMPV_ERROR_CODE_S_OK;
			};
			COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
				funcPtrComputeVariation,
				1,
				count,
				COMPV_CORE_LMSER_RESULT_COMPUTE_VARIATION_SAMPLES_PER_THREAD
			));
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	COMPV_ERROR_CODE computeStability(const int& min_area, const int& max_area, const double& max_variation) {
		for (std::vector<CompVMemZeroCompVConnectedComponentLmserPtr>::const_iterator i = m_vecMem.begin(); i < m_vecMem.end(); ++i) {
			const size_t count = (i == (m_vecMem.end() - 1)) ? m_nItemIdx : (*i)->cols();
			CompVConnectedComponentLmserRef ptr = (*i)->ptr();
			auto funcPtrComputeStability = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
				for (size_t ii = start; ii < end; ++ii) {
					ptr[ii].computeStability(min_area, max_area, max_variation);
				}
				return COMPV_ERROR_CODE_S_OK;
			};
			COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
				funcPtrComputeStability,
				1,
				count,
				COMPV_CORE_LMSER_RESULT_COMPUTE_STABILITY_SAMPLES_PER_THREAD
			));
		}
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	COMPV_INLINE COMPV_ERROR_CODE relase() {
		if (!m_vecMem.empty()) {
			// Margaret Thatcher: "I want my memory back"
			for (std::vector<CompVMemZeroCompVConnectedComponentLmserPtr>::iterator i = m_vecMem.begin(); i < m_vecMem.end(); ++i) {
				const size_t count = (i == (m_vecMem.end() - 1)) ? m_nItemIdx : (*i)->cols();
				CompVConnectedComponentLmserRef ptr = (*i)->ptr();
				auto funcPtrDelete = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
					for (size_t ii = start; ii < end; ++ii) {
						ptr[ii].~CompVConnectedComponentLmser();
					}
					return COMPV_ERROR_CODE_S_OK;
				};
				COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
					funcPtrDelete,
					1,
					count,
					COMPV_CORE_LMSER_RESULT_DELETE_PTR_SAMPLES_PER_THREAD
				));
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_INLINE COMPV_ERROR_CODE reset() {
		COMPV_CHECK_CODE_RETURN(relase());
		if (!m_vecMem.empty()) {
			// For next time -> alloc right size'd mem
			const size_t total = size();
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

	virtual const CompVConnectedComponentLabelingRegionMserVector& points() const override;
	virtual const CompVConnectedComponentLabelingRegionMserVector& boundingBoxes() const override;

	COMPV_ERROR_CODE reset();

	COMPV_INLINE CompVConnectedComponentLabelingRegionMserVector& vecRegions() {
		return m_vecRegions;
	}
	COMPV_INLINE CompVConnectedComponentLabelingLMSERStackMem& stackMem() {
		return m_StackMem;
	}

	static COMPV_ERROR_CODE newObj(CompVConnectedComponentLabelingResultLMSERImplPtrPtr result);

private:

private:
	CompVConnectedComponentLabelingRegionMserVector m_vecRegions;
	CompVConnectedComponentLabelingLMSERStackMem m_StackMem;
	bool m_bBoundingBoxesComputed;
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_CCL_LMSER_RESULT_H_ */
