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

#define COMPV_CORE_LMSER_RESULT_DELETE_PTR_SAMPLES_PER_THREAD						(1024)
#define COMPV_CORE_LMSER_RESULT_COMPUTE_VARIATION_SAMPLES_PER_THREAD				(256)
#define COMPV_CORE_LMSER_RESULT_COMPUTE_STABILITY_AND_POINTS_SAMPLES_PER_THREAD		(1) // very intensive function where regions' stability is computed and all points are built

template<typename T>
struct CompVConnectedComponentLmserLinkedListNode {
	T data;
	struct CompVConnectedComponentLmserLinkedListNode<T>* link; // "next" for "frw" and "prev" for "FrwBkw"
};

template<typename T>
struct CompVConnectedComponentLmserLinkedListFrwOnly {
	CompVConnectedComponentLmserLinkedListFrwOnly()
		: head(nullptr)
	{ }
	COMPV_ALWAYS_INLINE void push_front(CompVConnectedComponentLmserLinkedListNode<T>* node) {
		node->link = head, head = node; // node->next = head, head = node;
	}
	CompVConnectedComponentLmserLinkedListNode<T>* head;
};

template<typename T>
struct CompVConnectedComponentLmserLinkedListFrwBkw {
	CompVConnectedComponentLmserLinkedListFrwBkw()
		: tail(nullptr)
	{ }
	COMPV_ALWAYS_INLINE bool empty()const {
		return !tail;
	}
	COMPV_ALWAYS_INLINE void push_back(CompVConnectedComponentLmserLinkedListNode<T>* node) {
		node->link = tail, tail = node; // node->prev = tail, tail = node;
	}
	COMPV_ALWAYS_INLINE void pop_back() {
#if 0 // tail never null -> checked by the caller
		tail = tail ? tail->link : nullptr;
#else
		tail = tail->link; // tail = tail->prev;
#endif
	}
	CompVConnectedComponentLmserLinkedListNode<T>* tail;
};

typedef CompVConnectedComponentLmserLinkedListNode<int> CompVConnectedComponentLmserLinkedListNodePixelIdx;
typedef CompVConnectedComponentLmserLinkedListFrwOnly<int> CompVConnectedComponentLmserLinkedListPixelIdx;

typedef CompVConnectedComponentLmserLinkedListNode<const struct CompVConnectedComponentLmser*> CompVConnectedComponentLmserLinkedListNodeMerge;
typedef CompVConnectedComponentLmserLinkedListFrwOnly<int> CompVConnectedComponentLmserLinkedListMerge;

typedef struct CompVConnectedComponentLmser* CompVConnectedComponentLmserRef;
typedef std::vector<CompVConnectedComponentLmserRef, CompVAllocatorNoDefaultConstruct<CompVConnectedComponentLmserRef> > CompVConnectedComponentLmserRefVector;
typedef std::vector<struct CompVConnectedComponentLmser, CompVAllocatorNoDefaultConstruct<struct CompVConnectedComponentLmser> > CompVConnectedComponentLmserVector;

struct CompVConnectedComponentLmser {
	friend struct CompVConnectedComponentLabelingLMSERStackMem;
	struct CompVConnectedComponentLmser* sister;
	struct CompVConnectedComponentLmser* child;
	struct CompVConnectedComponentLmser* parent;
	double variation;
	bool stable;
	uint16_t greyLevel; // uint16_t instead of uint8_t because the highest value is #256
	int area;
	CompVConnectedComponentLmserLinkedListNodeMerge* linked_list_merges_head; /* CompVConnectedComponentLmserLinkedListFrwOnly */
	CompVConnectedComponentLmserLinkedListNodePixelIdx* linked_list_points_head; /* CompVConnectedComponentLmserLinkedListFrwOnly */

	COMPV_ALWAYS_INLINE void merge(CompVConnectedComponentLmserLinkedListNodeMerge*& pool, struct CompVConnectedComponentLmser* b) {
		area += b->area;
		b->sister = this->child, this->child = b, b->parent = this;
		pool->data = b;
		pool->link = linked_list_merges_head, linked_list_merges_head = pool++; // push_front(b)
	}

	// Not MT-friendly
	void collectStableRegions(const double& one_minus_min_diversity, const double& one_minus_min_diversity_scale, CompVConnectedComponentLmserRefVector& vecRegions_stable, size_t& index) {
		bool& stable_ = stable;
		if (stable_) {
			const int min_parent_area = COMPV_MATH_ROUNDFU_2_NEAREST_INT((area * one_minus_min_diversity_scale), int);
			for (struct CompVConnectedComponentLmser* parent_ = parent;
				(parent_ && (parent_->area < min_parent_area) && (stable_ = (!parent_->stable || (parent_->variation > variation))));
				(parent_ = parent_->parent)
				) /* do noting */;

			stable_ =
				stable_ &&
				(checkCrit(
					COMPV_MATH_ROUNDFU_2_NEAREST_INT((area * one_minus_min_diversity), int), // max_child_area
					variation
				));
			if (stable_) {
				vecRegions_stable[index++] = this;
			}
		}
		struct CompVConnectedComponentLmser* child_ = child;
		while (child_) {
			child_->collectStableRegions(one_minus_min_diversity, one_minus_min_diversity_scale, vecRegions_stable, index);
			child_ = child_->sister;
		}
	}

