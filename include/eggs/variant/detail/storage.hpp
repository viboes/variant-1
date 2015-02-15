//! \file eggs/variant/detail/storage.hpp
// Eggs.Variant
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2014
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef EGGS_VARIANT_DETAIL_STORAGE_HPP
#define EGGS_VARIANT_DETAIL_STORAGE_HPP

#include <eggs/variant/detail/pack.hpp>
#include <eggs/variant/detail/visitor.hpp>

#include <cstddef>
#include <new>
#include <type_traits>
#include <typeinfo>
#include <utility>

#include <eggs/variant/detail/config/prefix.hpp>

namespace eggs { namespace variants { namespace detail
{
    template <typename Ts, bool IsTriviallyDestructible>
    struct _union;

#if EGGS_CXX11_HAS_UNRESTRICTED_UNIONS
    ///////////////////////////////////////////////////////////////////////////
    template <typename Ts, bool IsTriviallyDestructible>
    struct _basic_union;

    template <bool IsTriviallyDestructible>
    struct _basic_union<pack<>, IsTriviallyDestructible>
    {
        EGGS_CXX11_CONSTEXPR _basic_union() EGGS_CXX11_NOEXCEPT {}
    };

    template <typename T, typename ...Ts>
    struct _basic_union<pack<T, Ts...>, true>
    {
        EGGS_CXX11_CONSTEXPR _basic_union(
            std::integral_constant<std::size_t, 0>) EGGS_CXX11_NOEXCEPT
          : _empty{}
        {}

        template <typename ...Args>
        EGGS_CXX11_CONSTEXPR _basic_union(
            std::integral_constant<std::size_t, 1>, Args&&... args)
          : _head(std::forward<Args>(args)...)
        {}

        template <std::size_t I, typename ...Args>
        EGGS_CXX11_CONSTEXPR _basic_union(
            std::integral_constant<std::size_t, I>, Args&&... args)
          : _tail(std::integral_constant<std::size_t, I - 1>{},
                std::forward<Args>(args)...)
        {}

        EGGS_CXX14_CONSTEXPR void* target() EGGS_CXX11_NOEXCEPT
        {
            return &_empty;
        }

        EGGS_CXX11_CONSTEXPR void const* target() const EGGS_CXX11_NOEXCEPT
        {
            return &_empty;
        }

    private:
        union
        {
            char _empty;
            T _head;
            _basic_union<pack<Ts...>, true> _tail;
        };
    };

    template <typename T, typename ...Ts>
    struct _basic_union<pack<T, Ts...>, false>
    {
        EGGS_CXX11_CONSTEXPR _basic_union(
            std::integral_constant<std::size_t, 0>) EGGS_CXX11_NOEXCEPT
          : _empty{}
        {}

        template <typename ...Args>
        EGGS_CXX11_CONSTEXPR _basic_union(
            std::integral_constant<std::size_t, 1>, Args&&... args)
          : _head(std::forward<Args>(args)...)
        {}

        template <std::size_t I, typename ...Args>
        EGGS_CXX11_CONSTEXPR _basic_union(
            std::integral_constant<std::size_t, I>, Args&&... args)
          : _tail(std::integral_constant<std::size_t, I - 1>{},
                std::forward<Args>(args)...)
        {}

        ~_basic_union() {}

        EGGS_CXX14_CONSTEXPR void* target() EGGS_CXX11_NOEXCEPT
        {
            return &_empty;
        }

        EGGS_CXX11_CONSTEXPR void const* target() const EGGS_CXX11_NOEXCEPT
        {
            return &_empty;
        }

    private:
        union
        {
            char _empty;
            T _head;
            _basic_union<pack<Ts...>, false> _tail;
        };
    };

