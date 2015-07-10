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
#include "vw_exception.h"
#include "hash.h"

using namespace System;
using namespace System::Text;

namespace VW
{
	VowpalWabbitBase::VowpalWabbitBase(vw* vw)
		: m_vw(vw)
	{
		m_examples = nullptr;

		if (m_vw != nullptr)
		{
			m_hasher = GetHasher();
		}
	}

	VowpalWabbitBase::VowpalWabbitBase(String^ args)
        : VowpalWabbitBase((vw*)nullptr)
	{
		try
		{
			auto string = msclr::interop::marshal_as<std::string>(args);
			m_vw = VW::initialize(string);
			initialize_parser_datastructures(*m_vw);

			m_hasher = GetHasher();
		}
		CATCHRETHROW
	}

	VowpalWabbitBase::VowpalWabbitBase(String^ args, System::IO::Stream^ stream)
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

			m_hasher = GetHasher();
		}
		CATCHRETHROW
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

	void VowpalWabbitBase::RunMultiPass()
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

	void VowpalWabbitBase::SaveModel(String^ filename)
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

	uint32_t VowpalWabbitBase::HashSpace(String^ s)
	{
		auto newHash = m_hasher(s, hash_base);

#ifdef DEBUG
		auto oldHash = HashSpaceNative(s);
		assert(newHash == oldHash);
#endif

		return (uint32_t)newHash;
	}

	uint32_t VowpalWabbitBase::HashFeature(String^ s, unsigned long u)
	{
		auto newHash = m_hasher(s, u) & m_vw->parse_mask;

#ifdef DEBUG
		auto oldHash = HashFeatureNative(s, u);
		assert(newHash == oldHash);
#endif

		return (uint32_t)newHash;
	}

	uint32_t VowpalWabbitBase::HashSpaceNative(String^ s)
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

	uint32_t VowpalWabbitBase::HashFeatureNative(String^ s, unsigned long u)
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
}