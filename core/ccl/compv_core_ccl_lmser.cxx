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
#include "compv/core/compv_core.h"
#include "compv/base/parallel/compv_parallel.h"
#include "compv/base/compv_cpu.h"
#include "compv/base/compv_memz.h"

#define COMPV_CORE_LMSER_ACCESSIBILITY_BUILD_SAMPLES_PER_THREAD (40 * 40)

#define LMSER_HIGHEST_GREYLEVEL		256
#define LMSER_GOTO(stepx) goto __________________________##stepx##__________________________
#define LMSER_MERGE(a, b) \
 { \
	CompVConnectedComponentPoints& ap = a->region.points; \
	CompVConnectedComponentPoints& bp = b->region.points; \
	ap.insert(ap.end(), bp.begin(), bp.end()); \
	b->sister = a->child, a->child = b, b->parent = a; \
}

#define LMSER_EDGE_RIGHT		16	// 0b000 1000 0
#define LMSER_EDGE_BOTTOM		8	// 0b000 0100 0
#define LMSER_EDGE_LEFT			4	// 0b000 0010 0
#define LMSER_EDGE_TOP			2	// 0b000 0001 0
#define LMSER_EDGE_RIGHT_TOP	18  // (RIGTH | TOP)
#define LMSER_EDGE_LEFT_TOP		6	// (LEFT | TOP)
#define LMSER_EDGE_LEFT_BOTTOM	12	// (LEFT | BOTTOM)
#define LMSER_EDGE_RIGHT_BOTTOM	24	// (RIGHT | BOTTOM)

#define COMPV_THIS_CLASSNAME	"CompVConnectedComponentLabelingLMSER"

COMPV_NAMESPACE_BEGIN()

static const uint8_t LMSER_EDGES_MASKS[8] = {
	LMSER_EDGE_RIGHT,
	LMSER_EDGE_BOTTOM,
	LMSER_EDGE_LEFT,
	LMSER_EDGE_TOP,
	LMSER_EDGE_RIGHT_TOP,
	LMSER_EDGE_LEFT_TOP,
	LMSER_EDGE_LEFT_BOTTOM,
	LMSER_EDGE_RIGHT_BOTTOM,
};

CompVConnectedComponentLabelingLMSER::CompVConnectedComponentLabelingLMSER()
	: CompVConnectedComponentLabeling(COMPV_LMSER_ID)
{

}

CompVConnectedComponentLabelingLMSER::~CompVConnectedComponentLabelingLMSER()
{

}

