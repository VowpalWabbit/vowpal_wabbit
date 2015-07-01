/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"

namespace VW
{
	void VowpalWabbitPrediction::ReadFromExample(VowpalWabbitExample^ example)
	{
		// dispatch to sub classes
		ReadFromExample(example->m_vw->m_vw, example->m_example);
	}

	void VowpalWabbitScalarPrediction::ReadFromExample(vw* vw, example* ex)
	{
		try
		{
			Value = VW::get_prediction(ex);
		}
		CATCHRETHROW
	}

	void VowpalWabbitCostSensitivePrediction::ReadFromExample(vw* vw, example* ex)
	{
		try
		{
			Value = VW::get_cost_sensitive_prediction(ex);
		}
		CATCHRETHROW
	}

	void VowpalWabbitMultilabelPrediction::ReadFromExample(vw* vw, example* ex)
	{
		size_t length;
		uint32_t* labels;
		
		try
		{
			labels = VW::get_multilabel_predictions(ex, length);
		}
		CATCHRETHROW

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
}