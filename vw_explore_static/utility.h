#pragma once

#include <random>

typedef unsigned __int64 u64;
typedef unsigned __int32 u32;
typedef unsigned __int16 u16;
typedef unsigned __int8  u8;
typedef unsigned char    byte;

typedef signed __int64 i64;
typedef signed __int32 i32;
typedef signed __int16 i16;
typedef signed __int8  i8;

//TODO: Create a custom Guid class, use std::atomic internally for now

///
/// Convenience wrapper for psuedo-random number generation. 
///
template <typename IntType>
class PRG
{
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

	double Uniform_Unit()
	{
		//TODO: Why doesn't numeric_limits work?
		return uniform(engine) / (std::numeric_limits<IntType>::max)();
	}

	RNGType Get_Engine()
	{
		return engine;
	}

private:
    std::random_device rd;
    RNGType engine;
    std::uniform_int_distribution<IntType> uniform;
};