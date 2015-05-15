#include "vw_clr.h"

namespace Microsoft
{
	namespace Research
	{
		namespace MachineLearning
		{
			VowpalWabbitBase::VowpalWabbitBase(vw* vw) 
				: m_vw(vw), m_isDisposed(false)
			{
			}

			VowpalWabbitBase::VowpalWabbitBase(System::String^ pArgs) 
				: m_vw(nullptr), m_isDisposed(false)
			{
				auto string = msclr::interop::marshal_as<std::string>(pArgs);
				m_vw = VW::initialize(string);
				initialize_parser_datastructures(*m_vw);
			}

			VowpalWabbit::VowpalWabbit(System::String^ pArgs) 
				: VowpalWabbitBase(pArgs)
			{
			}

			VowpalWabbitBase::~VowpalWabbitBase()
			{
				this->!VowpalWabbitBase();
			}

			VowpalWabbitBase::!VowpalWabbitBase()
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

			VowpalWabbitModel::VowpalWabbitModel(System::String^ pArgs)
				: VowpalWabbitBase(pArgs)
			{
			}

			VowpalWabbit::VowpalWabbit(VowpalWabbitModel^ model) 
				: VowpalWabbitBase(VW::seed_vw_model(model->m_vw, ""))
			{
			}

			uint32_t VowpalWabbit::HashSpace(System::String^ s)
			{
				auto string = msclr::interop::marshal_as<std::string>(s);
				return VW::hash_space(*m_vw, string);
			}

			uint32_t VowpalWabbit::HashFeature(System::String^ s, unsigned long u)
			{
				auto string = msclr::interop::marshal_as<std::string>(s);
				return VW::hash_feature(*m_vw, string, u);
			}

			VowpalWabbitExample^ VowpalWabbit::ReadExample(System::String^ line)
			{
				auto string = msclr::interop::marshal_as<std::string>(line);

				auto ex = VW::read_example(*m_vw, string.c_str());
				return gcnew VowpalWabbitExample(m_vw, ex);
			}

			VowpalWabbitExample^ VowpalWabbit::ImportExample(cli::array<FeatureSpace^>^ featureSpaces)
			{
				auto f = new VW::primitive_feature_space[featureSpaces->Length];
				auto handles = gcnew cli::array<GCHandle>(featureSpaces->Length);

				for (int i = 0; i < featureSpaces->Length; i++)
				{
					auto fs = featureSpaces[i];
					auto pf = GCHandle::Alloc(fs->Features, GCHandleType::Pinned);
					handles[i] = pf;

					f[i].name = fs->Name;
					f[i].fs = static_cast<feature*>(pf.AddrOfPinnedObject().ToPointer());
					f[i].len = fs->Features->Length;
				}

				auto ex = VW::import_example(*m_vw, f, featureSpaces->Length);

				for (int i = 0; i < handles->Length; i++)
				{
					handles[i].Free();
				}
				delete f;

				return gcnew VowpalWabbitExample(m_vw, ex);
			}

			VowpalWabbitExample^ VowpalWabbit::CreateEmptyExample()
			{
				auto ex = VW::read_example(*m_vw, "");
				return gcnew VowpalWabbitExample(m_vw, ex, true);
			}
		}
	}
}