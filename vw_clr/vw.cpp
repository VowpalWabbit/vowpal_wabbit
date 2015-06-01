#include "vw_clr.h"

namespace Microsoft
{
	namespace Research
	{
		namespace MachineLearning
		{
			VowpalWabbit::VowpalWabbit(System::String^ pArgs)
				: VowpalWabbitBase(pArgs)
			{
			}

			vw* wrapped_seed_vw_model(vw* vw)
			{
				try
				{
					return VW::seed_vw_model(vw, "");
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
			}

			VowpalWabbit::VowpalWabbit(VowpalWabbitModel^ model)
				: VowpalWabbitBase(wrapped_seed_vw_model(model->m_vw)), m_model(model)
			{
				m_model->IncrementReference();
			}

			VowpalWabbit::~VowpalWabbit()
			{
				this->!VowpalWabbit();
			}

			VowpalWabbit::!VowpalWabbit()
			{
				if (m_model != nullptr)
				{
					// this object doesn't own the VW instance
					m_model->DecrementReference();
					m_model = nullptr;
				}
				else
				{
					// this object owns the VW instance.
					this->InternalDispose();
				}
			}

			uint32_t VowpalWabbit::HashSpace(System::String^ s)
			{
				auto string = msclr::interop::marshal_as<std::string>(s);

				try
				{
					return VW::hash_space(*m_vw, string);
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
			}

			uint32_t VowpalWabbit::HashFeature(System::String^ s, unsigned long u)
			{
				try
				{
					auto string = msclr::interop::marshal_as<std::string>(s);
					return VW::hash_feature(*m_vw, string, u);
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
			}

			VowpalWabbitExample^ VowpalWabbit::ReadExample(System::String^ line)
			{
				auto string = msclr::interop::marshal_as<std::string>(line);

				try
				{
					auto ex = VW::read_example(*m_vw, string.c_str());
					return gcnew VowpalWabbitExample(m_vw, ex);
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
			}

			VowpalWabbitExample^ VowpalWabbit::CreateEmptyExample()
			{
				try
				{
					auto ex = VW::read_example(*m_vw, "");
					return gcnew VowpalWabbitExample(m_vw, ex);
				}
				catch (std::exception const& ex)
				{
					throw gcnew System::Exception(gcnew System::String(ex.what()));
				}
			}
		}
	}
}