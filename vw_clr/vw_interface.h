/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw.h"
#include "vw_clr.h"
#include "vw_prediction.h"

namespace VW
{
	ref class VowpalWabbitNative;

	public interface class IVowpalWabbitNative : System::IDisposable
	{
		property VowpalWabbitNative^ Underlying
		{
			VowpalWabbitNative^ get();
		}

		property VowpalWabbitSettings^ Settings
		{
			VowpalWabbitSettings^ get();
		}

		/// <summary>
		/// Run multi-passe training. 
		/// </summary>
		void RunMultiPass();

		/// <summary>
		/// Persist model to file specified by -i.
		/// </summary>
		void SaveModel();

		/// <summary>
		/// Persist model to <paramref name="filename"/>.
		/// </summary>
		/// <param name="filename">The destination filename for the model.</param>
		void SaveModel(String^ filename);

		/// <summary>
		/// Gets Collected performance statistics.
		/// </summary>
		property VowpalWabbitPerformanceStatistics^ PerformanceStatistics
		{
			VowpalWabbitPerformanceStatistics^ get();
		}

		/// <summary>
		/// Hashes the given namespace <paramref name="s"/>.
		/// </summary>
		/// <param name="s">String to be hashed.</param>
		/// <returns>The resulting hash code.</returns>
		/// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
		uint32_t HashSpaceNative(String^ s);

		/// <summary>
		/// Hashes the given namespace <paramref name="s"/>.
		/// </summary>
		/// <param name="s">String to be hashed.</param>
		/// <returns>The resulting hash code.</returns>
		/// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
		uint32_t HashSpace(String^ s);

		/// <summary>
		/// Hash the given feature <paramref name="s"/>.
		/// </summary>
		/// <param name="s">String to be hashed.</param>
		/// <param name="u">Hash offset.</param>
		/// <returns>The resulting hash code.</returns>
		/// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
		uint32_t HashFeatureNative(String^ s, unsigned long u);

		/// <summary>
		/// Hash the given feature <paramref name="s"/>.
		/// </summary>
		/// <param name="s">String to be hashed.</param>
		/// <param name="u">Hash offset.</param>
		/// <returns>The resulting hash code.</returns>
		/// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
		uint32_t HashFeature(String^s, unsigned long u);

		/// <summary>
		/// Learns from the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		void Learn(String^ line);

		/// <summary>
		/// Predicts based on the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		void PredictAndDiscard(String^ line);

		/// <summary>
		/// Learns from the given example and returns the prediction for the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		TPrediction LearnAndPredict(String^ line);

		/// <summary>
		/// Predicts based on the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		TPrediction Predict(String^ line);

			/// <summary>
			/// Invokes the driver.
			/// </summary>
			void Driver();
	};
}