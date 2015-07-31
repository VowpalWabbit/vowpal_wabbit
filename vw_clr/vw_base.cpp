/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"
#include "vw_base.h"
#include "vw_model.h"
#include "vw_prediction.h"

#include "clr_io.h"
#include "vw_exception.h"
#include "parse_args.h"
#include "parse_regressor.h"

using namespace System;
using namespace System::Text;

namespace VW
{
	VowpalWabbitBase::VowpalWabbitBase(VowpalWabbitSettings^ settings)
		: m_examples(nullptr), m_vw(nullptr), m_model(nullptr)
	{
		if (settings == nullptr)
		{
			settings = gcnew VowpalWabbitSettings;
		}

		m_settings = settings;

		try
		{
			auto string = msclr::interop::marshal_as<std::string>(settings->Arguments);

			if (settings->Model != nullptr)
			{
				m_model = settings->Model;
				m_vw = VW::seed_vw_model(m_model->m_vw, string);
				m_model->IncrementReference();
			}
			else 
			{
				if (settings->ModelStream == nullptr)
				{
					m_vw = VW::initialize(string);
				}
				else
				{
					clr_io_buf model(settings->ModelStream);
					char** argv = nullptr;
					int argc = 0;

					try
					{
						string += " --no_stdin";
						argv = VW::get_argv_from_string(string, argc);

						vw& all = parse_args(argc, argv);
						parse_modules(all, model);
						parse_sources(all, model);
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

				initialize_parser_datastructures(*m_vw);
			}

			m_hasher = GetHasher();
		}
		CATCHRETHROW
	}

	VowpalWabbitBase::~VowpalWabbitBase()
	{
		this->!VowpalWabbitBase();
	}

	VowpalWabbitBase::!VowpalWabbitBase()
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

	VowpalWabbitSettings^ VowpalWabbitBase::Settings::get()
	{
		return m_settings;
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
			try
			{
				ex = VW::alloc_examples(0, 1);
			}
			CATCHRETHROW
        }
        return ex;
    }

    void VowpalWabbitBase::ReturnExampleToPool(example* ex)
    {
		// make sure we're not a ring based example 
		assert(!VW::is_ring_example(*m_vw, ex));

        if (m_examples == nullptr)
        {
            m_examples = new stack<example*>();
        }

		try
		{
			VW::empty_example(*m_vw, *ex);
		}
		CATCHRETHROW

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

			if (m_vw != nullptr)
			{
				release_parser_datastructures(*m_vw);

				// make sure don't try to free m_vw twice in case VW::finish throws.
				vw* vw_tmp = m_vw;
				m_vw = nullptr;
				VW::finish(*vw_tmp);
			}
			
			// don't add code here as in the case of VW::finish thrown an exception it won't be called
		}
		CATCHRETHROW
	}

	/// <summary>
	/// Hashes the given value <paramref name="s"/>.
	/// </summary>
	/// <param name="s">String to be hashed.</param>
	/// <param name="u">Hash offset.</param>
	/// <returns>The resulting hash code.</returns>
	size_t hashall(String^ s, unsigned long u)
	{
		// get raw bytes from string
		auto keys = Encoding::UTF8->GetBytes(s);

		uint32_t h1 = u;
		uint32_t k1 = 0;

		const uint32_t c1 = 0xcc9e2d51;
		const uint32_t c2 = 0x1b873593;

		int length = keys->Length;
		int i = 0;
		while (i <= length - 4)
		{
			// convert byte array to integer
			k1 = (uint32_t)(keys[i] | keys[i + 1] << 8 | keys[i + 2] << 16 | keys[i + 3] << 24);

			k1 *= c1;
			k1 = ROTL32(k1, 15);
			k1 *= c2;

			h1 ^= k1;
			h1 = ROTL32(h1, 13);
			h1 = h1 * 5 + 0xe6546b64;

			i += 4;
		}

		k1 = 0;
		int tail = length - length % 4;
		switch (length & 3)
		{
		case 3:
			k1 ^= (uint32_t)(keys[tail + 2] << 16);
		case 2:
			k1 ^= (uint32_t)(keys[tail + 1] << 8);
		case 1:
			k1 ^= (uint32_t)(keys[tail]);
			k1 *= c1;
			k1 = ROTL32(k1, 15);
			k1 *= c2;
			h1 ^= k1;
			break;
		}

		// finalization
		h1 ^= (uint32_t)length;

		return MURMUR_HASH_3::fmix(h1);
	}

	/// <summary>
	/// Hashes the given value <paramref name="s"/>.
	/// </summary>
	/// <param name="s">String to be hashed.</param>
	/// <param name="u">Hash offset.</param>
	/// <returns>The resulting hash code.</returns>
	size_t hashstring(String^ s, unsigned long u)
	{
		s = s->Trim();

		int sInt = 0;
		if (int::TryParse(s, sInt))
		{
			return sInt + u;
		}
		else
		{
			return hashall(s, u);
		}
	}

	Func<String^, unsigned long, size_t>^ VowpalWabbitBase::GetHasher()
	{
		//feature manipulation
		string hash_function("strings");
		if (m_vw->vm.count("hash"))
		{
			hash_function = m_vw->vm["hash"].as<string>();
		}

		if (hash_function == "strings")
		{
			return gcnew Func<String^, unsigned long, size_t>(&hashstring);
		}
		else if (hash_function == "all")
		{
			return gcnew Func<String^, unsigned long, size_t>(&hashall);

		}
		else
		{
			THROW("Unsupported hash function: " << hash_function);
		}
	}
}