    ///////////////////////////////////////////////////////////////////////////
    template <typename Ts, bool IsTriviallyDestructible>
    struct _union
      : _basic_union<Ts, IsTriviallyDestructible>
    {
        using base_type = _basic_union<Ts, IsTriviallyDestructible>;

        EGGS_CXX11_CONSTEXPR _union() EGGS_CXX11_NOEXCEPT
          : base_type{std::integral_constant<std::size_t, 0>{}}
        {}

        template <std::size_t I, typename ...Args>
        EGGS_CXX11_CONSTEXPR _union(
            std::integral_constant<std::size_t, I> which, Args&&... args)
          : base_type(which, std::forward<Args>(args)...)
        {}

        using base_type::target;
    };
#else
    ///////////////////////////////////////////////////////////////////////////
    namespace conditionally_deleted
    {
        template <bool CopyCnstr, bool MoveCnstr = CopyCnstr>
        struct cnstr
        {};

#  if EGGS_CXX11_HAS_DEFAULTED_FUNCTIONS && EGGS_CXX11_HAS_DELETED_FUNCTIONS
        template <>
        struct cnstr<true, false>
        {
            cnstr() EGGS_CXX11_NOEXCEPT = default;
            cnstr(cnstr const&) EGGS_CXX11_NOEXCEPT = delete;
            cnstr(cnstr&&) EGGS_CXX11_NOEXCEPT = default;
            cnstr& operator=(cnstr const&) EGGS_CXX11_NOEXCEPT = default;
            cnstr& operator=(cnstr&&) EGGS_CXX11_NOEXCEPT = default;
        };

        template <>
        struct cnstr<false, true>
        {
            cnstr() EGGS_CXX11_NOEXCEPT = default;
            cnstr(cnstr const&) EGGS_CXX11_NOEXCEPT = default;
            cnstr(cnstr&&) EGGS_CXX11_NOEXCEPT = delete;
            cnstr& operator=(cnstr const&) EGGS_CXX11_NOEXCEPT = default;
            cnstr& operator=(cnstr&&) EGGS_CXX11_NOEXCEPT = default;
        };

        template <>
        struct cnstr<true, true>
        {
            cnstr() EGGS_CXX11_NOEXCEPT = default;
            cnstr(cnstr const&) EGGS_CXX11_NOEXCEPT = delete;
            cnstr(cnstr&&) EGGS_CXX11_NOEXCEPT = delete;
            cnstr& operator=(cnstr const&) EGGS_CXX11_NOEXCEPT = default;
            cnstr& operator=(cnstr&&) EGGS_CXX11_NOEXCEPT = default;
        };
#  endif

        template <bool CopyAssign, bool MoveAssign = CopyAssign>
        struct assign
        {};

#  if EGGS_CXX11_HAS_DEFAULTED_FUNCTIONS && EGGS_CXX11_HAS_DELETED_FUNCTIONS
        template <>
        struct assign<true, false>
        {
            assign() EGGS_CXX11_NOEXCEPT = default;
            assign& operator=(assign const&) EGGS_CXX11_NOEXCEPT = delete;
            assign& operator=(assign&&) EGGS_CXX11_NOEXCEPT = default;
        };

        template <>
        struct assign<false, true>
        {
            assign() EGGS_CXX11_NOEXCEPT = default;
            assign& operator=(assign const&) EGGS_CXX11_NOEXCEPT = default;
            assign& operator=(assign&&) EGGS_CXX11_NOEXCEPT = delete;
        };

        template <>
        struct assign<true, true>
        {
            assign() EGGS_CXX11_NOEXCEPT = default;
            assign& operator=(assign const&) EGGS_CXX11_NOEXCEPT = delete;
            assign& operator=(assign&&) EGGS_CXX11_NOEXCEPT = delete;
        };
#  endif
    }

    template <bool CopyCnstr, bool MoveCnstr = CopyCnstr>
    using conditionally_deleted_cnstr =
        conditionally_deleted::cnstr<CopyCnstr, MoveCnstr>;

        template <bool CopyAssign, bool MoveAssign = CopyAssign>
    using conditionally_deleted_assign =
        conditionally_deleted::assign<CopyAssign, MoveAssign>;

    ///////////////////////////////////////////////////////////////////////////
#  if EGGS_CXX11_STD_HAS_ALIGNED_UNION
    using std::aligned_union;
#  else
    template <std::size_t ...Vs>
    struct _static_max;

