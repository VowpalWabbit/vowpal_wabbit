//
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
//-----------------------------------------------------------------------------

#include "hash.h"

namespace MURMUR_HASH_3
{

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

static inline uint32_t getblock(const uint32_t * p, int i)
{ return p[i];
}

}

//-----------------------------------------------------------------------------

uint64_t uniform_hash(const void * key, size_t len, uint64_t seed)
{ const uint8_t * data = (const uint8_t*)key;
  const int nblocks = (int)len / 4;

  uint32_t h1 = (uint32_t)seed;

  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  // --- body
  const uint32_t * blocks = (const uint32_t *)(data + nblocks * 4);

  for (int i = -nblocks; i; i++)
  { uint32_t k1 = MURMUR_HASH_3::getblock(blocks, i);

    k1 *= c1;
    k1 = ROTL32(k1, 15);
    k1 *= c2;

    h1 ^= k1;
    h1 = ROTL32(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;
  }

  // --- tail
  const uint8_t * tail = (const uint8_t*)(data + nblocks * 4);

  uint32_t k1 = 0;

  switch (len & 3)
  { case 3: k1 ^= tail[2] << 16;
    case 2: k1 ^= tail[1] << 8;
    case 1: k1 ^= tail[0];
      k1 *= c1; k1 = ROTL32(k1, 15); k1 *= c2; h1 ^= k1;
  }

  // --- finalization
  h1 ^= len;

  return MURMUR_HASH_3::fmix(h1);
}
