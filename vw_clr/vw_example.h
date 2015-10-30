/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw_clr.h"
#include "vw_interface.h"
#include "vw_labelcomparator.h"

namespace VW
{
  /// <summary>
	/// A CLR representation of a vowpal wabbit example.
	/// </summary>
	/// <remarks>
	/// Underlying memory is allocated by native code, but examples are not part of the ring.
	/// </remarks>
	[System::Diagnostics::DebuggerDisplay("{m_string}")]
	public ref class VowpalWabbitExample
	{
	private:
		/// <summary>
		/// Reference to an optional underlying example.
		/// </summary>
		/// <remarks>If this instance owns <see name="m_example"/> this is null.</remarks>
		initonly VowpalWabbitExample^ m_innerExample;

	protected:
		/// <summary>
		/// Returns native example data structure to owning instance.
		/// </summary>
		!VowpalWabbitExample();

	internal:
		/// <summary>
		/// Initializes a new instance of <see cref="VowpalWabbitExample"/>.
		/// </summary>
		/// <param name="owner">The parent instance. Examples cannot be shared between vw instances.</param>
		/// <param name="example">The already allocated example structure</param>
		VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, example* example);

		/// <summary>
		/// The native example data structure.
		/// </summary>
		example* m_example;

		/// <summary>
		/// The owner of this example.
		/// </summary>
		IVowpalWabbitExamplePool^ m_owner;

		/// <summary>
		/// The optional string version of the example.
		/// </summary>
		String^ m_string;

	public:
		/// <summary>
		/// Initializes a new instance of <see cref="VowpalWabbitExample"/>.
		/// </summary>
		/// <param name="owner">The parent instance. Examples cannot be shared between <see cref="IVowpalWabbitExamplePool"/> instances.</param>
		/// <param name="example">The inner example this instance wraps.</param>
		VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, VowpalWabbitExample^ example);

		/// <summary>
		/// Returns native example data structure to owning pool.
		/// </summary>
		~VowpalWabbitExample();

		/// <summary>
		/// Extracts the prediction from this example using the given prediction factory.
		/// </summary>
		/// <returns>The prediction stored in this example.</returns>
		generic<typename T>
		T GetPrediction(VowpalWabbit^ vw, IVowpalWabbitPredictionFactory<T>^ factory);

		/// <summary>
		/// An optional inner example this example wraps.
		/// </summary>
		property VowpalWabbitExample^ InnerExample
  { VowpalWabbitExample^ get();
		}

		/// <summary>
		/// The owner of this example.
		/// </summary>
		property IVowpalWabbitExamplePool^ Owner
  { IVowpalWabbitExamplePool^ get();
		}

		/// <summary>
		/// The corresponding VowpalWabbitString for this example.
		/// </summary>
		property String^ VowpalWabbitString
		{
			String^ get();
			void set(String^ value);
		}

    /// <summary>
    /// True if this is a new line example, otherwise false.
    /// </summary>
    /// <remarks>A example without features is considered a new line example.</reamrks>
    property bool IsNewLine
    {
      bool get();
    }

    String^ Diff(VowpalWabbit^ vw, VowpalWabbitExample^ other, IVowpalWabbitLabelComparator^ labelComparator);
	};
}
