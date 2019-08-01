/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once
constexpr int quadratic_constant = 27942141;
constexpr int cubic_constant = 21791;
constexpr int cubic_constant2 = 37663;
constexpr int affix_constant = 13903957;
constexpr uint64_t constant = 11650396;

constexpr float probability_tolerance = 1e-5f;

// FNV-like hash constant for 32bit
// http://www.isthe.com/chongo/tech/comp/fnv/#FNV-param
constexpr uint32_t FNV_prime = 16777619;