    template <std::size_t V0>
    struct _static_max<V0>
      : std::integral_constant<std::size_t, V0>
    {};

    template <std::size_t V0, std::size_t V1, std::size_t ...Vs>
    struct _static_max<V0, V1, Vs...>
      : _static_max<V0 < V1 ? V1 : V0, Vs...>
    {};

    template <std::size_t Len, typename ...Types>
    struct aligned_union
      : std::aligned_storage<
            _static_max<Len, sizeof(Types)...>::value
          , _static_max<std::alignment_of<Types>::value...>::value
        >
    {};
#  endif

    ///////////////////////////////////////////////////////////////////////////
    template <typename ...Ts, bool IsTriviallyDestructible>
    struct _union<pack<Ts...>, IsTriviallyDestructible>
      : conditionally_deleted_cnstr<
            !all_of<pack<std::is_copy_constructible<Ts>...>>::value
          , !all_of<pack<std::is_move_constructible<Ts>...>>::value
        >
      , conditionally_deleted_assign<
            !all_of<pack<std::is_copy_assignable<Ts>...>>::value
          , !all_of<pack<std::is_move_assignable<Ts>...>>::value
        >
    {
        _union() EGGS_CXX11_NOEXCEPT
        {}

        template <std::size_t I, typename ...Args>
        _union(std::integral_constant<std::size_t, I> which, Args&&... args)
        {
            ::new (target()) typename at_index<
                I, pack<empty, Ts...>
            >::type(std::forward<Args>(args)...);
        }

        void* target() EGGS_CXX11_NOEXCEPT
        {
            return &_buffer;
        }

        void const* target() const EGGS_CXX11_NOEXCEPT
        {
            return &_buffer;
        }

        typename aligned_union<0, Ts...>::type _buffer;
    };
#endif

    ///////////////////////////////////////////////////////////////////////////
    template <
        typename Ts, typename Union
      , bool TriviallyCopyable, bool TriviallyDestructible>
    struct _storage;

    template <typename ...Ts, typename Union>
    struct _storage<pack<Ts...>, Union, true, true>
    {
    public:
        EGGS_CXX11_CONSTEXPR _storage() EGGS_CXX11_NOEXCEPT
          : _which{0}
        {}

#if EGGS_CXX11_HAS_DEFAULTED_FUNCTIONS
        _storage(_storage const& rhs) = default;
        _storage(_storage&& rhs) = default;
#endif

        template <std::size_t I, typename ...Args>
        EGGS_CXX11_CONSTEXPR _storage(
            std::integral_constant<std::size_t, I> which, Args&&... args)
          : _which{which}
          , _buffer{which, std::forward<Args>(args)...}
        {}

        template <std::size_t I, typename ...Args>
        void emplace(std::integral_constant<std::size_t, I> which, Args&&... args)
        {
            ::new (target()) typename at_index<
                I, pack<empty, Ts...>
            >::type(std::forward<Args>(args)...);
            _which = which;
        }

#if EGGS_CXX11_HAS_DEFAULTED_FUNCTIONS
        _storage& operator=(_storage const& rhs) = default;
        _storage& operator=(_storage&& rhs) = default;
#endif

        void swap(_storage& rhs)
        {
            std::swap(*this, rhs);
        }

        EGGS_CXX11_CONSTEXPR std::size_t which() const
        {
            return _which;
        }

        EGGS_CXX14_CONSTEXPR void* target()
        {
            return _buffer.target();
        }

        EGGS_CXX11_CONSTEXPR void const* target() const
        {
            return _buffer.target();
        }

    protected:
        std::size_t _which;
        Union _buffer;
    };

