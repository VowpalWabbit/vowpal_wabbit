/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw_clr.h"
#include <stack>
#include "vw_interface.h"
#include "vw_arguments.h"

using namespace std;
using namespace System::Collections::Generic;

namespace VW
{
	ref class VowpalWabbitPrediction;
	ref class VowpalWabbitModel;

	/// <summary>
	/// A base wrapper around vowpal wabbit machine learning instance.
	/// </summary>
	/// <remarks>
	/// Since the model class must delay diposal of <see cref="m_vw"/> until all referencing
	/// VowpalWabbit instances are disposed, the base class does not dispose <see cref="m_vw"/>.
	/// </remarks>
	public ref class VowpalWabbitBase abstract : IVowpalWabbitExamplePool
	{
	private:
		/// <summary>
		/// The settings used for this instance.
		/// </summary>
		initonly VowpalWabbitSettings^ m_settings;

		/// <summary>
		/// An optional shared model.
		/// </summary>
		VowpalWabbitModel^ m_model;

		/// <summary>
		/// Example pool.
		/// </summary>
		Stack<VowpalWabbitExample^>^ m_examples;

    /// <summary>
    /// Extracted command line arguments.
    VowpalWabbitArguments^ m_arguments;

    /// <summary>
    /// Initialize from passed model.
    /// </summary>
    void InitializeFromModel(string args, io_buf& model);

		/// <summary>
		/// Reference count to native data structure.
		/// </summary>
		System::Int32 m_instanceCount;

	internal:
		/// <summary>
		/// The native vowpal wabbit data structure.
		/// </summary>
		vw* m_vw;

		/// <summary>
		/// Gets or creates a native example from a CLR maintained, but natively allocated pool.
		/// </summary>
		/// <returns>A ready to use cleared native example data structure.</returns>
		VowpalWabbitExample^ GetOrCreateNativeExample();

		/// <summary>
		/// Thread-safe increment of reference count.
		/// </summary>
		void IncrementReference();

		/// <summary>
		/// Thread-safe decrement of reference count.
		/// </summary>
		void DecrementReference();

	protected:
		/// <summary>
		/// True if all nativedata structures are disposed.
		/// </summary>
		bool m_isDisposed;

		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitBase"/> instance.
		/// </summary>
		/// <param name="settings">Command line arguments.</param>
		VowpalWabbitBase(VowpalWabbitSettings^ settings);

		/// <summary>
		/// Cleanup.
		/// </summary>
		!VowpalWabbitBase();

		/// <summary>
		/// Internal dipose using reference counting to delay disposal of shared native data structures.
		/// </summary>
		void InternalDispose();

	public:
		/// <summary>
		/// Cleanup.
		/// </summary>
		virtual ~VowpalWabbitBase();

		/// <summary>
		/// The settings used for this instance.
		/// </summary>
		property VowpalWabbitSettings^ Settings
  { VowpalWabbitSettings^ get();
		}

    /// <summary>
    /// Extracted command line arguments.
    /// </summary>
    property VowpalWabbitArguments^ Arguments
    {
      VowpalWabbitArguments^ get();
    }

    /// <summary>
    /// The read/writable model id.
    /// </summary>
    property String^ ID
    {
      String^ get();
      void set(String^ id);
    }

		/// <summary>
		/// Gets or creates an empty example.
		/// </summary>
		/// <returns>An initialized and empty example</returns>
		VowpalWabbitExample^ GetOrCreateEmptyExample();

		/// <summary>
		/// Puts a native example data structure back into the pool.
		/// </summary>
		/// <param name="example">The example to be returned.</param>
		virtual void ReturnExampleToPool(VowpalWabbitExample^ example) sealed;

    /// <summary>
    /// Performs the following steps to reset the learning state:
    ///
    /// - Save model to in-memory buffer
    /// - Dipose existing instance
    /// - Initialize new instance from in-memory buffer
    /// </summary>
    void Reload([System::Runtime::InteropServices::Optional] String^ args);

    /// <summary>
    /// Compares features created by current instance are compatible to features created by <paramref name="other"/>.
    /// </summary>
    /// <returns>
    /// Null if compatible, otherwise the difference 
    String^ AreFeaturesCompatible(VowpalWabbitBase^ other);
	};
}
