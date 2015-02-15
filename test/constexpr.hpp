// Eggs.Variant
//
// Copyright Agustin K-ballo Berge, Fusion Fenix 2014
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef EGGS_VARIANT_TEST_CONSTEXPR_HPP
#define EGGS_VARIANT_TEST_CONSTEXPR_HPP

#include <initializer_list>

#include <eggs/variant/detail/config/prefix.hpp>

#if EGGS_CXX11_HAS_CONSTEXPR
struct Constexpr
{
    constexpr Constexpr() {}
    constexpr Constexpr(int) {}
    constexpr Constexpr(std::initializer_list<int>) {}

    constexpr Constexpr(Constexpr const&) {} // not trivially copyable
};
#endif

#endif /*EGGS_VARIANT_TEST_CONSTEXPR_HPP*/
