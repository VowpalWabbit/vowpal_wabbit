#pragma once

#include <random>

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


//TODO: Create a custom Guid class, use std::atomic internally for now

///
/// Convenience wrapper for psuedo-random number generation. 
///
template <typename IntType>
class PRG
{
public:
	// This type should be used by callers who get access to the underlying engine
	typedef std::mt19937_64 RNGType;

public:
	PRG() : engine(rd()) { }

	PRG(IntType seed) : engine(seed) { }

    IntType Uniform_Int()
	{
		return uniform(engine);
	}

    IntType Uniform_Int(IntType low, IntType high)
	{
		return (uniform(engine) % (high - low + 1)) + low;
	}

	double Uniform_Unit_Interval()
	{
		return (double)uniform(engine) / (std::numeric_limits<IntType>::max)();
	}

	// Passing by reference is important here: if we pass by value and the caller evolves the 
	// PRG engine, it will only evolve the *copy*, not this engine
	RNGType& Get_Engine()
	{
		return engine;
	}

private:
    std::random_device rd;
    RNGType engine;
    std::uniform_int_distribution<IntType> uniform;
};
