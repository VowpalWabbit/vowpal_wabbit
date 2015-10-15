/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vowpalwabbit.h"
#include "vw_example.h"
#include "vw_prediction.h"

namespace VW
{
	VowpalWabbitExample::VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, example* example) :
		m_owner(owner), m_example(example), m_innerExample(nullptr)
	{
	}

	VowpalWabbitExample::VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, VowpalWabbitExample^ example) :
		m_owner(owner), m_example(example->m_example), m_innerExample(example)
	{
	}

	VowpalWabbitExample::!VowpalWabbitExample()
	{
		if (m_owner != nullptr)
			m_owner->ReturnExampleToPool(this);
	}

	VowpalWabbitExample::~VowpalWabbitExample()
	{
		this->!VowpalWabbitExample();
	}

	VowpalWabbitExample^ VowpalWabbitExample::InnerExample::get()
	{
		return m_innerExample;
	}

	IVowpalWabbitExamplePool^ VowpalWabbitExample::Owner::get()
	{
		return m_owner;
	}

	generic<typename T>
	T VowpalWabbitExample::GetPrediction(VowpalWabbit^ vw, IVowpalWabbitPredictionFactory<T>^ factory)
	{
#ifdef _DEBUG
		if (vw == nullptr)
			throw gcnew ArgumentNullException("vw");
#endif

		return factory->Create(vw->m_vw, m_example);
	}
}
