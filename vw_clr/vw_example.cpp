/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_base.h"
#include "vw_example.h"
#include "vw_prediction.h"

namespace VW
{
	VowpalWabbitExample::VowpalWabbitExample(IVowpalWabbitNative^ vw, example* example) :
		m_vw(vw->Underlying), m_example(example)
	{
	}

	VowpalWabbitExample::!VowpalWabbitExample()
	{
		if (m_example != nullptr)
		{
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
		m_vw->Learn(m_example);
	}

	void VowpalWabbitExample::PredictAndDiscard()
	{
		m_vw->PredictAndDiscard(m_example);
	}

	generic<typename TPrediction>
		where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
	TPrediction VowpalWabbitExample::LearnAndPredict()
	{
		auto prediction = gcnew TPrediction();
		
		m_vw->LearnAndPredict(m_example, prediction);

		return prediction;
	}

	generic<typename TPrediction>
		where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
	TPrediction VowpalWabbitExample::Predict()
	{
		auto prediction = gcnew TPrediction();

		m_vw->Predict(m_example, prediction);

		return prediction;
	}

	VowpalWabbitExample^ VowpalWabbitExample::UnderlyingExample::get()
	{
		return this;
	}
}