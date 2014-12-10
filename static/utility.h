/*******************************************************************/
// Classes declared in this file are intended for internal use only. 
/*******************************************************************/

#pragma once
#include <stdint.h>
#include <sys/types.h>  /* defines size_t */

#ifdef WIN32
typedef unsigned __int64 u64;
typedef unsigned __int32 u32;
typedef unsigned __int16 u16;
typedef unsigned __int8  u8;
typedef signed __int64 i64;
typedef signed __int32 i32;
typedef signed __int16 i16;
typedef signed __int8  i8;
#else
typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t  u8;
typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;
#endif

typedef unsigned char    byte;

#include <string>
#include <stdint.h>
#include <math.h>

/*!
*  \addtogroup MultiWorldTestingCpp
*  @{
*/

MWT_NAMESPACE {

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
//-----------------------------------------------------------------------------
// MurmurHash3 was written by Austin Appleby, and is placed in the public
// domain. The author hereby disclaims copyright to this source code.

// Note - The x86 and x64 versions do _not_ produce the same results, as the
// algorithms are optimized for their respective platforms. You can still
// compile and run any of them on any platform, but your performance with the
// non-native version will be less than optimal.
//-----------------------------------------------------------------------------

// Platform-specific functions and macros
#if defined(_MSC_VER)                       // Microsoft Visual Studio
#   include <stdint.h>

#   include <stdlib.h>
#   define ROTL32(x,y)  _rotl(x,y)
#   define BIG_CONSTANT(x) (x)

#else                                       // Other compilers
#   include <stdint.h>   /* defines uint32_t etc */

	inline uint32_t rotl32(uint32_t x, int8_t r)
	{
		return (x << r) | (x >> (32 - r));
	}

#   define ROTL32(x,y)     rotl32(x,y)
#   define BIG_CONSTANT(x) (x##LLU)

#endif                                      // !defined(_MSC_VER)

struct murmur_hash {

	//-----------------------------------------------------------------------------
	// Block read - if your platform needs to do endian-swapping or can only
	// handle aligned reads, do the conversion here
private:
	static inline uint32_t getblock(const uint32_t * p, int i)
	{
		return p[i];
	}

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

	//-----------------------------------------------------------------------------
public:
	uint32_t uniform_hash(const void * key, size_t len, uint32_t seed)
	{
		const uint8_t * data = (const uint8_t*)key;
		const int nblocks = (int)len / 4;

		uint32_t h1 = seed;

		const uint32_t c1 = 0xcc9e2d51;
		const uint32_t c2 = 0x1b873593;

		// --- body
		const uint32_t * blocks = (const uint32_t *)(data + nblocks * 4);

		for (int i = -nblocks; i; i++) {
			uint32_t k1 = getblock(blocks, i);

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

		switch (len & 3) {
		case 3: k1 ^= tail[2] << 16;
		case 2: k1 ^= tail[1] << 8;
		case 1: k1 ^= tail[0];
			k1 *= c1; k1 = ROTL32(k1, 15); k1 *= c2; h1 ^= k1;
		}

		// --- finalization
		h1 ^= len;

		return fmix(h1);
	}
};

class HashUtils
{
public:
	static u64 Compute_Id_Hash(const std::string& unique_id)
	{
		size_t ret = 0;
		const char *p = unique_id.c_str();
		while (*p != '\0')
		if (*p >= '0' && *p <= '9')
			ret = 10 * ret + *(p++) - '0';
		else
		{
			murmur_hash foo;
			return foo.uniform_hash(unique_id.c_str(), unique_id.size(), 0);
		}
		return ret;
	}
};

const size_t max_int = 100000;
const float max_float = max_int;
const float min_float = 0.00001f;
const size_t max_digits = (size_t) roundf((float) (-log(min_float) / log(10.)));

class NumberUtils
{
public:
	static void Float_To_String(float f, char* str)
	{
		int x = (int)f;
		int d = (int)(fabs(f - x) * 100000);
		sprintf(str, "%d.%05d", x, d);
	}

	template<bool trailing_zeros>
	static void print_mantissa(char*& begin, float f)
	{ // helper for print_float
		char values[10];
		size_t v = (size_t)f;
		size_t digit = 0;
		size_t first_nonzero = 0;
		for (size_t max = 1; max <= v; ++digit)
		{
			size_t max_next = max * 10;
			char v_mod = (char) (v % max_next / max);
			if (!trailing_zeros && v_mod != '\0' && first_nonzero == 0)
				first_nonzero = digit;
			values[digit] = '0' + v_mod;
			max = max_next;
		}
		if (!trailing_zeros)
			for (size_t i = max_digits; i > digit; i--)
				*begin++ = '0';
		while (digit > first_nonzero)
			*begin++ = values[--digit];
	}

	static void print_float(char* begin, float f)
	{
		bool sign = false;
		if (f < 0.f)
			sign = true;
		float unsigned_f = fabsf(f);
		if (unsigned_f < max_float && unsigned_f > min_float)
		{
			if (sign)
				*begin++ = '-';
			print_mantissa<true>(begin, unsigned_f);
			unsigned_f -= (size_t)unsigned_f;
			unsigned_f *= max_int;
			if (unsigned_f >= 1.f)
			{
				*begin++ = '.';
				print_mantissa<false>(begin, unsigned_f);
			}
		}
		else if (unsigned_f == 0.)
			*begin++ = '0';
		else
		{
			sprintf(begin, "%g", f);
			return;
		}
		*begin = '\0';
		return;
	}
};

//A quick implementation similar to drand48 for cross-platform compatibility
namespace PRG {
	const uint64_t a = 0xeece66d5deece66dULL;
	const uint64_t c = 2147483647;

	const int bias = 127 << 23;

	union int_float {
	  int32_t i;
	  float f;
	};

	struct prg {
	private:
		uint64_t v;
	public:
		prg() { v = c; }
		prg(uint64_t initial) { v = initial; }

		float merand48(uint64_t& initial)
		{
			initial = a * initial + c;
			int_float temp;
			temp.i = ((initial >> 25) & 0x7FFFFF) | bias;
			return temp.f - 1;
		}

		float Uniform_Unit_Interval()
		{
			return merand48(v);
		}

		uint32_t Uniform_Int(uint32_t low, uint32_t high)
		{
			merand48(v);
			uint32_t ret = low + ((v >> 25) % (high - low + 1));
			return ret;
		}
	};
}
}
/*! @} End of Doxygen Groups*/