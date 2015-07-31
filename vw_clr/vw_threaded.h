/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vowpalwabbit.h"

namespace VW
{
	public ref class VowpalWabbitThreaded : VowpalWabbitNative
	{
	private:
		size_t m_node;
		size_t m_exampleCounter;
		size_t m_syncCount;

	internal:
		// ad idea to override Learn as there are multiline examples!!!
		/*
		/// <summary>
		/// Learns from the given example.
		/// </summary>
		/// <param name="ex">The native example.</param>
		void Learn(example* ex) override;

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance learns from this example and returns the prediction result for this example.
		/// </summary>
		virtual void LearnAndPredict(example* ex, VowpalWabbitPrediction^ result) override;
		*/
	public:
		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitThreaded"/> instance. 
		/// </summary>
		/// <param name="args">Command line arguments.</param>
		VowpalWabbitThreaded(VowpalWabbitSettings^ args, size_t node);

		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitThreaded"/> instance. 
		/// </summary>
		/// <param name="args">Command line arguments.</param>
		// TODO: create factory
		VowpalWabbitThreaded(VowpalWabbitSettings^ args, VowpalWabbitThreaded^ parent, size_t node);

		void EndOfPass();

		property size_t NodeId
		{
			size_t get();
		}
	};
}
