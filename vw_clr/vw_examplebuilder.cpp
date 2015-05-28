#include "vw_clr.h"
#include "vw.h"
#include "parser.h"

namespace Microsoft
{
	namespace Research
	{
		namespace MachineLearning
		{
			VowpalWabbitExampleBuilder::VowpalWabbitExampleBuilder(VowpalWabbitBase^ vw) :
				m_vw(vw->m_vw), m_example(nullptr), m_clrExample(nullptr)
			{
				m_example = VW::alloc_examples(0, 1);
				m_vw->p->lp.default_label(&m_example->l);
				m_clrExample = gcnew VowpalWabbitExample(m_vw, m_example);
			}

			VowpalWabbitExampleBuilder::~VowpalWabbitExampleBuilder()
			{
				this->!VowpalWabbitExampleBuilder();
			}

			VowpalWabbitExampleBuilder::!VowpalWabbitExampleBuilder()
			{
				if (m_clrExample)
				{
					// in case CreateExample is not getting called
					delete m_clrExample;

					m_clrExample = nullptr;
				}
			}

			VowpalWabbitExample^ VowpalWabbitExampleBuilder::CreateExample()
			{
				if (!m_clrExample)
					return nullptr;

				// finalize example
				VW::parse_atomic_example(*m_vw, m_example, false);
				VW::setup_example(*m_vw, m_example);

				// hand memory management off to VowpalWabbitExample
				auto ret = m_clrExample;
				m_example = nullptr;
				m_clrExample = nullptr;

				return ret;
			}

			void VowpalWabbitExampleBuilder::Label::set(System::String^ value)
			{
				if (!value)
					return;

				auto labelString = msclr::interop::marshal_as<std::string>(value);
				VW::parse_example_label(*m_vw, *m_example, labelString);
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
	}
}