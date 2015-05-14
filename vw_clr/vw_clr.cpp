// This is the main DLL file.

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

			VowpalWabbitExample::VowpalWabbitExample(vw* vw, example* example) :
				m_vw(vw), m_example(example), m_isDisposed(false)
			{
			}

			VowpalWabbitExample::!VowpalWabbitExample()
			{
				// TODO: cleanup by calling finish
			}

			VowpalWabbitExample::~VowpalWabbitExample()
			{
				this->!VowpalWabbitExample();
			}
		}
	}
}