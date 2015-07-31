/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"
#include "vowpalwabbit.h"
#include "best_constant.h"
#include "parser.h"
#include "hash.h"

using namespace System;
using namespace System::Text;

namespace VW
{
	VowpalWabbitNative::VowpalWabbitNative(VowpalWabbitSettings^ pArgs)
		: VowpalWabbitBase(pArgs)
	{
	}

	generic<typename TPrediction>
		where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
	TPrediction VowpalWabbitNative::PredictOrLearn(String^ line, bool predict)
	{
		auto bytes = System::Text::Encoding::UTF8->GetBytes(line);
		auto lineHandle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

		example* ex = nullptr;
		try
		{
			ex = VW::read_example(*m_vw, reinterpret_cast<char*>(lineHandle.AddrOfPinnedObject().ToPointer()));

			if (predict)
				m_vw->l->predict(*ex);
			else
				m_vw->learn(ex);

			auto prediction = gcnew TPrediction();
			prediction->ReadFromExample(m_vw, ex);

			m_vw->l->finish_example(*m_vw, *ex);
			ex = nullptr;

			return prediction;
		}
		CATCHRETHROW
		finally
		{
			lineHandle.Free();

			if (ex != nullptr)
			{
				VW::finish_example(*m_vw, ex);
			}
		}
	}

	generic<typename TPrediction>
		where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
	TPrediction VowpalWabbitNative::LearnAndPredict(String^ line)
	{
		return PredictOrLearn<TPrediction>(line, false);
	}

	generic<typename TPrediction>
		where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
	TPrediction VowpalWabbitNative::Predict(String^ line)
	{
		return PredictOrLearn<TPrediction>(line, true);
	}

	void VowpalWabbitNative::Learn(String^ line)
	{
		auto bytes = System::Text::Encoding::UTF8->GetBytes(line);
		auto lineHandle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

		example* ex = nullptr;
		try
		{
			ex = VW::read_example(*m_vw, reinterpret_cast<char*>(lineHandle.AddrOfPinnedObject().ToPointer()));

			m_vw->learn(ex);

			m_vw->l->finish_example(*m_vw, *ex);
			ex = nullptr;
		}
		CATCHRETHROW
		finally
		{
			lineHandle.Free();

			if (ex != nullptr)
			{
				VW::finish_example(*m_vw, ex);
			}
		}
	}

	void VowpalWabbitNative::PredictAndDiscard(String^ line)
	{
		auto bytes = System::Text::Encoding::UTF8->GetBytes(line);
		auto lineHandle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

		example* ex = nullptr;
		try
		{
			ex = VW::read_example(*m_vw, reinterpret_cast<char*>(lineHandle.AddrOfPinnedObject().ToPointer()));

			m_vw->l->predict(*ex);

			m_vw->l->finish_example(*m_vw, *ex);
			ex = nullptr;
		}
		CATCHRETHROW
		finally
		{
			lineHandle.Free();

			if (ex != nullptr)
			{
				VW::finish_example(*m_vw, ex);
			}
		}
	}


	void VowpalWabbitNative::Driver()
	{
		try
		{
			LEARNER::generic_driver(*m_vw);
		}
		CATCHRETHROW
	}

	void VowpalWabbitNative::RunMultiPass()
	{
		if (m_vw->numpasses > 1)
		{
			try
			{
				adjust_used_index(*m_vw);
				m_vw->do_reset_source = true;
				VW::start_parser(*m_vw, false);
				LEARNER::generic_driver(*m_vw);
				VW::end_parser(*m_vw);
			}
			CATCHRETHROW
		}
	}

	void VowpalWabbitNative::SaveModel()
	{
		string name = m_vw->final_regressor_name;
		if (name.empty())
		{
			return;
		}
		// this results in extra marshaling but should be fine here
		this->SaveModel(gcnew String(name.c_str()));
	}

	void VowpalWabbitNative::SaveModel(String^ filename)
	{
		if (String::IsNullOrEmpty(filename))
		{
			return;
		}

		String^ directoryName = System::IO::Path::GetDirectoryName(filename);

		if (!String::IsNullOrEmpty(directoryName))
		{
			System::IO::Directory::CreateDirectory(directoryName);
		}

		auto name = msclr::interop::marshal_as<std::string>(filename);

		try
		{
			VW::save_predictor(*m_vw, name);
		}
		CATCHRETHROW
	}

