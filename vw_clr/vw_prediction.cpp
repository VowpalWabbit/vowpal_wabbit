/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_prediction.h"
#include "vw_example.h"
#include "vw_base.h"
#include "vowpalwabbit.h"

namespace VW
{
	float VowpalWabbitScalarPredictionFactory::Create(vw* vw, example* ex)
	{
#if _DEBUG
		if (ex == nullptr)
			throw gcnew ArgumentNullException("ex");
#endif

		try
		{
			return VW::get_prediction(ex);
		}
		CATCHRETHROW
	}

	float VowpalWabbitCostSensitivePredictionFactory::Create(vw* vw, example* ex)
	{
#if _DEBUG
		if (ex == nullptr)
			throw gcnew ArgumentNullException("ex");
#endif

		try
		{
			return VW::get_cost_sensitive_prediction(ex);
		}
		CATCHRETHROW
	}

	cli::array<int>^ VowpalWabbitMultilabelPredictionFactory::Create(vw* vw, example* ex)
	{
#if _DEBUG
		if (ex == nullptr)
			throw gcnew ArgumentNullException("ex");
#endif

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

		auto values = gcnew cli::array<int>((int)length);
		Marshal::Copy(IntPtr(labels), values, 0, (int)length);

		return values;
	}

	cli::array<float>^ VowpalWabbitTopicPredictionFactory::Create(vw* vw, example* ex)
	{
#if _DEBUG
		if (vw == nullptr)
			throw gcnew ArgumentNullException("vw");

		if (ex == nullptr)
			throw gcnew ArgumentNullException("ex");
#endif

		auto values = gcnew cli::array<float>(vw->lda);
		Marshal::Copy(IntPtr(ex->topic_predictions.begin), values, 0, vw->lda);

		return values;
	}
}
