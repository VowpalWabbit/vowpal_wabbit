/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::IO;

namespace VW
{
	ref class VowpalWabbitModel;

	public ref class VowpalWabbitSettings
	{
	public:
		VowpalWabbitSettings()
		{
			Arguments = String::Empty;
			EnableExampleCaching = true;
			MaxExampleCacheSize = UINT32_MAX;
			MaxExampleQueueLengthPerInstance = UINT32_MAX;
			ExampleCountPerRun = 1000;
		}

		/// <summary>
		/// Command line arguments.
		/// </summary>
		property String^ Arguments;

		/// <summary>
		/// Model used for initialization.
		/// </summary>
		property Stream^ ModelStream;

		/// <summary>
		/// Shared native vowpwal wabbit data structure.
		/// </summary>
		property VowpalWabbitModel^ Model;

		property System::Threading::Tasks::ParallelOptions^ ParallelOptions;

		/// <summary>
		/// Set to true to disable example caching when used with a serializer. Defaults to true.
		/// </summary>
		property bool EnableExampleCaching;

		/// <summary>
		/// Maximum number of serialized examples cached. Defaults to UINT32_MAX.
		/// </summary>
		property uint32_t MaxExampleCacheSize;

		/// <summary>
		/// Maximum number of examples accepted by VowpalWabbitManager until Learn/Predict/... start to block. Defaults to UINT32_MAX.
		/// </summary>
		property uint32_t MaxExampleQueueLengthPerInstance;

		/// <summary>
		/// In multi-threaded mode, this is the number of examples processed per run.
		/// After ecah run the models are synchronized.
		/// Defaults to 1000.
		/// </summary>
		property uint32_t ExampleCountPerRun;

		VowpalWabbitSettings^ ShallowCopy()
		{
			auto copy = gcnew VowpalWabbitSettings();

			copy->Arguments = Arguments;
			copy->Model = Model;
			copy->ParallelOptions = ParallelOptions;
			copy->EnableExampleCaching = EnableExampleCaching;
			copy->MaxExampleCacheSize = MaxExampleCacheSize;
			copy->MaxExampleQueueLengthPerInstance = MaxExampleQueueLengthPerInstance;

			return copy;
		}
	};

}