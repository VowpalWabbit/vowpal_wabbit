#pragma once

using namespace System;
using namespace System::Collections::Generic;

namespace MultiWorldTesting {

/// <summary>
/// Represents a recorder that exposes a method to record exploration data based on generic contexts. 
/// </summary>
/// <typeparam name="Ctx">The Context type.</typeparam>
/// <remarks>
/// Exploration data is specified as a set of tuples <context, action, probability, key> as described below. An 
/// application passes an IRecorder object to the @MwtExplorer constructor. See 
/// @StringRecorder for a sample IRecorder object.
/// </remarks>
generic <class Ctx>
public interface class IRecorder
{
public:
	virtual void Record(Ctx context, UInt32 action, float probability, String^ uniqueKey) = 0;
};

generic <class Ctx>
public interface class IPolicy
{
public:
	virtual UInt32 ChooseAction(Ctx context) = 0;
};

generic <class Ctx>
public interface class IScorer
{
public:
	virtual List<float>^ ScoreActions(Ctx context) = 0;
};

generic <class Ctx>
public interface class IExplorer
{
};

public interface class IStringContext
{
public:
	virtual String^ ToString() = 0;
};

}