/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"

namespace Microsoft
{
	namespace Research
	{
		namespace MachineLearning
		{
			void VowpalWabbitPrediction::ReadFromExample(VowpalWabbitExample^ example)
			{
				ReadFromExample(example->m_vw->m_vw, example->m_example);
			}

			void VowpalWabbitScalarPrediction::ReadFromExample(vw* vw, example* ex)
			{
				Value = VW::get_prediction(ex);
			}

			void VowpalWabbitCostSensitivePrediction::ReadFromExample(vw* vw, example* ex)
			{
				Value = VW::get_cost_sensitive_prediction(ex);
			}

			void VowpalWabbitMultilabelPrediction::ReadFromExample(vw* vw, example* ex)
			{
				size_t length;
				uint32_t* labels = VW::get_multilabel_predictions(ex, length);

				if (length > Int32::MaxValue)
				{
					throw gcnew ArgumentOutOfRangeException("Multi-label predictions too large");
				}

				Values = gcnew cli::array<int>((int)length);
				Marshal::Copy(IntPtr(labels), Values, 0, (int)length);
			}

			void VowpalWabbitTopicPrediction::ReadFromExample(vw* vw, example* ex)
			{
				Values = gcnew cli::array<float>(vw->lda);
				Marshal::Copy(IntPtr(ex->topic_predictions.begin), Values, 0, vw->lda);
			}

			void VowpalWabbitPredictionNone::ReadFromExample(vw* vw, example* ex)
			{
				// no-op
			}
		}
	}
}