	// MT-friendly
	void computeFinalPoints(CompVConnectedComponentLabelingRegionMser& cc_final, size_t& index, const int& stride, const float& stride_scale) const {
#if 0 // Non-recurssive call
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("Recurssive call faster");
		CompVConnectedComponentPoints& points_final = cc_final.points;
		std::vector<const struct CompVConnectedComponentLmser*, CompVAllocatorNoDefaultConstruct<const struct CompVConnectedComponentLmser* > > stack(area);
		size_t ii = 0;
		stack[ii++] = this;
		while (ii) {
			const struct CompVConnectedComponentLmser* cc = stack[--ii];
			for (const CompVConnectedComponentLmserLinkedListNodePixelIdx* node = cc->linked_list_points_head; node; node = node->link) {
				CompVPoint2DInt16& point = points_final[index++];
				point.y = static_cast<int16_t>(node->data * stride_scale);
				point.x = static_cast<int16_t>(node->data - (point.y * stride));
			}
			for (const CompVConnectedComponentLmserLinkedListNodeMerge* node = cc->linked_list_merges_head; node; node = node->link) {
				stack[ii++] = node->data;
			}
		}
#elif 1	// Recurssive call
		CompVConnectedComponentPoints& points_final = cc_final.points;
		for (const CompVConnectedComponentLmserLinkedListNodePixelIdx* node = linked_list_points_head; node; node = node->link) {
			CompVPoint2DInt16& point = points_final[index++];
			point.y = static_cast<int16_t>(node->data * stride_scale);
			point.x = static_cast<int16_t>(node->data - (point.y * stride));
		}
		for (const CompVConnectedComponentLmserLinkedListNodeMerge* node = linked_list_merges_head; node; node = node->link) {
			node->data->computeFinalPoints(cc_final, index, stride, stride_scale);
		}
#endif
	}

private:
	// MT-friendly
	COMPV_INLINE void computeVariation(const int& delta) {
		const int deltaPlus = (greyLevel + delta);
		struct CompVConnectedComponentLmser* parent_;
		for (parent_ = this; parent_->parent && (parent_->parent->greyLevel <= deltaPlus); parent_ = parent_->parent)
			/* do nothing */;

		variation = (parent_->area - area) / static_cast<double>(area);
	}

	// MT-friendly
	COMPV_INLINE void computeStability(const int& min_area, const int& max_area, const double& max_variation) {
		const bool stable_ = (!parent || (parent->variation >= variation)) &&
			(variation <= max_variation) && (min_area <= area && area <= max_area);
		struct CompVConnectedComponentLmser* child_ = child;
		if (child_) {
			if (stable_) {
				do {
					if (variation < child_->variation) {
						stable = true;
						break;
					}
				} while ((child_ = child_->sister));
			}
		}
		else {
			stable = stable_;
		}
	}

