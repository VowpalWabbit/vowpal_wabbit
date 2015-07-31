/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw_clr.h"
#include <stack>

using namespace std;

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
	public ref class VowpalWabbitBase abstract 
	{
	private:
		VowpalWabbitModel^ m_model;

		VowpalWabbitSettings^ m_settings;

		/// <summary>
		/// The CLR maintained, but natively allocated example pool.
		/// </summary>
		stack<example*>* m_examples;
		/// <summary>
		/// Select the right hash method based on args.
		/// </summary>
		Func<String^, unsigned long, size_t>^ GetHasher();

	internal:
		/// <summary>
		/// The native vowpal wabbit data structure.
		/// </summary>
		vw* m_vw;

		/// <summary>
		/// Gets or creates a native example from a CLR maintained, but natively allocated pool.
		/// </summary>
		/// <returns>A ready to use cleared native example data structure.</returns>
		example* GetOrCreateNativeExample();

		/// <summary>
		/// Puts a native example data structure back into the pool.
		/// </summary>
		/// <param name="ex">The example to be returned.</param>
		void ReturnExampleToPool(example* ex);

	protected:
		/// <summary>
		/// The selected hasher method.
		/// </summary>
		/// <remarks>
		/// Avoiding if-else for hash function selection. Delegates outperform function pointers according to http://stackoverflow.com/questions/13443250/performance-of-c-cli-function-pointers-versus-net-delegates
		/// </remarks>
		initonly Func<String^, unsigned long, size_t>^ m_hasher;

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

		virtual property VowpalWabbitSettings^ Settings
		{
			VowpalWabbitSettings^ get();
		}
	};
}