	VowpalWabbitPerformanceStatistics^ VowpalWabbitNative::PerformanceStatistics::get()
	{
		// see parse_args.cc:finish(...)

		auto stats = gcnew VowpalWabbitPerformanceStatistics();

		if (m_vw->current_pass == 0)
		{
			stats->NumberOfExamplesPerPass = m_vw->sd->example_number;
		}
		else
		{
			stats->NumberOfExamplesPerPass = m_vw->sd->example_number / m_vw->current_pass;
		}

		stats->WeightedExampleSum = m_vw->sd->weighted_examples;
		stats->WeightedLabelSum = m_vw->sd->weighted_labels;

		if (m_vw->holdout_set_off || (m_vw->sd->holdout_best_loss == FLT_MAX))
		{
			stats->AverageLoss = m_vw->sd->sum_loss / m_vw->sd->weighted_examples;
		}
		else
		{
			stats->AverageLoss = m_vw->sd->holdout_best_loss;
		}

		float best_constant; float best_constant_loss;
		if (get_best_constant(*m_vw, best_constant, best_constant_loss))
		{
			stats->BestConstant = best_constant;
			if (best_constant_loss != FLT_MIN)
			{
				stats->BestConstantLoss = best_constant_loss;
			}
		}

		stats->TotalNumberOfFeatures = m_vw->sd->total_features;

		return stats;
	}

	uint32_t VowpalWabbitNative::HashSpace(String^ s)
	{
		auto newHash = m_hasher(s, hash_base);

#ifdef DEBUG
		auto oldHash = HashSpaceNative(s);
		assert(newHash == oldHash);
#endif

		return (uint32_t)newHash;
	}

	uint32_t VowpalWabbitNative::HashFeature(String^ s, unsigned long u)
	{
		auto newHash = m_hasher(s, u) & m_vw->parse_mask;

#ifdef DEBUG
		auto oldHash = HashFeatureNative(s, u);
		assert(newHash == oldHash);
#endif

		return (uint32_t)newHash;
	}

	uint32_t VowpalWabbitNative::HashSpaceNative(String^ s)
	{
		auto bytes = System::Text::Encoding::UTF8->GetBytes(s);
		auto handle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

		try
		{
			return VW::hash_space(*m_vw, reinterpret_cast<char*>(handle.AddrOfPinnedObject().ToPointer()));
		}
		CATCHRETHROW
			finally
		{
			handle.Free();
		}
	}

	uint32_t VowpalWabbitNative::HashFeatureNative(String^ s, unsigned long u)
	{
		auto bytes = System::Text::Encoding::UTF8->GetBytes(s);
		auto handle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

		try
		{
			return VW::hash_feature(*m_vw, reinterpret_cast<char*>(handle.AddrOfPinnedObject().ToPointer()), u);
		}
		CATCHRETHROW
			finally
		{
			handle.Free();
		}
	}

	void VowpalWabbitNative::Learn(example* ex)
	{
		try
		{
			m_vw->learn(ex);

			// as this is not a ring-based example it is not free'd
			m_vw->l->finish_example(*m_vw, *ex);
		}
		CATCHRETHROW
	}

	void VowpalWabbitNative::PredictAndDiscard(example* ex)
	{
		try
		{
			m_vw->l->predict(*ex);

			// as this is not a ring-based example it is not free'd
			m_vw->l->finish_example(*m_vw, *ex);
		}
		CATCHRETHROW
	}

	void VowpalWabbitNative::LearnAndPredict(example* ex, VowpalWabbitPrediction^ result)
	{
		try
		{
			m_vw->learn(ex);

			result->ReadFromExample(m_vw, ex);

			// as this is not a ring-based example it is not free'd
			m_vw->l->finish_example(*m_vw, *ex);
		}
		CATCHRETHROW
	}

	void VowpalWabbitNative::Predict(example* ex, VowpalWabbitPrediction^ result)
	{
		try
		{
			m_vw->l->predict(*ex);

			result->ReadFromExample(m_vw, ex);

			// as this is not a ring-based example it is not free'd
			m_vw->l->finish_example(*m_vw, *ex);
		}
		CATCHRETHROW
	}

	VowpalWabbitNative^ VowpalWabbitNative::Underlying::get()
	{
		return this;
	}
}