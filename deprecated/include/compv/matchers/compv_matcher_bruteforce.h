/* Copyright (C) 2016-2017 Doubango Telecom <https://www.doubango.org>
* File author: Mamadou DIOP (Doubango Telecom, France).
* License: GPLv3. For commercial license please contact us.
* Source code: https://github.com/DoubangoTelecom/compv
* WebSite: http://compv.org
*/
#if !defined(_COMPV_MATCHERS_MATCHER_BRUTEFORCE_H_)
#define _COMPV_MATCHERS_MATCHER_BRUTEFORCE_H_

#include "compv/compv_config.h"
#include "compv/compv_array.h"
#include "compv/matchers/compv_matcher.h"

#if defined(_COMPV_API_H_)
#error("This is a private file and must not be part of the API")
#endif

COMPV_NAMESPACE_BEGIN()

// Class: CompVMatcher
class COMPV_API CompVMatcherBruteForce : public CompVMatcher
{
protected:
    CompVMatcherBruteForce();
public:
    virtual COMPV_INLINE const char* getObjectId() {
        return "CompVMatcherBruteForce";
    };
    virtual ~CompVMatcherBruteForce();
    // override CompVSettable::set
    virtual COMPV_ERROR_CODE set(int id, const void* valuePtr, size_t valueSize);
    // override CompVSettable::get
    virtual COMPV_ERROR_CODE get(int id, const void*& valuePtr, size_t valueSize);
    // override CompVMatcher::process
    virtual COMPV_ERROR_CODE process(const CompVPtr<CompVArray<uint8_t>* >&queryDescriptions, const CompVPtr<CompVArray<uint8_t>* >&trainDescriptions, CompVPtr<CompVArray<CompVDMatch>* >* matches);

    static COMPV_ERROR_CODE newObj(CompVPtr<CompVMatcher* >* matcher);

private:
    static COMPV_ERROR_CODE processAt(size_t queryIdxStart, size_t count, const CompVArray<uint8_t>* queryDescriptions, const CompVArray<uint8_t>* trainDescriptions, CompVArray<CompVDMatch>* matches);
private:
    COMPV_DISABLE_WARNINGS_BEGIN(4251 4267)
    bool m_bCrossCheck;
    int32_t m_nNormType;
    int32_t m_nKNN;
    COMPV_DISABLE_WARNINGS_END()
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_MATCHERS_MATCHER_BRUTEFORCE_H_ */
