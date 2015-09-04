/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"

namespace VW
{
    VowpalWabbitExample::VowpalWabbitExample(VowpalWabbitBase^ vw, example* example) :
		m_vw(vw), m_example(example)
	{
	}

	VowpalWabbitExample::!VowpalWabbitExample()
	{
		if (m_example != nullptr)
		{
			// make sure we're not a ring based example 
			assert(!VW::is_ring_example(*m_vw->m_vw, m_example));

            m_vw->ReturnExampleToPool(m_example);

			m_example = nullptr;
		}
	}

	VowpalWabbitExample::~VowpalWabbitExample()
	{
		this->!VowpalWabbitExample();
	}

	void VowpalWabbitExample::Learn()
	{
		try
		{
			m_vw->m_vw->learn(m_example);

			// as this is not a ring-based example it is not free'd
			m_vw->m_vw->l->finish_example(*m_vw->m_vw, *m_example);
		}
		CATCHRETHROW
	}

	void VowpalWabbitExample::PredictAndDiscard()
	{
		try
		{
			m_vw->m_vw->l->predict(*m_example);

			// as this is not a ring-based example it is not free'd
			m_vw->m_vw->l->finish_example(*m_vw->m_vw, *m_example);
		}
		CATCHRETHROW
	}

	generic<typename TPrediction>
		where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
	TPrediction VowpalWabbitExample::LearnAndPredict()
	{
		try
		{
			m_vw->m_vw->learn(m_example);

			auto prediction = gcnew TPrediction();
			prediction->ReadFromExample(m_vw->m_vw, m_example);

			// as this is not a ring-based example it is not free'd
			m_vw->m_vw->l->finish_example(*m_vw->m_vw, *m_example);

			return prediction; 
		}
		CATCHRETHROW
	}

	generic<typename TPrediction>
		where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
	TPrediction VowpalWabbitExample::Predict()
	{
		try
		{
			m_vw->m_vw->l->predict(*m_example);

			auto prediction = gcnew TPrediction();
			prediction->ReadFromExample(m_vw->m_vw, m_example);

			// as this is not a ring-based example it is not free'd
			m_vw->m_vw->l->finish_example(*m_vw->m_vw, *m_example);

			return prediction;
		}
		CATCHRETHROW
	}

	VowpalWabbitExample^ VowpalWabbitExample::UnderlyingExample::get()
	{
		return this;
	}
}