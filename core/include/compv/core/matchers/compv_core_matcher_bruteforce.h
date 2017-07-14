/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_CORE_MATCHERS_BRUTEFORCE_H_)
#define _COMPV_CORE_MATCHERS_BRUTEFORCE_H_

#include "compv/core/compv_core_config.h"
#include "compv/core/compv_core_common.h"
#include "compv/base/compv_matchers.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

COMPV_OBJECT_DECLARE_PTRS(MatcherBruteForce)

// Class: CompVMatcher
class COMPV_CORE_API CompVMatcherBruteForce : public CompVMatcher
{
protected:
    CompVMatcherBruteForce();
public:
	COMPV_OBJECT_GET_ID(CompVMatcherBruteForce);

    virtual ~CompVMatcherBruteForce();
    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
    virtual COMPV_ERROR_CODE get(int id, const void** valuePtrPtr, size_t valueSize) override /*Overrides(CompVCaps)*/;
    virtual COMPV_ERROR_CODE process(const CompVMatPtr &queryDescriptions, const CompVMatPtr &trainDescriptions, CompVMatPtrPtr matches) override /* Overrides(CompVMatcher) */;
    static COMPV_ERROR_CODE newObj(CompVMatcherPtrPtr matcher);

private:
    static COMPV_ERROR_CODE processAt(int queryIdxStart, size_t count, const CompVMatPtr& queryDescriptions, const CompVMatPtr& trainDescriptions, CompVMatPtr& matches);
private:
	COMPV_VS_DISABLE_WARNINGS_BEGIN(4251 4267)
    bool m_bCrossCheck;
    int m_nNormType;
	int m_nKNN;
	COMPV_VS_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_CORE_MATCHERS_BRUTEFORCE_H_ */
