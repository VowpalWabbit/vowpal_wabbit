#include "vw_clr.h"
#include "best_constant.h"

namespace Microsoft
{
	namespace Research
	{
		namespace MachineLearning
		{
			VowpalWabbitBase::VowpalWabbitBase(vw* vw)
				: m_vw(vw)
			{
			}

			VowpalWabbitBase::VowpalWabbitBase(System::String^ pArgs)
				: m_vw(nullptr)
			{
				try
				{
					auto string = msclr::interop::marshal_as<std::string>(pArgs);
					m_vw = VW::initialize(string);
					initialize_parser_datastructures(*m_vw);
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
			}

			void VowpalWabbitBase::InternalDispose()
			{
				try
				{
					if (m_vw)
					{
						release_parser_datastructures(*m_vw);

						VW::finish(*m_vw);
						m_vw = nullptr;
					}
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
			}

			void VowpalWabbitBase::RunMultiPass()
			{
				if (m_vw->numpasses > 1)
				{
					adjust_used_index(*m_vw);
					m_vw->do_reset_source = true;
					VW::start_parser(*m_vw, false);
					LEARNER::generic_driver(*m_vw);
					VW::end_parser(*m_vw);
				}
			}

			void VowpalWabbitBase::SaveModel()
			{
				string name = m_vw->final_regressor_name;
				if (name.empty())
				{
					return;
				}

				try
				{
					VW::save_predictor(*m_vw, name);
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
			}

			void VowpalWabbitBase::SaveModel(System::String^ filename)
			{
				if (System::String::IsNullOrEmpty(filename))
				{
					return;
				}

				auto name = msclr::interop::marshal_as<std::string>(filename);

				try
				{
					VW::save_predictor(*m_vw, name);
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
			}

			VowpalWabbitPerformanceStatistics^ VowpalWabbitBase::PerformanceStatistics::get()
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
		}
	}
}