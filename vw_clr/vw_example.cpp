#include "vw_clr.h"

namespace Microsoft
{
	namespace Research
	{
		namespace  MachineLearning
		{
			VowpalWabbitExample::VowpalWabbitExample(vw* vw, example* example) : 
				VowpalWabbitExample(vw, example, false)
			{
			}

			VowpalWabbitExample::VowpalWabbitExample(vw* vw, example* example, bool isEmpty) :
				m_vw(vw), m_example(example), m_isDisposed(false), m_isEmpty(isEmpty)
			{
			}

			VowpalWabbitExample::!VowpalWabbitExample()
			{
				if (m_isDisposed)
				{
					return;
				}

				VW::finish_example(*m_vw, m_example);

				m_isDisposed = true;
			}

			VowpalWabbitExample::~VowpalWabbitExample()
			{
				this->!VowpalWabbitExample();
			}

			float VowpalWabbitExample::Learn()
			{
				m_vw->learn(m_example);

				return VW::get_prediction(m_example);
			}

			float VowpalWabbitExample::Predict()
			{
				m_vw->l->predict(*m_example);

				//BUG: The below method may return garbage as it assumes a certain structure for ex->ld
				//which may not be the actual one used (e.g., for cost-sensitive multi-class learning)
				return VW::get_prediction(m_example);
			}

			float VowpalWabbitExample::CostSensitivePrediction::get()
			{
				return VW::get_cost_sensitive_prediction(m_example);
			}

			cli::array<int>^ VowpalWabbitExample::MultilabelPredictions::get()
			{
				size_t length;
				uint32_t* labels = VW::get_multilabel_predictions(m_example, length);

				auto result = gcnew cli::array<int>((int)length);
				Marshal::Copy(IntPtr(labels), result, 0, (int)length);

				return result;
			}

			bool VowpalWabbitExample::IsEmpty::get()
			{
				return m_isEmpty;
			}

			void VowpalWabbitExample::AddLabel(System::String^ label)
			{
				auto string = msclr::interop::marshal_as<std::string>(label);
				VW::parse_example_label(*m_vw, *m_example, string);
			}

			void VowpalWabbitExample::AddLabel(float label)
			{
				VW::add_label(m_example, label);
			}
			void VowpalWabbitExample::AddLabel(float label, float weight)
			{
				VW::add_label(m_example, label, weight);
			}

			void VowpalWabbitExample::AddLabel(float label, float weight, float base)
			{
				VW::add_label(m_example, label, weight, base);
			}
		}
	}
}