/* Copyright (C) 2016-2018 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_CCL_H_)
#define _COMPV_BASE_CCL_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_caps.h"
#include "compv/base/compv_mat.h"

#include <map>
#include <vector>

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(ConnectedComponentLabeling)

typedef int32_t compv_ccl_indice_t; /* use to hold max(width, height) */

struct CompVConnectedComponentLabelingFactory {
	int id;
	const char* name;
	COMPV_ERROR_CODE(*newObj)(CompVConnectedComponentLabelingPtrPtr ccl);
};

/* Connected component labeling setters and getters */
enum {
	/* Common to all features */

	
	/* Light Speed Labeling */
	COMPV_LSL_ID,
	COMPV_LSL_SET_INT_TYPE,
	COMPV_LSL_TYPE_STD,
	COMPV_LSL_TYPE_RLC,
	COMPV_LSL_TYPE_XRLC,
	COMPV_LSL_TYPE_RLE,

	/* Contour Tracing approach by Fu Chang et al */
	COMPV_CT_ID,

	/* By Wan-Yu Chang et al.  */
	COMPV_CCIT_ID,

	/* By Di Stefano */
	COMPV_DISTEFANO_ID,

	/* Block Based with Decision Trees algorithm by Grana et al. */
	COMPV_BBDT_ID,

	/* the Scan Array Union Find algorithm by Wu et al. */
	COMPV_SAUF_ID,

	/* Configuration-Transition-Based algorithm by He et al. */
	COMPV_CTB_ID,

	/* The stripe-based algorithm by Zhao et al. */
	COMPV_SBLA_ID,

	/* Optimized Pixel Prediction by Grana et al. */
	COMPV_PRED_ID
};

struct CompVConnectedComponentLabelingResult {
	CompVMatPtr labels;
	compv_ccl_indice_t labels_count;
	compv_ccl_indice_t label_background;
	void reset() {
		labels = nullptr;
		labels_count = 0;
	}
};

// Class: CompVConnectedComponentLabeling
class COMPV_BASE_API CompVConnectedComponentLabeling : public CompVObj, public CompVCaps
{
protected:
	CompVConnectedComponentLabeling(int id);
public:
	virtual ~CompVConnectedComponentLabeling();
	COMPV_INLINE int id()const {
		return m_nId;
	}

	virtual COMPV_ERROR_CODE process(const CompVMatPtr& binar, CompVConnectedComponentLabelingResult& result) = 0;

	static COMPV_ERROR_CODE newObj(CompVConnectedComponentLabelingPtrPtr ccl, int id);
	static COMPV_ERROR_CODE addFactory(const CompVConnectedComponentLabelingFactory* factory);
	static const CompVConnectedComponentLabelingFactory* findFactory(int id);

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	static std::map<int, const CompVConnectedComponentLabelingFactory*> s_Factories;
	int m_nId;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_CCL_H_ */