    template <typename ...Ts, typename Union>
    struct _storage<pack<Ts...>, Union, false, true>
      : _storage<pack<Ts...>, Union, true, true>
    {
        using base_type = _storage<pack<Ts...>, Union, true, true>;

#if EGGS_CXX11_HAS_DEFAULTED_FUNCTIONS
        EGGS_CXX11_CONSTEXPR _storage() EGGS_CXX11_NOEXCEPT = default;
#else
        EGGS_CXX11_CONSTEXPR _storage() EGGS_CXX11_NOEXCEPT
          : base_type{}
        {}
#endif

        _storage(_storage const& rhs)
#if EGGS_CXX11_STD_HAS_IS_NOTHROW_TRAITS
            EGGS_CXX11_NOEXCEPT_IF(all_of<pack<
                std::is_nothrow_copy_constructible<Ts>...
            >>::value)
#endif
          : base_type{}
        {
            detail::copy_construct{}(
                pack<empty, Ts...>{}, rhs._which
              , target(), rhs.target()
            );
            _which = rhs._which;
        }

        _storage(_storage&& rhs)
#if EGGS_CXX11_STD_HAS_IS_NOTHROW_TRAITS
            EGGS_CXX11_NOEXCEPT_IF(all_of<pack<
                std::is_nothrow_move_constructible<Ts>...
            >>::value)
#endif
          : base_type{}
        {
            detail::move_construct{}(
                pack<empty, Ts...>{}, rhs._which
              , target(), rhs.target()
            );
            _which = rhs._which;
        }

        template <std::size_t I, typename ...Args>
        EGGS_CXX11_CONSTEXPR _storage(
            std::integral_constant<std::size_t, I> which, Args&&... args)
          : base_type{which, std::forward<Args>(args)...}
        {}

        template <std::size_t I, typename ...Args>
        void emplace(std::integral_constant<std::size_t, I> which, Args&&... args)
        {
            _which = 0;

            base_type::emplace(which, std::forward<Args>(args)...);
        }

        _storage& operator=(_storage const& rhs)
#if EGGS_CXX11_STD_HAS_IS_NOTHROW_TRAITS
            EGGS_CXX11_NOEXCEPT_IF(all_of<pack<
                std::is_nothrow_copy_assignable<Ts>...
              , std::is_nothrow_copy_constructible<Ts>...
            >>::value)
#endif
        {
            if (_which == rhs._which)
            {
                detail::copy_assign{}(
                    pack<empty, Ts...>{}, _which
                  , target(), rhs.target()
                );
            } else {
                _which = 0;

                detail::copy_construct{}(
                    pack<empty, Ts...>{}, rhs._which
                  , target(), rhs.target()
                );
                _which = rhs._which;
            }
            return *this;
        }

        _storage& operator=(_storage&& rhs)
#if EGGS_CXX11_STD_HAS_IS_NOTHROW_TRAITS
            EGGS_CXX11_NOEXCEPT_IF(all_of<pack<
                std::is_nothrow_move_assignable<Ts>...
              , std::is_nothrow_move_constructible<Ts>...
            >>::value)
#endif
        {
            if (_which == rhs._which)
            {
                detail::move_assign{}(
                    pack<empty, Ts...>{}, _which
                  , target(), rhs.target()
                );
            } else {
                _which = 0;

                detail::move_construct{}(
                    pack<empty, Ts...>{}, rhs._which
                  , target(), rhs.target()
                );
                _which = rhs._which;
            }
            return *this;
        }

        void swap(_storage& rhs)
        {
            if (_which == rhs._which)
            {
                detail::swap{}(
                    pack<empty, Ts...>{}, _which
                  , target(), rhs.target()
                );
            } else if (_which == 0) {
                *this = std::move(rhs);
                rhs._which = 0;
            } else if (rhs._which == 0) {
                rhs = std::move(*this);
                _which = 0;
            } else {
                std::swap(*this, rhs);
            }
        }

        using base_type::which;
        using base_type::target;

    protected:
        using base_type::_which;
        using base_type::_buffer;
    };

