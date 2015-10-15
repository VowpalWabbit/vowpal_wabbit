/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"
#include "vw_model.h"
#include "parse_regressor.h"
#include "parse_args.h"
#include "clr_io.h"

namespace VW
{
	VowpalWabbitModel::VowpalWabbitModel(VowpalWabbitSettings^ settings)
		: VowpalWabbitBase(settings), m_instanceCount(0)
	{
		if (settings == nullptr)
			throw gcnew ArgumentNullException("settings");

		if (settings->Model != nullptr)
			throw gcnew ArgumentNullException("VowpalWabbitModel cannot be initialized from another model");
	}

	VowpalWabbitModel::VowpalWabbitModel(String^ args)
		: VowpalWabbitModel(gcnew VowpalWabbitSettings(args))
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
