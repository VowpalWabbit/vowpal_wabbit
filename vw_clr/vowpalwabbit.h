/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw_clr.h"
#include "vw_base.h"
#include "vw_model.h"
#include "vw_prediction.h"
#include "vw_interface.h"

namespace VW
{
	/// <summary>
	/// Simple string example based wrapper for vowpal wabbit.  
	/// </summary>
	/// <remarks>If possible use VowpalWabbit{T} types as this wrapper suffers from marshalling performance wise.</remarks> 
	public ref class VowpalWabbitNative : VowpalWabbitBase, IVowpalWabbitNative
	{
	private:
		/// <summary>
		/// Optional shared vowpalwabbit model.
		/// </summary>
		VowpalWabbitModel^ m_model;

		/// <summary>
		/// Learns from or predicts the given example (<paramref name="line"/>) and returns the prediction result.
		/// </summary>
		/// <returns>The prediction result.</returns>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		TPrediction PredictOrLearn(String^ line, bool predict);

	internal:
		/// <summary>
		/// Learns from the given example.
		/// </summary>
		/// <param name="ex">The native example.</param>
		void Learn(example* ex);

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance predicts an outcome using this example and discards the result.
		/// </summary>
		/// <remarks>Used with multi-line examples.</remarks>
		void PredictAndDiscard(example* ex);

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance learns from this example and returns the prediction result for this example.
		/// </summary>
		/// <remarks>
		/// Due to C++/CLI limitation prediction results need to be allocated upfront. 
		/// Can't specify gcnew constraints on generic argument and use it in derived clas.
		/// </remarks>
		void LearnAndPredict(example* ex, VowpalWabbitPrediction^ result);

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance predicts an outcome using this example.
		/// </summary>
		void Predict(example* ex, VowpalWabbitPrediction^ result);

	public:
		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitBase"/> instance. 
		/// </summary>
		/// <param name="args">Command line arguments.</param>
		VowpalWabbitNative(VowpalWabbitSettings^ args);

		/// <summary>
		/// Run multi-passe training. 
		/// </summary>
		virtual void RunMultiPass();

		/// <summary>
		/// Persist model to file specified by -i.
		/// </summary>
		virtual void SaveModel();

		/// <summary>
		/// Persist model to <paramref name="filename"/>.
		/// </summary>
		/// <param name="filename">The destination filename for the model.</param>
		virtual void SaveModel(String^ filename);

		/// <summary>
		/// Gets Collected performance statistics.
		/// </summary>
		virtual property VowpalWabbitPerformanceStatistics^ PerformanceStatistics
		{
			VowpalWabbitPerformanceStatistics^ get();
		}

		virtual property VowpalWabbitNative^ Underlying
		{
			VowpalWabbitNative^ get();
		}

		/// <summary>
		/// Hashes the given namespace <paramref name="s"/>.
		/// </summary>
		/// <param name="s">String to be hashed.</param>
		/// <returns>The resulting hash code.</returns>
		/// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
		virtual uint32_t HashSpaceNative(String^ s);

		/// <summary>
		/// Hashes the given namespace <paramref name="s"/>.
		/// </summary>
		/// <param name="s">String to be hashed.</param>
		/// <returns>The resulting hash code.</returns>
		/// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
		virtual uint32_t HashSpace(String^ s);

		/// <summary>
		/// Hash the given feature <paramref name="s"/>.
		/// </summary>
		/// <param name="s">String to be hashed.</param>
		/// <param name="u">Hash offset.</param>
		/// <returns>The resulting hash code.</returns>
		/// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
		virtual uint32_t HashFeatureNative(String^ s, unsigned long u);

		/// <summary>
		/// Hash the given feature <paramref name="s"/>.
		/// </summary>
		/// <param name="s">String to be hashed.</param>
		/// <param name="u">Hash offset.</param>
		/// <returns>The resulting hash code.</returns>
		/// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
		virtual uint32_t HashFeature(String^s, unsigned long u);

		/// <summary>
		/// Learns from the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		virtual void Learn(String^ line);

		/// <summary>
		/// Predicts based on the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		virtual void PredictAndDiscard(String^ line);

		/// <summary>
		/// Learns from the given example and returns the prediction for the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		virtual TPrediction LearnAndPredict(String^ line);

		/// <summary>
		/// Predicts based on the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		virtual TPrediction Predict(String^ line);

		/// <summary>
		/// Invokes the driver.
		/// </summary>
		virtual void Driver();
	};
}