    template <typename ...Ts, typename Union>
    struct _storage<pack<Ts...>, Union, false, false>
      : _storage<pack<Ts...>, Union, false, true>
    {
        using base_type = _storage<pack<Ts...>, Union, false, true>;

#if EGGS_CXX11_HAS_DEFAULTED_FUNCTIONS
        EGGS_CXX11_CONSTEXPR _storage() EGGS_CXX11_NOEXCEPT = default;
        _storage(_storage const& rhs) = default;
        _storage(_storage&& rhs) = default;
#else
        EGGS_CXX11_CONSTEXPR _storage() EGGS_CXX11_NOEXCEPT
          : base_type{}
        {}

        _storage(_storage const& rhs)
#if EGGS_CXX11_STD_HAS_IS_NOTHROW_TRAITS
            EGGS_CXX11_NOEXCEPT_IF(
                std::is_nothrow_copy_constructible<base_type>::value
            )
#endif
          : base_type{static_cast<base_type const&>(rhs)}
        {}

        _storage(_storage&& rhs)
#if EGGS_CXX11_STD_HAS_IS_NOTHROW_TRAITS
            EGGS_CXX11_NOEXCEPT_IF(
                std::is_nothrow_move_constructible<base_type>::value
            )
#endif
          : base_type{static_cast<base_type&&>(rhs)}
        {}
#endif

        template <std::size_t I, typename ...Args>
        _storage(std::integral_constant<std::size_t, I> which, Args&&... args)
        {
            emplace(which, std::forward<Args>(args)...);
        }

        ~_storage()
        {
            _destroy();
        }

        template <std::size_t I, typename ...Args>
        void emplace(std::integral_constant<std::size_t, I> which, Args&&... args)
        {
            _destroy();
            base_type::emplace(which, std::forward<Args>(args)...);
        }

        _storage& operator=(_storage const& rhs)
#if EGGS_CXX11_STD_HAS_IS_NOTHROW_TRAITS
            EGGS_CXX11_NOEXCEPT_IF(all_of<pack<
                std::is_nothrow_copy_assignable<Ts>...
              , std::is_nothrow_copy_constructible<Ts>...
            >>::value)
#endif
        {
            if (_which != rhs._which)
            {
                _destroy();
            }
            base_type::operator=(rhs);
            return *this;
        }

        _storage& operator=(_storage&& rhs)
#if EGGS_CXX11_STD_HAS_IS_NOTHROW_TRAITS
            EGGS_CXX11_NOEXCEPT_IF(all_of<pack<
                std::is_nothrow_move_assignable<Ts>...
              , std::is_nothrow_move_constructible<Ts>...
            >>::value)
#endif
        {
            if (_which != rhs._which)
            {
                _destroy();
            }
            base_type::operator=(std::move(rhs));
            return *this;
        }

        void swap(_storage& rhs)
        {
            if (_which == 0)
            {
                base_type::swap(rhs);
                rhs._destroy();
            } else if (rhs._which == 0) {
                base_type::swap(rhs);
                _destroy();
            } else {
                base_type::swap(rhs);
            }
        }

        using base_type::which;
        using base_type::target;

    protected:
        void _destroy()
        {
            detail::destroy{}(
                pack<empty, Ts...>{}, _which
              , target()
            );
        }

    protected:
        using base_type::_which;
        using base_type::_buffer;
    };

    template <typename ...Ts>
    using storage = _storage<
        pack<Ts...>
      , _union<
            pack<Ts...>
#  if EGGS_CXX11_STD_HAS_IS_TRIVIALLY_DESTRUCTIBLE
          , all_of<pack<std::is_trivially_destructible<Ts>...>>::value
#  else
          , all_of<pack<std::is_pod<Ts>...>>::value
#  endif
        >
#if EGGS_CXX11_STD_HAS_IS_TRIVIALLY_DESTRUCTIBLE
#  if EGGS_CXX11_STD_HAS_IS_TRIVIALLY_COPYABLE
      , all_of<pack<std::is_trivially_copyable<Ts>...>>::value
      , all_of<pack<std::is_trivially_destructible<Ts>...>>::value
#  else
      , all_of<pack<std::is_pod<Ts>...>>::value
      , all_of<pack<std::is_trivially_destructible<Ts>...>>::value
#  endif
#else
      , all_of<pack<std::is_pod<Ts>...>>::value
      , all_of<pack<std::is_pod<Ts>...>>::value
#endif
    >;
}}}

#include <eggs/variant/detail/config/suffix.hpp>

#endif /*EGGS_VARIANT_DETAIL_STORAGE_HPP*/
