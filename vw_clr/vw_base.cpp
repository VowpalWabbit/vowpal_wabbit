/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"
#include "best_constant.h"
#include "clr_io.h"
#include "parser.h"
#include "parse_regressor.h"
#include "parse_args.h"

namespace Microsoft
{
	namespace Research
	{
		namespace MachineLearning
		{
			VowpalWabbitBase::VowpalWabbitBase(vw* vw)
				: m_vw(vw)
			{
                m_examples = nullptr;
			}

			VowpalWabbitBase::VowpalWabbitBase(System::String^ args)
                : VowpalWabbitBase((vw*)nullptr)
			{
				try
				{
					auto string = msclr::interop::marshal_as<std::string>(args);
					m_vw = VW::initialize(string);
					initialize_parser_datastructures(*m_vw);
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
			}

			VowpalWabbitBase::VowpalWabbitBase(System::String^ args, System::IO::Stream^ stream)
                : VowpalWabbitBase((vw*)nullptr)
			{
				clr_io_buf model(stream);
				char** argv = nullptr;
				int argc = 0;
				try
				{
					auto string = msclr::interop::marshal_as<std::string>(args);
					string += " --no_stdin";
					argv = VW::get_argv_from_string(string, argc);

					vw& all = parse_args(argc, argv);
					parse_modules(all, model);
					parse_sources(all, model);
					initialize_parser_datastructures(all);

					m_vw = &all;
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
				finally
				{
					if (argv != nullptr)
					{
						for (int i = 0; i < argc; i++)
							free(argv[i]);
						free(argv);
					}
				}
			}

            example* VowpalWabbitBase::GetOrCreateNativeExample()
            {
                example* ex = nullptr;
                if (m_examples != nullptr && !m_examples->empty())
                {
                    ex = m_examples->top();
                    m_examples->pop();
                }
                else
                {
                    ex = VW::alloc_examples(0, 1);
                }
                return ex;
            }

            void VowpalWabbitBase::ReturnExampleToPool(example* ex)
            {
                if (m_examples == nullptr)
                {
                    m_examples = new stack<example*>();
                }

                VW::empty_example(*m_vw, *ex);

                m_examples->push(ex);
            }

			void VowpalWabbitBase::InternalDispose()
			{
				try
				{
					if (m_examples != nullptr)
					{
						while (!m_examples->empty())
						{
							example* ex = m_examples->top();

							if (m_vw->multilabel_prediction)
							{
								VW::dealloc_example(m_vw->p->lp.delete_label, *ex, MULTILABEL::multilabel.delete_label);
							}
							else
							{
								VW::dealloc_example(m_vw->p->lp.delete_label, *ex);
							}

							::free_it(ex);

							m_examples->pop();
						}
						delete m_examples;
						m_examples = nullptr;
					}

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
                // this results in extra marshaling but should be fine here
                this->SaveModel(gcnew String(name.c_str()));
			}

			void VowpalWabbitBase::SaveModel(System::String^ filename)
			{
				if (System::String::IsNullOrEmpty(filename))
				{
					return;
				}

                System::String^ directoryName = System::IO::Path::GetDirectoryName(filename);
                if (!System::String::IsNullOrEmpty(directoryName))
                {
                    System::IO::Directory::CreateDirectory(directoryName);
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