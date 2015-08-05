/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw_clr.h"
#include "vw_interface.h"

namespace VW
{
	/// <summary>
	/// A CLR representation of a vowpal wabbit example.
	/// </summary>
	/// <remarks>
	/// Underlying memory is allocated by native code, but examples are not part of the ring.
	/// To optimize performance each <see cref="VowpalWabbitBase"/> instance maintains an example pool.
	/// </remarks>
	public ref class VowpalWabbitExample 
	{
	private:
		initonly VowpalWabbitExample^ m_innerExample;

	protected:
		/// <summary>
		/// Returns native example data structure to parent <see cref="VowpalWabbitBase"/> instance.
		/// </summary>
		!VowpalWabbitExample();

	internal:
		/// <summary>
		/// Initializes a new instance of <see cref="VowpalWabbitExample"/>.
		/// </summary>
		/// <param name="owner">The parent instance. Examples cannot be shared between <see cref="VowpalWabbitBase"/> instances.</param>
		/// <param name="example">The already allocated example structure</param>
		VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, example* example);

		/// <summary>
		/// The native example data structure.
		/// </summary>
		example* m_example;

		IVowpalWabbitExamplePool^ m_owner;

	public:
		VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, VowpalWabbitExample^ example);

		/// <summary>
		/// Returns native example data structure to parent <see cref="VowpalWabbitBase"/> instance.
		/// </summary>
		~VowpalWabbitExample();

		generic<typename T>
		T GetPrediction(VowpalWabbit^ vw, IVowpalWabbitPredictionFactory<T>^ factory);

		property VowpalWabbitExample^ InnerExample
		{
			VowpalWabbitExample^ get();
		}

		property IVowpalWabbitExamplePool^ Owner
		{
			IVowpalWabbitExamplePool^ get();
		}
	};
}