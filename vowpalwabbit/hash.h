/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once

#include <sys/types.h>  // defines size_t

// Platform-specific functions and macros
#if defined(_MSC_VER)                       // Microsoft Visual Studio
#   include <stdint.h>

#   include <stdlib.h>
#   define ROTL32(x,y)  _rotl(x,y)
#   define BIG_CONSTANT(x) (x)

#else                                       // Other compilers
#   include <stdint.h>   // defines uint32_t etc

inline uint32_t rotl32(uint32_t x, int8_t r)
{
  return (x << r) | (x >> (32 - r));
}

#   define ROTL32(x,y)     rotl32(x,y)
#   define BIG_CONSTANT(x) (x##LLU)

#endif                                      // !defined(_MSC_VER)

namespace MURMUR_HASH_3
{

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

static inline uint32_t fmix(uint32_t h)
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}
}

const uint32_t hash_base = 0;

uint32_t uniform_hash(const void *key, size_t length, uint32_t seed);
