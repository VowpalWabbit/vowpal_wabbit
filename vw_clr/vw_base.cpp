/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"
#include "vw_base.h"
#include "vw_model.h"
#include "vw_prediction.h"
#include "vw_example.h"

#include "clr_io.h"
#include "clr_io_memory.h"
#include "vw_exception.h"
#include "parse_args.h"
#include "parse_regressor.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Text;

namespace VW
{
    VowpalWabbitBase::VowpalWabbitBase(VowpalWabbitSettings^ settings)
        : m_examples(nullptr), m_vw(nullptr), m_model(nullptr), m_settings(settings != nullptr ? settings : gcnew VowpalWabbitSettings), m_instanceCount(0)
    {
        m_examples = gcnew Stack<VowpalWabbitExample^>;

        try
        {
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
                        InitializeFromModel(string, model);
                        settings->ModelStream->Close();
                    }
                }
            }
            catch (...)
            {
                // memory leak, but better than crashing
                m_vw = nullptr;
                throw;
            }
        }
        CATCHRETHROW
    }

    void VowpalWabbitBase::InitializeFromModel(string args, io_buf& model)
    {
        char** argv = nullptr;
        int argc = 0;

        args.append(" --no_stdin");
        argv = VW::get_argv_from_string(args, argc);

        m_vw = &parse_args(argc, argv);

        try
        {
            parse_modules(*m_vw, model);
            parse_sources(*m_vw, model);
            initialize_parser_datastructures(*m_vw);
        }
        catch (...)
        {
            VW::finish(*m_vw);
            m_vw = nullptr;
            throw;
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

    VowpalWabbitBase::~VowpalWabbitBase()
    {
        this->!VowpalWabbitBase();
    }

    VowpalWabbitBase::!VowpalWabbitBase()
    {
        if (m_instanceCount <= 0)
        {
            this->InternalDispose();
        }
    }

    void VowpalWabbitBase::IncrementReference()
    {
        // thread-safe increase of model reference counter
        System::Threading::Interlocked::Increment(m_instanceCount);
    }

    void VowpalWabbitBase::DecrementReference()
    {
        // thread-safe decrease of model reference counter
        if (System::Threading::Interlocked::Decrement(m_instanceCount) <= 0)
        {
            this->InternalDispose();
        }
    }

    void VowpalWabbitBase::InternalDispose()
    {
        if (m_vw != nullptr)
        {
            // de-allocate example pools that are managed for each even shared instances
            auto multilabel_prediction = m_vw->multilabel_prediction;
            auto delete_label = m_vw->p->lp.delete_label;

            if (m_examples != nullptr)
            {
                for each (auto ex in m_examples)
                {
                    if (multilabel_prediction)
                    {
                        VW::dealloc_example(delete_label, *ex->m_example, MULTILABEL::multilabel.delete_label);
                    }
                    else
                    {
                        VW::dealloc_example(delete_label, *ex->m_example);
                    }

                    ::free_it(ex->m_example);

                    // cleanup pointers in example chain
                    auto inner = ex;
                    while ((inner = inner->InnerExample) != nullptr)
                    {
                        inner->m_owner = nullptr;
                        inner->m_example = nullptr;
                    }

                    ex->m_example = nullptr;

                    // avoid that this example is returned again
                    ex->m_owner = nullptr;
                }

                m_examples = nullptr;
            }

            if (m_model != nullptr)
            {
                // this object doesn't own the VW instance
                m_model->DecrementReference();
                m_model = nullptr;
            }
        }

        try
        {
            if (m_vw != nullptr)
            {
                release_parser_datastructures(*m_vw);

                // make sure don't try to free m_vw twice in case VW::finish throws.
                vw* vw_tmp = m_vw;
                m_vw = nullptr;
                VW::finish(*vw_tmp);
            }

            // don't add code here as in the case of VW::finish throws an exception it won't be called
        }
        CATCHRETHROW
    }

    VowpalWabbitSettings^ VowpalWabbitBase::Settings::get()
    {
        return m_settings;
    }

    VowpalWabbitArguments^ VowpalWabbitBase::Arguments::get()
    {
        if (m_arguments == nullptr)
        {
            m_arguments = gcnew VowpalWabbitArguments(m_vw);
        }

        return m_arguments;
    }

    VowpalWabbitExample^ VowpalWabbitBase::GetOrCreateNativeExample()
    {
        if (m_examples->Count == 0)
        {
            try
            {
                auto ex = VW::alloc_examples(0, 1);
                m_vw->p->lp.default_label(&ex->l);
                return gcnew VowpalWabbitExample(this, ex);
            }
            CATCHRETHROW
        }

        auto ex = m_examples->Pop();

        try
        {
            VW::empty_example(*m_vw, *ex->m_example);
            m_vw->p->lp.default_label(&ex->m_example->l);

            return ex;
        }
        CATCHRETHROW
    }

    VowpalWabbitExample^ VowpalWabbitBase::GetOrCreateEmptyExample()
    {
        VowpalWabbitExample^ ex = nullptr;

        try
        {
            ex = GetOrCreateNativeExample();

            char empty = '\0';
            VW::read_line(*m_vw, ex->m_example, &empty);

            VW::parse_atomic_example(*m_vw, ex->m_example, false);
            VW::setup_example(*m_vw, ex->m_example);

            return ex;
        }
        catch (...)
        {
            delete ex;
            throw;
        }
    }

    void VowpalWabbitBase::ReturnExampleToPool(VowpalWabbitExample^ ex)
    {
#if _DEBUG
        if (m_vw == nullptr)
            throw gcnew ObjectDisposedException("VowpalWabbitExample was not properly disposed as the owner is already disposed");

        if (ex == nullptr)
            throw gcnew ArgumentNullException("ex");
#endif

        // make sure we're not a ring based example
        assert(!VW::is_ring_example(*m_vw, ex->m_example));

        if (m_examples != nullptr)
            m_examples->Push(ex);
#if _DEBUG
        else // this should not happen as m_vw is already set to null
            throw gcnew ObjectDisposedException("VowpalWabbitExample was disposed after the owner is disposed");
#endif
    }

    void VowpalWabbitBase::Reload([System::Runtime::InteropServices::Optional] String^ args)
    {
        if (m_settings->ParallelOptions != nullptr)
        {
            throw gcnew NotSupportedException("Cannot reload model if AllRecude is enabled.");
        }

        clr_io_memory_buf mem_buf;

        if (args == nullptr)
            args = String::Empty;

        auto stringArgs = msclr::interop::marshal_as<std::string>(args);

        try
        {
            VW::save_predictor(*m_vw, mem_buf);
            mem_buf.flush();

            release_parser_datastructures(*m_vw);

            // make sure don't try to free m_vw twice in case VW::finish throws.
            vw* vw_tmp = m_vw;
            m_vw = nullptr;
            VW::finish(*vw_tmp);

            // reload from model
            // seek to beginning
            mem_buf.reset_file(0);
            InitializeFromModel(stringArgs.c_str(), mem_buf);
        }
        CATCHRETHROW
    }

    String^ VowpalWabbitBase::AreFeaturesCompatible(VowpalWabbitBase^ other)
    {
        auto diff = VW::are_features_compatible(*m_vw, *other->m_vw);

        return diff == nullptr ? nullptr : gcnew String(diff);
    }

    String^ VowpalWabbitBase::ID::get()
    {
        return gcnew String(m_vw->id.c_str());
    }

    void VowpalWabbitBase::ID::set(String^ value)
    {
        m_vw->id = msclr::interop::marshal_as<std::string>(value);
    }
}
