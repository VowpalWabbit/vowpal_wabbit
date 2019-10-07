/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 *///
// MurmurHash3, by Austin Appleby
//
// Originals at:
// http://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.cpp
// http://code.google.com/p/smhasher/source/browse/trunk/MurmurHash3.h
//
// Notes:
// 1) this code assumes we can read a 4-byte value from any address
//    without crashing (i.e non aligned access is supported). This is
//    not a problem on Intel/x86/AMD64 machines (including new Macs)
// 2) It produces different results on little-endian and big-endian machines.
//
// Adopted for VW and contributed by Ariel Faigon.
//

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - The x86 and x64 versions do _not_ produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with the
// non-native version will be less than optimal.
//----
#pragma once

#include "future_compat.h"

#include <sys/types.h>
#include <cstdint>

// All modern compilers will optimize this to the rotate intrinsic.
constexpr inline uint32_t rotl32(uint32_t x, int8_t r) noexcept
{
  return (x << r) | (x >> (32 - r));
}

namespace MURMUR_HASH_3
{
  //-----------------------------------------------------------------------------
  // Finalization mix - force all bits of a hash block to avalanche
  VW_STD14_CONSTEXPR static inline uint32_t fmix(uint32_t h) noexcept
  {
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

    return h;
  }

  //-----------------------------------------------------------------------------
  // Block read - if your platform needs to do endian-swapping or can only
  // handle aligned reads, do the conversion here
  constexpr static inline uint32_t getblock(const uint32_t * p, int i) noexcept
  {
    return p[i];
  }
}

VW_STD14_CONSTEXPR inline uint64_t uniform_hash(const void* key, size_t len, uint64_t seed)
{
  const uint8_t* data = (const uint8_t*)key;
  const int nblocks = (int)len / 4;

  uint32_t h1 = (uint32_t)seed;

  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  // --- body
  const uint32_t* blocks = (const uint32_t *)(data + nblocks * 4);

  for (int i = -nblocks; i; i++)
  {
    uint32_t k1 = MURMUR_HASH_3::getblock(blocks, i);

    k1 *= c1;
    k1 = rotl32(k1, 15);
    k1 *= c2;

    h1 ^= k1;
    h1 = rotl32(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;
  }

  // --- tail
  const uint8_t * tail = (const uint8_t*)(data + nblocks * 4);

  uint32_t k1 = 0;

  // The 'fall through' comments below silence the implicit-fallthrough warning introduced in GCC 7.
  // Once we move to C++17 these should be replaced with the [[fallthrough]] attribute.
  switch (len & 3u)
  {
  case 3:
    k1 ^= tail[2] << 16;
    // fall through
  case 2:
    k1 ^= tail[1] << 8;
    // fall through
  case 1: k1 ^= tail[0];
    k1 *= c1;
    k1 = rotl32(k1, 15);
    k1 *= c2; h1 ^= k1;
  default:
    break;
  }

  // --- finalization
  h1 ^= len;

  return MURMUR_HASH_3::fmix(h1);
}
