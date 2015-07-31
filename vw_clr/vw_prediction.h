/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw_clr.h"

namespace VW
{
	ref class VowpalWabbitExample;

	/// <summary>
	/// Base-class for prediction results. 
	/// </summary>
	public ref class VowpalWabbitPrediction abstract
	{
	public:
		/// <summary>
		/// Extracts data and forwards to <see cref="ReadFromExample(vw*, example*)" />
		/// </summary>
		void ReadFromExample(VowpalWabbitExample^ example);

		/// <summary>
		/// Subclasses must extract the prediction result from the example.
		/// </summary>
		virtual void ReadFromExample(vw* vw, example* ex) abstract;
	};

	/// <summary>
	/// A scalar prediction result.
	/// </summary>
	public ref class VowpalWabbitScalarPrediction : VowpalWabbitPrediction
	{
	public:
		/// <summary>
		/// Extracts prediction results from example.
		/// </summary>
		void ReadFromExample(vw* vw, example* ex) override;

		/// <summary>
		/// The scalar prediction.
		/// </summary>
		property float Value;
	};

	/// <summary>
	/// A cost sensitive prediction result.
	/// </summary>
	public ref class VowpalWabbitCostSensitivePrediction : VowpalWabbitPrediction
	{
	public:
		/// <summary>
		/// Extracts prediction results from example.
		/// </summary>
		void ReadFromExample(vw* vw, example* ex) override;

		/// <summary>
		/// The cost sensitive prediction.
		/// </summary>
		property float Value;
	};

	/// <summary>
	/// A multi label prediction result.
	/// </summary>
	public ref class VowpalWabbitMultilabelPrediction : VowpalWabbitPrediction
	{
	public:
		/// <summary>
		/// Extracts prediction results from example.
		/// </summary>
		void ReadFromExample(vw* vw, example* ex) override;

		/// <summary>
		/// The predicted labels.
		/// </summary>
		property cli::array<int>^ Values;
	};

	/// <summary>
	/// A topic prediction result.
	/// </summary>
	public ref class VowpalWabbitTopicPrediction : VowpalWabbitPrediction
	{
	public:
		/// <summary>
		/// Extracts prediction results from example.
		/// </summary>
		void ReadFromExample(vw* vw, example* ex) override;

		/// <summary>
		/// The predicted topics.
		/// </summary>
		property cli::array<float>^ Values;
	};
}