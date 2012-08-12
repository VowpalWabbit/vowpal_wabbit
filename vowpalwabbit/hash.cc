// Tweaked for VW and contributed by Ariel Faigon.
// Original at: http://murmurhash.googlepages.com/
//
// Incorporates MurmurHash2, by Austin Appleby
// Incorporates MurmurHash3, by Austin Appleby
//
// Note - This code makes a few assumptions about how your machine behaves:
//
// 1. We can read a 4-byte value from any address without crashing
//    (i.e non aligned access is supported) - this is not a problem
//    on Intel/x86/AMD64 machines (including new Macs)
// 2. sizeof(int) == 4
//
// And it has a few limitations -
//  1. It will not work incrementally.
//  2. It will not produce the same results on little-endian and
//     big-endian machines.
//

// Platform-specific functions and macros
// Microsoft Visual Studio
#if defined(_MSC_VER)
    typedef unsigned char uint8_t;
    typedef unsigned long uint32_t; 
    typedef unsigned __int64 uint64_t;
// Other compilers
#else   // defined(_MSC_VER)
#  include <stdint.h>	/* defines uint32_t etc */
#endif // !defined(_MSC_VER)

#include <sys/types.h>	/* defines size_t */

#ifdef MURMUR3  /* compile w/ -D MURMUR3 to default to this hash */

//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - The x86 and x64 versions do _not_ produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with the
// non-native version will be less than optimal.

//-----------------------------------------------------------------------------
// Platform-specific functions and macros

// Microsoft Visual Studio

#if defined(_MSC_VER)

#define FORCE_INLINE	__forceinline

#include <stdlib.h>

#define ROTL32(x,y)	_rotl(x,y)
#define ROTL64(x,y)	_rotl64(x,y)

#define BIG_CONSTANT(x) (x)

// Other compilers

#else	// defined(_MSC_VER)

#define	FORCE_INLINE __attribute__((always_inline))

inline uint32_t rotl32 ( uint32_t x, int8_t r )
{
  return (x << r) | (x >> (32 - r));
}

inline uint64_t rotl64 ( uint64_t x, int8_t r )
{
  return (x << r) | (x >> (64 - r));
}

#define	ROTL32(x,y)	rotl32(x,y)
#define ROTL64(x,y)	rotl64(x,y)

#define BIG_CONSTANT(x) (x##LLU)

#endif // !defined(_MSC_VER)

//-----------------------------------------------------------------------------
// Block read - if your platform needs to do endian-swapping or can only
// handle aligned reads, do the conversion here

FORCE_INLINE uint32_t getblock ( const uint32_t * p, int i )
{
  return p[i];
}

FORCE_INLINE uint64_t getblock ( const uint64_t * p, int i )
{
  return p[i];
}

//-----------------------------------------------------------------------------
// Finalization mix - force all bits of a hash block to avalanche

FORCE_INLINE uint32_t fmix ( uint32_t h )
{
  h ^= h >> 16;
  h *= 0x85ebca6b;
  h ^= h >> 13;
  h *= 0xc2b2ae35;
  h ^= h >> 16;

  return h;
}

//
// ariel: changed the interface to match vw
// removed all the functions we don't need (leave only the 32-bit hash)
// renamed MurmurHash3_x86_32 -> uniform_hash
//
uint32_t uniform_hash (const void * key, size_t len, uint32_t seed)
{
  const uint8_t * data = (const uint8_t*)key;
  const int nblocks = len / 4;

  uint32_t h1 = seed;

  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  //----------
  // body

  const uint32_t * blocks = (const uint32_t *)(data + nblocks*4);

  for(int i = -nblocks; i; i++)
  {
    uint32_t k1 = getblock(blocks,i);

    k1 *= c1;
    k1 = ROTL32(k1,15);
    k1 *= c2;
    
    h1 ^= k1;
    h1 = ROTL32(h1,13); 
    h1 = h1*5+0xe6546b64;
  }

  //----------
  // tail

  const uint8_t * tail = (const uint8_t*)(data + nblocks*4);

  uint32_t k1 = 0;

  switch(len & 3)
  {
  case 3: k1 ^= tail[2] << 16;
  case 2: k1 ^= tail[1] << 8;
  case 1: k1 ^= tail[0];
          k1 *= c1; k1 = ROTL32(k1,15); k1 *= c2; h1 ^= k1;
  };

  //----------
  // finalization

  h1 ^= len;

  h1 = fmix(h1);

  return h1;
}

//-----------------------------------------------------------------------------

#else

#define MIX(h,k,m) { k *= m; k ^= k >> r; k *= m; h *= m; h ^= k; }

uint32_t uniform_hash( const void *key, size_t len, uint32_t seed)
{
    // 'm' and 'r' are mixing constants generated offline.
    // They're not really 'magic', they just happen to work well.

    const unsigned int m = 0x5bd1e995;
    const int r = 24;

    // Initialize the hash to a 'random' value

    unsigned int h = seed ^ len;

    // Mix 4 bytes at a time into the hash

    const unsigned char * data = (const unsigned char *)key;

    while (len >= 4) {
	unsigned int k = *(unsigned int *)data;

	k *= m;
	k ^= k >> r;
	k *= m;

	h *= m;
	h ^= k;

	data += 4;
	len -= 4;
    }

    // Handle the last few bytes of the input array
    switch (len) {
	case 3: h ^= data[2] << 16;
	case 2: h ^= data[1] << 8;
	case 1: h ^= data[0];
		h *= m;
    };

    // Do a few final mixes of the hash to ensure the last few
    // bytes are well-incorporated.
    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

#endif /* MURMUR2 or MURMUR3 */

