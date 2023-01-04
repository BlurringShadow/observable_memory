#pragma once

#include <mimalloc.h>
#include <vector>
#include <deque>
#include <forward_list>
#include <list>
#include <set>
#include <map>
#include <unordered_set>
#include <unordered_map>

#include <stdsharp/memory/memory.h>

#include "resource.h"

namespace observable_memory::mimalloc
{
    struct deleter
    {
        void operator()(void* p) const noexcept { mi_free(p); }

        constexpr void operator()(::std::nullptr_t) = delete;
    };

    template<typename T>
    struct proxy_allocator : ::std::reference_wrapper<T>
    {
        using ::std::reference_wrapper<T>::reference_wrapper;

        using traits = ::stdsharp::allocator_traits<T>;

        using pointer = typename traits::pointer;
        using const_pointer = typename traits::const_pointer;
        using void_pointer = typename traits::void_pointer;
        using const_void_pointer = typename traits::const_void_pointer;
        using value_type = typename traits::value_type;
        using size_type = typename traits::size_type;
        using difference_type = typename traits::difference_type;
        using propagate_on_container_copy_assignment =
            typename traits::propagate_on_container_copy_assignment;
        using propagate_on_container_move_assignment =
            typename traits::propagate_on_container_move_assignment;
        using propagate_on_container_swap = typename traits::propagate_on_container_swap;
        using is_always_equal = typename traits::is_always_equal;

        template<typename U>
        struct rebind
        {
            using other = proxy_allocator<U>;
        };

#define ALLOC_MEMBER_FUN(attr, name)                                                   \
    template<typename... Args>                                                         \
        requires requires(T t) { traits::name(t, ::std ::declval<Args>()...); }        \
    constexpr auto name(Args&&... args)                                                \
        const noexcept(noexcept(traits::name(this->get(), ::std::declval<Args>()...))) \
    {                                                                                  \
        return traits::name(this->get(), ::std::forward<Args>(args)...);               \
    }

        ALLOC_MEMBER_FUN([[nodiscard]], allocate)
        ALLOC_MEMBER_FUN(, deallocate)
        ALLOC_MEMBER_FUN([[nodiscard]], max_size)
        ALLOC_MEMBER_FUN([[nodiscard]], construct)
        ALLOC_MEMBER_FUN(, destroy)
        ALLOC_MEMBER_FUN([[nodiscard]], select_on_container_copy_construction)

#undef ALLOC_MEMBER_FUN

        template<typename... Args>
            requires requires(T t) { t.allocate_at_least(::std::declval<Args>()...); }
        constexpr auto allocate_at_least(Args&&... args) const
            noexcept(noexcept(this->get().allocate_at_least(::std::declval<Args>()...)))
        {
            return this->get().allocate_at_least(::std::forward<Args>(args)...);
        }

        template<typename OtherAlloc>
            requires requires(proxy_allocator instance, OtherAlloc other) { instance == other; }
        constexpr bool operator==(const proxy_allocator<OtherAlloc>& alloc) const
            noexcept(noexcept(*this == alloc.get()))
        {
            return *this == alloc.get();
        }

        template<typename OtherAlloc>
            requires ::std::invocable<::std::ranges::equal_to, T, OtherAlloc>
        constexpr bool operator==(const OtherAlloc& alloc) const
            noexcept(::stdsharp::nothrow_invocable<::std::ranges::equal_to, T, OtherAlloc>)
        {
            return ::std::ranges::equal_to{}(this->get(), alloc);
        }
    };

    template<typename T>
    proxy_allocator(T&) -> proxy_allocator<T>;

    template<typename T>
    using allocator = mi_stl_allocator<T>;
}