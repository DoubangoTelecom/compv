/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATCHERS_MATCHER_H_)
#define _COMPV_MATCHERS_MATCHER_H_

#include "compv/compv_config.h"
#include "compv/compv_common.h"
#include "compv/compv_obj.h"
#include "compv/compv_settable.h"
#include "compv/compv_array.h"

COMPV_NAMESPACE_BEGIN()

class CompVMatcher;

struct CompVMatcherFactory {
    int id;
    const char* name;
    COMPV_ERROR_CODE(*newObj)(CompVPtr<CompVMatcher* >* matcher);
};

/* Matchers setters and getters */
enum {
    /* Common to all matchers */
    // COMPV_MATCHER_SET_XXX_YYY,

    /* Brute force */
    COMPV_BRUTEFORCE_ID,
    COMPV_BRUTEFORCE_SET_INT32_KNN,
    COMPV_BRUTEFORCE_SET_INT32_NORM,
    COMPV_BRUTEFORCE_SET_BOOL_CROSS_CHECK,
    COMPV_BRUTEFORCE_NORM_HAMMING,
    COMPV_BRUTEFORCE_NORM_HAMMING2,
    COMPV_BRUTEFORCE_NORM_L2,

    /*  FLANN */
    COMPV_FLANN_ID,
};

// Class: CompVMatcher
class COMPV_API CompVMatcher : public CompVObj, public CompVSettable
{
protected:
    CompVMatcher();
public:
    virtual ~CompVMatcher();
    static COMPV_ERROR_CODE init();
    static COMPV_ERROR_CODE addFactory(const CompVMatcherFactory* factory);
    static const CompVMatcherFactory* findFactory(int matcherId);
    static COMPV_ERROR_CODE newObj(int matcherId, CompVPtr<CompVMatcher* >* matcher);
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVArray<uint8_t>* >&queryDescriptions, const CompVPtr<CompVArray<uint8_t>* >&trainDescriptions, CompVPtr<CompVArray<CompVDMatch>* >* matches) = 0;

private:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    static std::map<int, const CompVMatcherFactory*> s_Factories;
    COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATCHERS_MATCHER_H_ */
