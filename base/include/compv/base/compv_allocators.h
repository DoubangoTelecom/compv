/* Copyright (C) 2011-2020 Doubango Telecom <https://www.doubango.org>
 * File author: Mamadou DIOP (Doubango Telecom, France).
 * License: GPLv3. For commercial license please contact us.
 * Source code: https://github.com/DoubangoTelecom/compv
 * WebSite: http://compv.org
 */
#if !defined(_COMPV_BASE_ALLOCATORS_H_)
#define _COMPV_BASE_ALLOCATORS_H_

#include "compv/base/compv_config.h"

#if COMPV_TBBMALLOC && 1 // To limit the amount of memory to allocate, use "CompVMem::setHeapLimit" -> https://software.intel.com/en-us/forums/intel-threading-building-blocks/topic/296353
#include "compv/base/tbbmalloc/scalable_allocator.h"
#define CompVAllocator tbb::scalable_allocator
#else
#define CompVAllocator std::allocator
#endif

#include <memory> /* std::allocator_traits */

COMPV_NAMESPACE_BEGIN()

template <typename T, typename A=CompVAllocator<T> >
class CompVAllocatorNoDefaultConstruct : public A {
    typedef std::allocator_traits<A> a_t;
public:
    template <typename U> struct rebind {
        using other =
        CompVAllocatorNoDefaultConstruct<
        U, typename a_t::template rebind_alloc<U>
        >;
    };
    
    using A::A;
    
    template <typename U>
    void construct(U* ptr) noexcept(std::is_nothrow_default_constructible<U>::value) {
#if 0 // do not call default copy-constructor (triggered by resize())
        ::new(static_cast<void*>(ptr)) U;
#endif
    }
    template <typename U, typename...Args>
    void construct(U* ptr, Args&&... args) {
        a_t::construct(static_cast<A&>(*this), ptr, std::forward<Args>(args)...);
    }
};

COMPV_NAMESPACE_END()

#endif /* _COMPV_BASE_ALLOCATORS_H_ */