COMPV_ERROR_CODE CompVConnectedComponentLabelingLMSER::set(int id, const void* valuePtr, size_t valueSize) /*Overrides(CompVConnectedComponentLabeling)*/
{
	switch (id) {
	case COMPV_CCL_SET_INT_CONNECTIVITY:
	default:
		COMPV_CHECK_CODE_RETURN(CompVConnectedComponentLabeling::set(id, valuePtr, valueSize));
		return COMPV_ERROR_CODE_S_OK;
	}
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingLMSER::process(const CompVMatPtr& ptr8uImage, CompVConnectedComponentLabelingResultPtrPtr result) /*Overrides(CompVConnectedComponentLabeling)*/
{
	COMPV_CHECK_EXP_RETURN(!ptr8uImage || ptr8uImage->isEmpty() || ptr8uImage->planeCount() != 1 || ptr8uImage->elmtInBytes() != sizeof(uint8_t) || !result
		, COMPV_ERROR_CODE_E_INVALID_PARAMETER);

	/* Create result */
	CompVConnectedComponentLabelingResultLMSERImplPtr result_;
	if (*result && (*result)->id() == id()) {
		result_ = reinterpret_cast<CompVConnectedComponentLabelingResultLMSERImpl*>(**result);
	}
	if (!result_) {
		COMPV_CHECK_CODE_RETURN(CompVConnectedComponentLabelingResultLMSERImpl::newObj(&result_));
	}
	COMPV_CHECK_CODE_RETURN(result_->reset());
	CompVConnectedComponentLabelingRegionMserRefsVector& vecRegions = result_->vecRegions();

	const int16_t width = static_cast<int16_t>(ptr8uImage->cols());
	const int16_t widthMinus1 = width - 1;
	const int16_t height = static_cast<int16_t>(ptr8uImage->rows());
	const int16_t heightMinus1 = height - 1;
	const int16_t stride = static_cast<int16_t>(ptr8uImage->stride());
	const float stride_scale = 1.f / float(stride);

	const uint8_t* ptr8uPixelsRef = ptr8uImage->ptr<const uint8_t>();

	const bool b8Connectivity = (connectivity() == 8);
	const int8_t maxEdges = b8Connectivity ? 8 : 4;
	const int16_t LMSER_EDGES_OFFSETS[8] = {
		1,
		stride,
		-1,
		-stride,
		1 - stride,
		-(1 + stride),
		stride - 1,
		stride + 1
	};

	// A binary mask of accessible pixels. These are the pixels to which the water
	// already has access.
	CompVMatPtr ptr8uAccessible;
	COMPV_CHECK_CODE_RETURN(CompVMat::newObjAligned<uint8_t>(&ptr8uAccessible, height, width, stride));
	uint8_t* ptr8uAccessibleRef = ptr8uAccessible->ptr<uint8_t>();
	auto funcPtrSetAccessibility = [&](const size_t ystart, const size_t yend) -> COMPV_ERROR_CODE {
		size_t ystart_ = ystart;
		size_t yend_ = yend;
		/* TOP line */
		if (!ystart) {
			uint8_t* mt_ptr8u = ptr8uAccessible->ptr<uint8_t>(0);
			CompVMem::set(&mt_ptr8u[1], 28, (width - 2), sizeof(uint8_t));
			mt_ptr8u[0] = 24;
			mt_ptr8u[widthMinus1] = 12;
			ystart_ = 1;
		}
		/* BOTTOM line */
		if (yend == height) {
			uint8_t* mt_ptr8u = ptr8uAccessible->ptr<uint8_t>(heightMinus1);
			CompVMem::set(&mt_ptr8u[1], 22, (width - 2), sizeof(uint8_t));
			mt_ptr8u[0] = 18;
			mt_ptr8u[widthMinus1] = 6;
			yend_ = heightMinus1;
		}
		/* OTHER lines */
		uint8_t* mt_ptr8uAccessibleRef = ptr8uAccessible->ptr<uint8_t>(ystart_);
		const int16_t width64_ = (widthMinus1 < 8) ? 0 : ((widthMinus1 - 8) & -8);
		int16_t x;
		for (size_t y = ystart_; y < yend_; ++y) {
			*reinterpret_cast<uint64_t*>(&mt_ptr8uAccessibleRef[0]) = 2170205185142300186ul;
			for (x = 8; x < width64_; x += 8) {
				*reinterpret_cast<uint64_t*>(&mt_ptr8uAccessibleRef[x]) = 2170205185142300190ul;
			}
			for (; x < widthMinus1; ++x) {
				mt_ptr8uAccessibleRef[x] = 30;
			}
			mt_ptr8uAccessibleRef[widthMinus1] = 14;
			mt_ptr8uAccessibleRef += stride;
		}
		return COMPV_ERROR_CODE_S_OK;
	};
	COMPV_CHECK_CODE_RETURN(CompVThreadDispatcher::dispatchDividingAcrossY(
		funcPtrSetAccessibility,
		width,
		height,
		std::max(COMPV_CORE_LMSER_ACCESSIBILITY_BUILD_SAMPLES_PER_THREAD, (width << 1)) // At least #2 lines per thread
	));

	// A priority queue of boundary pixels, where priority is minus the grey - level.
	// These pixels can be thought of as partially flooded pixels in the sense that
	// water has access to the pixel in question, but has either not yet entered, or
	// not yet explored all the edges out from the pixel. Along with the pixel id,
	// an edge number indicating the next edge to be explored can be stored.
	std::vector<int32_t, CompVAllocatorNoDefaultConstruct<int32_t> > boundaryPixels[256];
	int32_t current_priority = LMSER_HIGHEST_GREYLEVEL;

	// A stack C of component information.Each entry holds the pixels in a component
	// and / or the first and second order moments of the pixels in the component,
	// as well as the size history of the component and the current grey - level
	// at which the component is being processed.The maximum number of entries
	// on the stack will be the number of grey - levels.
	std::vector<CompVConnectedComponentLmserRef, CompVAllocatorNoDefaultConstruct<CompVConnectedComponentLmserRef> > stackC;

	// The heap could be dynamically allocated and implemented as linked lists, but
	// we prefer to count the number pixels at each grey - level in a single pre - sweep of
	// the image, just like the standard algorithm does. This allows us to both preallocate
	// and use fixed arrays for the stacks without stacks ever running into one
	// another.The total number of entries in the stacks is the number of pixels plus
	// the number of grey - levels(due to one stop - element for each stack).
	CompVConnectedComponentLabelingLMSERStackMem& stackMem = result_->stackMem();
	CompVConnectedComponentLmserRef stackMemRef_;
	COMPV_CHECK_CODE_RETURN(stackMem.requestNewItem(&stackMemRef_));

	// 1. Clear the accessible pixel mask, the heap of boundary pixels and the component
	// 	stack.Push a dummy - component onto the stack, with grey - level higher
	// 	than any allowed in the image.
	stackMemRef_->greyLevel = LMSER_HIGHEST_GREYLEVEL;
	stackC.push_back(stackMemRef_);

	// 2. Make the source pixel(with its first edge) the current pixel, mark it as
	// 	accessible and store the grey - level of it in the variable current_level.
	int32_t current_pixel = 0;
	int8_t current_edge = 0;
	ptr8uAccessibleRef[0] |= 1;
	uint8_t current_level = ptr8uPixelsRef[0];

__________________________step3__________________________:
	// 3. Push an empty component with current level onto the component stack.
	COMPV_CHECK_CODE_RETURN(stackMem.requestNewItem(&stackMemRef_, current_level));
	stackC.push_back(stackMemRef_);

	// 4. Explore the remaining edges to the neighbors of the current pixel, in order,
	// as follows : For each neighbor, check if the neighbor is already accessible. If it
	// is not, mark it as accessible and retrieve its grey - level.If the grey - level is not
	// lower than the current one, push it onto the heap of boundary pixels. If on
	// the other hand the grey - level is lower than the current one, enter the current
	// pixel back into the queue of boundary pixels for later processing(with the
	// next edge number), consider the new pixel and its grey - level and go to 3.
	do {
		const int16_t current_pixel_y = static_cast<int16_t>(current_pixel * stride_scale);
		const int16_t current_pixel_x = static_cast<int16_t>(current_pixel - (current_pixel_y * stride));		
		while (current_edge < maxEdges) {
			const uint8_t edge_mask = LMSER_EDGES_MASKS[current_edge];
			if ((ptr8uAccessibleRef[current_pixel] & edge_mask) == edge_mask) {
				const int32_t neighbor_pixel = current_pixel + LMSER_EDGES_OFFSETS[current_edge];
				if (!(ptr8uAccessibleRef[neighbor_pixel] & 1)) {
					ptr8uAccessibleRef[neighbor_pixel] |= 1;
					const uint8_t neighbor_level = ptr8uPixelsRef[neighbor_pixel];
					if (neighbor_level >= current_level) {
						boundaryPixels[neighbor_level].push_back(neighbor_pixel << 4);
						current_priority = COMPV_MATH_MIN(current_priority, neighbor_level);
					}
					else {
						boundaryPixels[current_level].push_back((current_pixel << 4) | ++current_edge);
						current_priority = COMPV_MATH_MIN(current_priority, current_level);
						current_edge = 0;
						current_pixel = neighbor_pixel;
						current_level = neighbor_level;
						LMSER_GOTO(step3);
					}
				}
			}

			++current_edge;
		} /* while (current_edge < maxEdges) */

		  // 5. Accumulate the current pixel to the component at the top of the stack(water
		  // 	saturates the current pixel).
		CompVConnectedComponentLmserRef& top = stackC.back();
		top->region.points.push_back(CompVPoint2DInt16(current_pixel_x, current_pixel_y));

		// 6. Pop the heap of boundary pixels. If the heap is empty, we are done. If the
		// 	returned pixel is at the same grey - level as the previous, go to 4.
		if (current_priority == LMSER_HIGHEST_GREYLEVEL) {
			LMSER_GOTO(we_are_done);
		}

		const int32_t boundaryPixel = boundaryPixels[current_priority].back();
		boundaryPixels[current_priority].pop_back();
		current_pixel = boundaryPixel >> 4;
		current_edge = boundaryPixel & 0x0f;

		for (; current_priority < LMSER_HIGHEST_GREYLEVEL && boundaryPixels[current_priority].empty(); ++current_priority)
			/* do nothing */;

		// 7. The returned pixel is at a higher grey - level, so we must now process all
		// 	components on the component stack until we reach the higher grey - level.
		// 	This is done with the ProcessStack sub - routine, see below.Then go to 4.
		const uint8_t new_pixel_grey_level = ptr8uPixelsRef[current_pixel];
		if (new_pixel_grey_level != current_level) {
			current_level = new_pixel_grey_level;

			///////////////////////////////////////////////////////
			// The ProcessStack sub - routine is as follows :
			// Sub - routine ProcessStack(new pixel grey level)
			///////////////////////////////////////////////////////
			do {
				// 1. Process component on the top of the stack.The next grey - level is the minimum
				// 	of new pixel grey level and the grey - level for the second component on
				// 	the stack.
				CompVConnectedComponentLmserRef top = stackC.back();
				stackC.pop_back();

				// 2. If new pixel grey level is smaller than the grey - level on the second component
				// on the stack, set the top of stack grey - level to new pixel grey level and return
				// from sub - routine(This occurs when the new pixel is at a grey - level for which
				// there is not yet a component instantiated, so we let the top of stack be that
				// level by just changing its grey - level.
				if (new_pixel_grey_level < stackC.back()->greyLevel) {
					COMPV_CHECK_CODE_RETURN(stackMem.requestNewItem(&stackMemRef_, new_pixel_grey_level));
					LMSER_MERGE(stackMemRef_, top);
					stackC.push_back(stackMemRef_);
					break;
				}

				// 3. Remove the top of stack and merge it into the second component on stack
				// as follows : Add the first and second moment accumulators together and / or
				// join the pixel lists.Either merge the histories of the components, or take the
				// history from the winner.Note here that the top of stack should be considered
				// one ’time - step’ back, so its current size is part of the history.Therefore the
				// top of stack would be the winner if its current size is larger than the previous
				// size of second on stack.
				CompVConnectedComponentLmserRef back = stackC.back();
				LMSER_MERGE(back, top);

			} while (new_pixel_grey_level > stackC.back()->greyLevel); // 4. If(new pixel grey level>top of stack grey level) go to 1.
		}
	} while (true);

__________________________we_are_done__________________________:
	const int input_area = width * height;
	const size_t min_area_ = static_cast<size_t>(input_area * minArea());
	const size_t max_area_ = static_cast<size_t>(input_area * maxArea());
	const double one_minus_min_diversity = 1.0 - minDiversity();
	const double one_minus_min_diversity_scale = 1.0 / one_minus_min_diversity;
	CompVConnectedComponentLabelingLMSER::stability(stackC.back(), delta(), min_area_, max_area_, maxVariation());
	CompVConnectedComponentLabelingLMSER::collect(stackC.back(), one_minus_min_diversity, one_minus_min_diversity_scale, vecRegions);

	*result = *result_;

	return COMPV_ERROR_CODE_S_OK;
}

void CompVConnectedComponentLabelingLMSER::stability(CompVConnectedComponentLmserRef& component, const int& delta, const size_t& min_area, const size_t& max_area, const double& max_variation)
{
	const int deltaPlus = (component->greyLevel + delta);
	CompVConnectedComponentLmserRef parent;
	for (parent = component; parent->parent && (parent->parent->greyLevel <= deltaPlus); parent = parent->parent)
		/* do nothing */;

	const double& aread_ = static_cast<double>(component->region.points.size());
	const size_t areai_ = component->region.points.size();
	component->variation = (parent->region.points.size() - aread_) / aread_;
	
	const int8_t stable_ = (!component->parent || (component->parent->variation >= component->variation)) &&
		(component->variation <= max_variation) && (min_area <= areai_ && areai_ <= max_area);

	CompVConnectedComponentLmserRef child_ = component->child;
	if (child_) {
		do {
			CompVConnectedComponentLabelingLMSER::stability(child_, delta, min_area, max_area, max_variation);
			component->stable |= (stable_ && (component->variation < child_->variation));
		} while ((child_ = child_->sister));
	}
	else {
		component->stable = stable_;
	}
}

void CompVConnectedComponentLabelingLMSER::collect(CompVConnectedComponentLmserRef& component, const double& one_minus_min_diversity, const double& one_minus_min_diversity_scale, CompVConnectedComponentLabelingRegionMserRefsVector& vecRegions)
{
	int8_t& stable_ = component->stable;
	if (stable_) {
		const double min_parent_area = (component->region.points.size() * one_minus_min_diversity_scale) + 0.5;
		for (CompVConnectedComponentLmserRef parent = component->parent;
			(parent && (parent->region.points.size() < min_parent_area) && (stable_ = (!parent->stable || (parent->variation > component->variation))));
			(parent = parent->parent)
			) /* do noting */;

		stable_ =
			stable_ &&
			(CompVConnectedComponentLabelingLMSER::checkCrit(
				component,
				((component->region.points.size() * one_minus_min_diversity) + 0.5), // max_child_area
				component->variation
			));

		if (stable_) {
			vecRegions.push_back(&component->region); // life tied to the result
		}
	}

	CompVConnectedComponentLmserRef child_ = component->child;
	while (child_) {
		CompVConnectedComponentLabelingLMSER::collect(child_, one_minus_min_diversity, one_minus_min_diversity_scale, vecRegions);
		child_ = child_->sister;
	}
}

bool CompVConnectedComponentLabelingLMSER::checkCrit(const CompVConnectedComponentLmserRef& component, const double& area, const double& variation)
{
	if (component->region.points.size() <= area) {
		return true;
	}
	if (component->stable && (component->variation < variation)) {
		return false;
	}

	CompVConnectedComponentLmserRef child_ = component->child;
	while (child_) {
		if (!CompVConnectedComponentLabelingLMSER::checkCrit(child_, area, variation)) {
			return false;
		}
		child_ = child_->sister;
	}

	return true;
}

COMPV_ERROR_CODE CompVConnectedComponentLabelingLMSER::newObj(CompVConnectedComponentLabelingPtrPtr ccl)
{
	COMPV_CHECK_CODE_RETURN(CompVCore::init());
	COMPV_CHECK_EXP_RETURN(!ccl, COMPV_ERROR_CODE_E_INVALID_PARAMETER);
	CompVConnectedComponentLabelingPtr _ccl = new CompVConnectedComponentLabelingLMSER();
	COMPV_CHECK_EXP_RETURN(!_ccl, COMPV_ERROR_CODE_E_OUT_OF_MEMORY);
	*ccl = *_ccl;
	return COMPV_ERROR_CODE_S_OK;
}

COMPV_NAMESPACE_END()
