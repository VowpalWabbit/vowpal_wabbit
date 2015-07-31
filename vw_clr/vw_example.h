/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw_clr.h"
#include "vowpalwabbit.h"

namespace VW
{
	ref class VowpalWabbitExample;
	ref class VowpalWabbitBase;
	ref class VowpalWabbitPrediction;

	/// <summary>
	/// Interface defining a vowpal wabbit example. 
	/// </summary>
	public interface class IVowpalWabbitExample : public IDisposable
	{
	public:
		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance learns from this example.
		/// </summary>
		virtual void Learn() = 0;

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance predicts an outcome using this example and discards the result.
		/// </summary>
		/// <remarks>Used with multi-line examples.</remarks>
		virtual void PredictAndDiscard() = 0;

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance learns from this example and returns the prediction result for this example.
		/// </summary>
		/// <returns>The prediction result.</returns>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
				virtual TPrediction LearnAndPredict() = 0;

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance predicts an outcome using this example.
		/// </summary>
		/// <returns>The prediction result.</returns>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
				virtual TPrediction Predict() = 0;

		/// <summary>
		/// The underlying <see cref="VowpalWabbitExample"/> this instance wraps.
		/// </summary>
		virtual property VowpalWabbitExample^ UnderlyingExample
		{
			VowpalWabbitExample^ get() = 0;
		}
	};

	/// <summary>
	/// A CLR representation of a vowpal wabbit example.
	/// </summary>
	/// <remarks>
	/// Underlying memory is allocated by native code, but examples are not part of the ring.
	/// To optimize performance each <see cref="VowpalWabbitBase"/> instance maintains an example pool.
	/// </remarks>
	public ref class VowpalWabbitExample : public IVowpalWabbitExample
	{
	protected:
		/// <summary>
		/// Returns native example data structure to parent <see cref="VowpalWabbitBase"/> instance.
		/// </summary>
		!VowpalWabbitExample();

	internal:
		/// <summary>
		/// Initializes a new instance of <see cref="VowpalWabbitExample"/>.
		/// </summary>
		/// <param name="vw">The parent instance. Examples cannot be shared between <see cref="VowpalWabbitBase"/> instances.</param>
		/// <param name="example">The already allocated example structure</param>
		VowpalWabbitExample(IVowpalWabbitNative^ vw, example* example);

		/// <summary>
		/// The parent instance. Examples cannot be shared between <see cref="VowpalWabbitBase"/> instances.
		/// </summary>
		initonly VowpalWabbitNative^ m_vw;

		/// <summary>
		/// The native example data structure.
		/// </summary>
		example* m_example;

	public:
		/// <summary>
		/// Returns native example data structure to parent <see cref="VowpalWabbitBase"/> instance.
		/// </summary>
		~VowpalWabbitExample();

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance learns from this example.
		/// </summary>
		virtual void Learn();

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance predicts an outcome using this example and discards the result.
		/// </summary>
		/// <remarks>Used with multi-line examples.</remarks>
		virtual void PredictAndDiscard();

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance learns from this example and returns the prediction result for this example.
		/// </summary>
		/// <returns>The prediction result.</returns>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		virtual TPrediction LearnAndPredict();

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance predicts an outcome using this example.
		/// </summary>
		/// <returns>The prediction result.</returns>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		virtual TPrediction Predict();

		/// <summary>
		/// As this is the acutal example, it simply returns this.
		/// </summary>
		virtual property VowpalWabbitExample^ UnderlyingExample
		{
			VowpalWabbitExample^ get();
		}
	};
}