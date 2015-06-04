#pragma once

using namespace System;
using namespace System::Collections::Generic;

/** \defgroup MultiWorldTestingCsharp
\brief C# implementation, for sample usage see: https://github.com/sidsen/vowpal_wabbit/blob/v0/cs_test/ExploreOnlySample.cs
*/

/*!
*  \addtogroup MultiWorldTestingCsharp
*  @{
*/

//! Interface for C# version of Multiworld Testing library.
//! For sample usage see: https://github.com/sidsen/vowpal_wabbit/blob/v0/cs_test/ExploreOnlySample.cs
namespace MultiWorldTesting {

/// <summary>
/// Represents a recorder that exposes a method to record exploration data based on generic contexts. 
/// </summary>
/// <typeparam name="Ctx">The Context type.</typeparam>
/// <remarks>
/// Exploration data is specified as a set of tuples (context, action, probability, key) as described below. An 
/// application passes an IRecorder object to the @MwtExplorer constructor. See 
/// @StringRecorder for a sample IRecorder object.
/// </remarks>
generic <class Ctx>
public interface class IRecorder
{
public:
	/// <summary>
	/// Records the exploration data associated with a given decision.
    /// This implementation should be thread-safe if multithreading is needed.
	/// </summary>
	/// <param name="context">A user-defined context for the decision.</param>
	/// <param name="action">Chosen by an exploration algorithm given context.</param>
	/// <param name="probability">The probability of the chosen action given context.</param>
	/// <param name="uniqueKey">A user-defined identifer for the decision.</param>
	virtual void Record(Ctx context, UInt32 action, float probability, String^ uniqueKey) = 0;
};

/// <summary>
/// Exposes a method for choosing an action given a generic context. IPolicy objects are 
/// passed to (and invoked by) exploration algorithms to specify the default policy behavior.
/// </summary>
/// <typeparam name="Ctx">The Context type.</typeparam>
generic <class Ctx>
public interface class IPolicy
{
public:
	/// <summary>
	/// Determines the action to take for a given context.
    /// This implementation should be thread-safe if multithreading is needed.
	/// </summary>
	/// <param name="context">A user-defined context for the decision.</param>
	/// <returns>Index of the action to take (1-based)</returns>
	virtual UInt32 ChooseAction(Ctx context) = 0;
};

/// <summary>
/// Exposes a method for specifying a score (weight) for each action given a generic context. 
/// </summary>
/// <typeparam name="Ctx">The Context type.</typeparam>
generic <class Ctx>
public interface class IScorer
{
public:
	/// <summary>
	/// Determines the score of each action for a given context.
    /// This implementation should be thread-safe if multithreading is needed.
	/// </summary>
	/// <param name="context">A user-defined context for the decision.</param>
	/// <returns>Vector of scores indexed by action (1-based).</returns>
	virtual List<float>^ ScoreActions(Ctx context) = 0;
};

/// <summary>
/// Represents a context interface with variable number of actions which is
/// enforced if exploration algorithm is initialized in variable number of actions mode.
/// </summary>
public interface class IVariableActionContext
{
public:
    /// <summary>
    /// Gets the number of actions for the current context.
    /// </summary>
    /// <returns>The number of actions available for the current context.</returns>
    virtual UInt32 GetNumberOfActions() = 0;
};

generic <class Ctx>
public interface class IExplorer
{
public:
    virtual void EnableExplore(bool explore) = 0;
};

generic <class Ctx>
public interface class IConsumePolicy
{
public:
    virtual void UpdatePolicy(IPolicy<Ctx>^ newPolicy) = 0;
};

generic <class Ctx>
public interface class IConsumePolicies
{
public:
    virtual void UpdatePolicy(cli::array<IPolicy<Ctx>^>^ newPolicies) = 0;
};

generic <class Ctx>
public interface class IConsumeScorer
{
public:
    virtual void UpdateScorer(IScorer<Ctx>^ newScorer) = 0;
};

public interface class IStringContext
{
public:
	virtual String^ ToString() = 0;
};

}

/*! @} End of Doxygen Groups*/