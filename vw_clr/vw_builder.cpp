/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_builder.h"
#include "parser.h"

namespace VW
{
	VowpalWabbitExampleBuilder::VowpalWabbitExampleBuilder(VowpalWabbit^ vw) :
		m_vw(vw->m_vw), m_example(nullptr)
	{
#ifdef _DEBUG
		if (vw == nullptr)
			throw gcnew ArgumentNullException("vw");
#endif

		m_example = vw->GetOrCreateNativeExample();
	}

	VowpalWabbitExampleBuilder::~VowpalWabbitExampleBuilder()
	{
		this->!VowpalWabbitExampleBuilder();
	}

	VowpalWabbitExampleBuilder::!VowpalWabbitExampleBuilder()
	{
		if (m_example != nullptr)
		{
			// in case CreateExample is not getting called
			delete m_example;

			m_example = nullptr;
		}
	}

	VowpalWabbitExample^ VowpalWabbitExampleBuilder::CreateExample()
	{
		if (m_example == nullptr)
			return nullptr;

		try
		{
			// finalize example
			VW::parse_atomic_example(*m_vw, m_example->m_example, false);
			VW::setup_example(*m_vw, m_example->m_example);
		}
		CATCHRETHROW

		// hand memory management off to VowpalWabbitExample
		auto ret = m_example;
		m_example = nullptr;

		return ret;
	}

	void VowpalWabbitExampleBuilder::ParseLabel(String^ value)
	{
		if (value == nullptr)
			return;

		auto bytes = System::Text::Encoding::UTF8->GetBytes(value);
		auto valueHandle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

		try
		{
			VW::parse_example_label(*m_vw, *m_example->m_example, reinterpret_cast<char*>(valueHandle.AddrOfPinnedObject().ToPointer()));
		}
		CATCHRETHROW
		finally
		{
			valueHandle.Free();
		}
	}

	VowpalWabbitNamespaceBuilder^ VowpalWabbitExampleBuilder::AddNamespace(Char featureGroup)
	{
		return AddNamespace((Byte)featureGroup);
	}

	VowpalWabbitNamespaceBuilder^ VowpalWabbitExampleBuilder::AddNamespace(Byte featureGroup)
	{
		uint32_t index = featureGroup;
		auto ex = m_example->m_example;
		ex->indices.push_back(index);

		return gcnew VowpalWabbitNamespaceBuilder(ex->sum_feat_sq + index, ex->atomics + index);
	}

	VowpalWabbitNamespaceBuilder::VowpalWabbitNamespaceBuilder(float* sum_feat_sq, v_array<feature>* atomic)
		: m_sum_feat_sq(sum_feat_sq), m_atomic(atomic)
	{
	}

	void VowpalWabbitNamespaceBuilder::AddFeature(uint32_t weight_index, float x)
	{
		// filter out 0-values
		if (x == 0)
		{
			return;
		}

		*m_sum_feat_sq += x * x;
		m_atomic->push_back({ x, weight_index });
	}

	void VowpalWabbitNamespaceBuilder::PreAllocate(int size)
	{
		m_atomic->resize((m_atomic->end - m_atomic->begin) + size);
	}
}
