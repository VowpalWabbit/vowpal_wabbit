/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"

using namespace System;

namespace VW
{
	VowpalWabbit::VowpalWabbit(String^ pArgs)
		: VowpalWabbitBase(pArgs)
	{
	}

	VowpalWabbit::VowpalWabbit(String^ pArgs, System::IO::Stream^ model)
		: VowpalWabbitBase(pArgs, model)
	{
	}

	vw* wrapped_seed_vw_model(vw* vw)
	{
		try
		{
			return VW::seed_vw_model(vw, "");
		}
		CATCHRETHROW
	}

	VowpalWabbit::VowpalWabbit(VowpalWabbitModel^ model)
		: VowpalWabbitBase(wrapped_seed_vw_model(model->m_vw)), m_model(model)
	{
		m_model->IncrementReference();
	}

	VowpalWabbit::~VowpalWabbit()
	{
		this->!VowpalWabbit();
	}

	VowpalWabbit::!VowpalWabbit()
	{
		if (m_model != nullptr)
		{
			// this object doesn't own the VW instance
			m_model->DecrementReference();
			m_model = nullptr;
		}
		else
		{
			// this object owns the VW instance.
			this->InternalDispose();
		}
	}

	generic<typename TPrediction>
		where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
	TPrediction VowpalWabbit::PredictOrLearn(String^ line, bool predict)
	{
		auto bytes = System::Text::Encoding::UTF8->GetBytes(line);
		auto lineHandle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

		example* ex = nullptr;
		try
		{
			ex = VW::read_example(*m_vw, reinterpret_cast<char*>(lineHandle.AddrOfPinnedObject().ToPointer()));

			if (predict)
				m_vw->l->predict(*ex);
			else
				m_vw->learn(ex);

			auto prediction = gcnew TPrediction();
			prediction->ReadFromExample(m_vw, ex);

			m_vw->l->finish_example(*m_vw, *ex);
			ex = nullptr;

			return prediction;
		}
		CATCHRETHROW
		finally
		{
			lineHandle.Free();

			if (ex != nullptr)
			{
				VW::finish_example(*m_vw, ex);
			}
		}
	}

	generic<typename TPrediction>
		where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
	TPrediction VowpalWabbit::Learn(String^ line)
	{
		return PredictOrLearn<TPrediction>(line, false);
	}

	generic<typename TPrediction>
		where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
	TPrediction VowpalWabbit::Predict(String^ line)
	{
		return PredictOrLearn<TPrediction>(line, true);
	}

	void VowpalWabbit::Learn(String^ line)
	{
		auto bytes = System::Text::Encoding::UTF8->GetBytes(line);
		auto lineHandle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

		example* ex = nullptr;
		try
		{
			ex = VW::read_example(*m_vw, reinterpret_cast<char*>(lineHandle.AddrOfPinnedObject().ToPointer()));

			m_vw->learn(ex);

			m_vw->l->finish_example(*m_vw, *ex);
			ex = nullptr;
		}
		CATCHRETHROW
		finally
		{
			lineHandle.Free();

			if (ex != nullptr)
			{
				VW::finish_example(*m_vw, ex);
			}
		}
	}

	void VowpalWabbit::Predict(String^ line)
	{
		auto bytes = System::Text::Encoding::UTF8->GetBytes(line);
		auto lineHandle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

		example* ex = nullptr;
		try
		{
			ex = VW::read_example(*m_vw, reinterpret_cast<char*>(lineHandle.AddrOfPinnedObject().ToPointer()));

			m_vw->l->predict(*ex);

			m_vw->l->finish_example(*m_vw, *ex);
			ex = nullptr;
		}
		CATCHRETHROW
		finally
		{
			lineHandle.Free();

			if (ex != nullptr)
			{
				VW::finish_example(*m_vw, ex);
			}
		}
	}


	void VowpalWabbit::Driver()
	{
		try
		{
			LEARNER::generic_driver(*m_vw);
		}
		CATCHRETHROW
	}
}