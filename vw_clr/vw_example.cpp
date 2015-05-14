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
				//return VowpalWabbitInterface.Learn(this.vw, example.Ptr);
				m_vw->learn(m_example);

				return VW::get_prediction(m_example);
			}

			float VowpalWabbitExample::Predict()
			{
				//return VowpalWabbitInterface.Learn(this.vw, example.Ptr);
				m_vw->l->predict(*m_example);

				//BUG: The below method may return garbage as it assumes a certain structure for ex->ld
				//which may not be the actual one used (e.g., for cost-sensitive multi-class learning)
				return VW::get_prediction(m_example);
			}

			//float Fo()
			//{
			//		return VW::get_multilabel_predictions(*pointer, static_cast<example*>(e), *plen);
			//}

			bool VowpalWabbitExample::IsEmpty()
			{
				return m_isEmpty;
			}
		}


	}
}