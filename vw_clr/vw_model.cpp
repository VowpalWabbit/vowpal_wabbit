/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"
#include "parse_regressor.h"
#include "parse_args.h"
#include "clr_io.h"

namespace Microsoft
{
	namespace Research
	{
		namespace MachineLearning
		{
			VowpalWabbitModel::VowpalWabbitModel(System::String^ pArgs)
				: VowpalWabbitBase(pArgs), m_instanceCount(0)
			{
			}

			vw* initialize(System::String^ pArgs, System::IO::Stream^ stream)
			{
				clr_io_buf io_temp(stream);
				try
				{
					auto string = msclr::interop::marshal_as<std::string>(pArgs);
					auto vw = VW::initialize(string, &io_temp);
					initialize_parser_datastructures(*vw);

					return vw;
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
			}

			VowpalWabbitModel::VowpalWabbitModel(System::String^ pArgs, System::IO::Stream^ stream)
				: VowpalWabbitBase(initialize(pArgs, stream))
			{
			}

			VowpalWabbitModel::~VowpalWabbitModel()
			{
				this->!VowpalWabbitModel();
			}

			VowpalWabbitModel::!VowpalWabbitModel()
			{
				if (m_instanceCount <= 0)
				{
					this->InternalDispose();
				}
			}

			void VowpalWabbitModel::IncrementReference()
			{
				// thread-safe increase of model reference counter
				System::Threading::Interlocked::Increment(m_instanceCount);
			}

			void VowpalWabbitModel::DecrementReference()
			{
				// thread-safe decrease of model reference counter
				if (System::Threading::Interlocked::Decrement(m_instanceCount) <= 0)
				{
					this->InternalDispose();
				}
			}
		}
	}
}