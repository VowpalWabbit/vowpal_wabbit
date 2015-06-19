/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"
#include "vw.h"
#include "parser.h"

namespace VW
{
	VowpalWabbitExampleBuilder::VowpalWabbitExampleBuilder(VowpalWabbitBase^ vw) :
		m_vw(vw->m_vw), m_example(nullptr), m_clrExample(nullptr)
	{
        m_example = vw->GetOrCreateNativeExample();
		m_vw->p->lp.default_label(&m_example->l);
		m_clrExample = gcnew VowpalWabbitExample(vw, m_example);
	}

	VowpalWabbitExampleBuilder::~VowpalWabbitExampleBuilder()
	{
		this->!VowpalWabbitExampleBuilder();
	}

	VowpalWabbitExampleBuilder::!VowpalWabbitExampleBuilder()
	{
		if (m_clrExample != nullptr)
		{
			// in case CreateExample is not getting called
			delete m_clrExample;

			m_clrExample = nullptr;
		}
	}

	VowpalWabbitExample^ VowpalWabbitExampleBuilder::CreateExample()
	{
		if (m_clrExample == nullptr)
			return nullptr;

		try
		{
			// finalize example
			VW::parse_atomic_example(*m_vw, m_example, false);
			VW::setup_example(*m_vw, m_example);
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(gcnew System::String(ex.what()));
		}

		// hand memory management off to VowpalWabbitExample
		auto ret = m_clrExample;
		m_example = nullptr;
		m_clrExample = nullptr;

		return ret;
	}

	void VowpalWabbitExampleBuilder::Label::set(System::String^ value)
	{
		if (value == nullptr)
			return;
				
		auto bytes = System::Text::Encoding::UTF8->GetBytes(value);
		auto valueHandle = GCHandle::Alloc(bytes, GCHandleType::Pinned);
				
		try
		{
			VW::parse_example_label(*m_vw, *m_example, reinterpret_cast<char*>(valueHandle.AddrOfPinnedObject().ToPointer()));
		}
		catch (std::exception const& ex)
		{
			throw gcnew System::Exception(gcnew System::String(ex.what()));
		}
		finally
		{
			valueHandle.Free();
		}
	}

	VowpalWabbitNamespaceBuilder^ VowpalWabbitExampleBuilder::AddNamespace(System::Byte featureGroup)
	{
		uint32_t index = featureGroup;
		m_example->indices.push_back(index);

		return gcnew VowpalWabbitNamespaceBuilder(m_example->sum_feat_sq + index, m_example->atomics + index);
	}

	VowpalWabbitNamespaceBuilder::VowpalWabbitNamespaceBuilder(float* sum_feat_sq, v_array<feature>* atomic)
		: m_sum_feat_sq(sum_feat_sq), m_atomic(atomic)
	{
	}

	void VowpalWabbitNamespaceBuilder::AddFeature(uint32_t weight_index, float x)
	{
		*m_sum_feat_sq += x * x;
		m_atomic->push_back({ x, weight_index });
	}
}