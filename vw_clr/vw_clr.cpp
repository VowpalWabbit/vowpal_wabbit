#include "vw_clr.h"

namespace Microsoft
{
	namespace Research
	{
		namespace  MachineLearning
		{
			VowpalWabbit::VowpalWabbit(System::String^ pArgs) : m_vw(nullptr), m_isDisposed(false)
			{
				auto string = msclr::interop::marshal_as<std::string>(pArgs);
				m_vw = VW::initialize(string);
				initialize_parser_datastructures(*m_vw);
			}

			VowpalWabbit::~VowpalWabbit()
			{
				this->!VowpalWabbit();
			}

			VowpalWabbit::!VowpalWabbit()
			{
				if (m_isDisposed)
				{
					return;
				}

				if (m_vw->numpasses > 1)
				{
					adjust_used_index(*m_vw);
					m_vw->do_reset_source = true;
					VW::start_parser(*m_vw, false);
					LEARNER::generic_driver(*m_vw);
					VW::end_parser(*m_vw);
				}
				else
				{
					release_parser_datastructures(*m_vw);
				}
				VW::finish(*m_vw);

				m_isDisposed = true;
			}

			VowpalWabbitExample^ VowpalWabbit::ReadExample(System::String^ line)
			{
				auto string = msclr::interop::marshal_as<std::string>(line);
				example* ex = VW::read_example(*m_vw, string.c_str());

				return gcnew VowpalWabbitExample(m_vw, ex);
			}

			VowpalWabbitExample^ VowpalWabbit::ImportExample(cli::array<FEATURE_SPACE>^ featureSpace)
			{
				pin_ptr<FEATURE_SPACE> ptr = &featureSpace[0];
				VW::primitive_feature_space * f = reinterpret_cast<VW::primitive_feature_space*>(ptr);

				example* ex = VW::import_example(*m_vw, f, featureSpace->Length);

				return gcnew VowpalWabbitExample(m_vw, ex);
			}
		}
	}
}