	// MT-friendly
	COMPV_INLINE bool checkCrit(const int& area_, const double& variation_) const {
		if (area <= area_) {
			return true;
		}
		if (stable && (variation < variation_)) {
			return false;
		}
		struct CompVConnectedComponentLmser* child_ = child;
		while (child_) {
			if (!child_->checkCrit(area_, variation_)) {
				return false;
			}
			child_ = child_->sister;
		}
		return true;
	}
};

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
		COMPV_CHECK_CODE_ASSERT(release());
		m_vecMem.clear();
	}
	
	COMPV_INLINE size_t size() const {
		size_t total = m_nItemIdx; // back
		for (std::vector<CompVMemZeroCompVConnectedComponentLmserPtr>::const_iterator i = m_vecMem.begin(); i < m_vecMem.end() - 1; ++i) {
			total += (*i)->cols();
		}
		return total;
	}
	
	COMPV_INLINE COMPV_ERROR_CODE requestNewItem(CompVConnectedComponentLmserRef* item, const int16_t greyLevel_ = 0) {
		if (m_nItemIdx >= m_nLastVecSize) {
			COMPV_CHECK_CODE_RETURN(pushNewMem());
		}
#if 0
		// https://en.wikipedia.org/wiki/Placement_syntax
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No need to call the ctors as the is no structured element (e.g. std::vector)");
		*item = new(&m_ptrMem[m_nItemIdx++])CompVConnectedComponentLmser(greyLevel_);
#else
		*item = &m_ptrMem[m_nItemIdx++];
		(*item)->greyLevel = greyLevel_;
#endif
		return COMPV_ERROR_CODE_S_OK;
	}

	// MT-friendly
	COMPV_ERROR_CODE computeVariation(const int& delta) {
		CompVConnectedComponentLmserRef ptr = nullptr;
		auto funcPtrComputeVariation = [&](const size_t start, const size_t end) -> COMPV_ERROR_CODE {
			for (size_t ii = start; ii < end; ++ii) {
#if 0
				ptr[ii].computeVariation(delta);
#else
				CompVConnectedComponentLmser& cc = ptr[ii];
				const int deltaPlus = (cc.greyLevel + delta);
				struct CompVConnectedComponentLmser* parent_;
				for (parent_ = &cc; parent_->parent && (parent_->parent->greyLevel <= deltaPlus); parent_ = parent_->parent)
					/* do nothing */;

				cc.variation = (parent_->area - cc.area) / static_cast<double>(cc.area);
#endif
			}
			return COMPV_ERROR_CODE_S_OK;
		};
		for (std::vector<CompVMemZeroCompVConnectedComponentLmserPtr>::const_iterator i = m_vecMem.begin(); i < m_vecMem.end(); ++i) {
			const size_t count = (i == (m_vecMem.end() - 1)) ? m_nItemIdx : (*i)->cols();
			ptr = (*i)->ptr();
			COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
				funcPtrComputeVariation,
				1,
				count,
				COMPV_CORE_LMSER_RESULT_COMPUTE_VARIATION_SAMPLES_PER_THREAD
			));
		}
		return COMPV_ERROR_CODE_S_OK;
	}

	// MT-friendly
	COMPV_ERROR_CODE computeStability(const int& min_area, const int& max_area, const double& max_variation, size_t& totalTemporaryStableRegions) {
		totalTemporaryStableRegions = 0;
		CompVThreadDispatcherPtr threadDisp = CompVParallel::threadDispatcher();
		const size_t maxThreads = threadDisp ? static_cast<size_t>(threadDisp->threadsCount()) : 1;
		const bool dispatcherNotMotherOfTheCurrentThread = (threadDisp && !threadDisp->isMotherOfTheCurrentThread());
		auto funcPtrComputeStability = [&](CompVConnectedComponentLmserRef ptr, const size_t start, const size_t end, size_t* numStableRegions) -> COMPV_ERROR_CODE {
			size_t& numStableRegions_ = *numStableRegions;
			for (size_t ii = start; ii < end; ++ii) {
#if 0
				ptr[ii].computeStability(min_area, max_area, max_variation);
#else
				CompVConnectedComponentLmser& cc = ptr[ii];
				const bool stable_ = (!cc.parent || (cc.parent->variation >= cc.variation)) &&
					(cc.variation <= max_variation) && (min_area <= cc.area && cc.area <= max_area);
				struct CompVConnectedComponentLmser* child_ = cc.child;
				if (child_) {
					if (stable_) {
						do {
							if (cc.variation < child_->variation) {
								cc.stable = true;
								break;
							}
						} while ((child_ = child_->sister));
					}
				}
				else {
					cc.stable = stable_;
				}
#endif
				numStableRegions_ += ptr[ii].stable;
			}
			return COMPV_ERROR_CODE_S_OK;
		};

		for (std::vector<CompVMemZeroCompVConnectedComponentLmserPtr>::const_iterator i = m_vecMem.begin(); i < m_vecMem.end(); ++i) {
			const size_t count = (i == (m_vecMem.end() - 1)) ? m_nItemIdx : (*i)->cols();
			const size_t threadsCount = dispatcherNotMotherOfTheCurrentThread
				? CompVThreadDispatcher::guessNumThreadsDividingAcrossY(1, count, maxThreads, COMPV_CORE_LMSER_RESULT_COMPUTE_STABILITY_AND_POINTS_SAMPLES_PER_THREAD)
				: 1;
			CompVConnectedComponentLmserRef ptr = (*i)->ptr();
			if (threadsCount > 1) {
				CompVAsyncTaskIds taskIds;
				taskIds.reserve(threadsCount);
				std::vector<size_t> numStableRegions(threadsCount, 0);
				const size_t heights = (count / threadsCount);
				size_t YStart = 0, YEnd;
				for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
					YEnd = (threadIdx == (threadsCount - 1)) ? count : (YStart + heights);
					COMPV_CHECK_CODE_RETURN(threadDisp->invoke(std::bind(funcPtrComputeStability, ptr, YStart, YEnd, &numStableRegions[threadIdx]), taskIds), "Dispatching task failed");
					YStart += heights;
				}
				COMPV_CHECK_CODE_RETURN(threadDisp->wait(taskIds), "Failed to wait for tasks execution");
				for (size_t threadIdx = 0; threadIdx < threadsCount; ++threadIdx) {
					totalTemporaryStableRegions += numStableRegions[threadIdx];
				}
			}
			else {
				COMPV_CHECK_CODE_RETURN(funcPtrComputeStability(ptr, 0, count, &totalTemporaryStableRegions));
			}
		}
		return COMPV_ERROR_CODE_S_OK;
	}

private:
	COMPV_INLINE COMPV_ERROR_CODE release() {
#if 0
		COMPV_DEBUG_INFO_CODE_NOT_OPTIMIZED("No need to call the dtors as the is no structured element (e.g. std::vector)");
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
#endif
		return COMPV_ERROR_CODE_S_OK;
	}
	COMPV_INLINE COMPV_ERROR_CODE reset() {
		COMPV_CHECK_CODE_RETURN(release());
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
