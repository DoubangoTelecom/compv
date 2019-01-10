/* Copyright (C) 2016-2019 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_BASE_MATCHERS_H_)
#define _COMPV_BASE_MATCHERS_H_

#include "compv/base/compv_config.h"
#include "compv/base/compv_common.h"
#include "compv/base/compv_obj.h"
#include "compv/base/compv_caps.h"
#include "compv/base/compv_mat.h"

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(Matcher)

struct CompVMatcherFactory {
	int id;
	const char* name;
	COMPV_ERROR_CODE(*newObjMatcher)(CompVMatcherPtrPtr matcher);
};

/* Matchers setters and getters */
enum {
	/* Common to all matchers */
	// COMPV_MATCHER_SET_XXX_YYY,

	/* Brute force */
	COMPV_BRUTEFORCE_ID,
	COMPV_BRUTEFORCE_SET_INT_KNN,
	COMPV_BRUTEFORCE_SET_INT_NORM,
	COMPV_BRUTEFORCE_SET_BOOL_CROSS_CHECK,
	COMPV_BRUTEFORCE_NORM_HAMMING,
	COMPV_BRUTEFORCE_NORM_HAMMING2,
	COMPV_BRUTEFORCE_NORM_L2,

	/*  FLANN */
	COMPV_FLANN_ID,
};

// Class: CompVMatcher
class COMPV_BASE_API CompVMatcher : public CompVObj, public CompVCaps
{
protected:
	CompVMatcher();
public:
	virtual ~CompVMatcher();
	static COMPV_ERROR_CODE init();
	static COMPV_ERROR_CODE addFactory(const CompVMatcherFactory* factory);
	static const CompVMatcherFactory* findFactory(int matcherId);
	static COMPV_ERROR_CODE newObj(CompVMatcherPtrPtr matcher, int matcherId);
	virtual COMPV_ERROR_CODE process(const CompVMatPtr &queryDescriptions, const CompVMatPtr &trainDescriptions, CompVMatPtrPtr matches) = 0;

private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
	static std::map<int, const CompVMatcherFactory*> s_Factories;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_MATCHERS_H_ */
