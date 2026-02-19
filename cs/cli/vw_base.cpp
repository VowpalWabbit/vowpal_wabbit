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
#include "vw/core/io_buf.h"
#include "vw/io/io_adapter.h"
#include "vw/common/vw_exception.h"
#include "vw/core/parse_args.h"
#include "vw/core/parse_regressor.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Text;

namespace VW
{
void trace_listener_cli(void* context, const std::string& message)
{
	auto listener = (Action<String^>^)GCHandle::FromIntPtr(IntPtr(context)).Target;
	auto str = gcnew String(message.c_str());
	listener(str);
}

VowpalWabbitBase::VowpalWabbitBase(VowpalWabbitSettings^ settings)
  : m_examples(nullptr), m_vw(nullptr), m_model(nullptr), m_settings(settings != nullptr ? settings : gcnew VowpalWabbitSettings), m_instanceCount(0)
{ if (m_settings->EnableThreadSafeExamplePooling)
    m_examples = Bag::CreateLockFree<VowpalWabbitExample^>(m_settings->MaxExamples);
  else
	m_examples = Bag::Create<VowpalWabbitExample^>(m_settings->MaxExamples);

  try
  { try
    { std::string string;
      if (settings->Arguments != nullptr)
        string = msclr::interop::marshal_as<std::string>(settings->Arguments);

	  trace_message_t trace_listener = nullptr;
	  void* trace_context = nullptr;

	  if (settings->TraceListener != nullptr)
	  {
		  m_traceListener = GCHandle::Alloc(settings->TraceListener);
		  trace_context = GCHandle::ToIntPtr(m_traceListener).ToPointer();
		  trace_listener = trace_listener_cli;
	  }

      if (settings->Model != nullptr)
      { m_model = settings->Model;
        if (!settings->Verbose && !settings->Arguments->Contains("--quiet") && !m_model->Arguments->CommandLine->Contains("--quiet"))
          string.append(" --quiet");
        m_vw = VW::seed_vw_model(m_model->m_vw, string, trace_listener, trace_context);
        m_model->IncrementReference();
      }
      else
      { if (!settings->Arguments->Contains("--no_stdin"))
		  string += " --no_stdin";
	    if (settings->ModelStream == nullptr)
        { if (!settings->Verbose && !settings->Arguments->Contains("--quiet"))
            string.append(" --quiet");

          m_vw = VW::initialize(string, nullptr, false, trace_listener, trace_context);
        }
        else
        {
          VW::io_buf model;
          auto* stream = new clr_stream_adapter(settings->ModelStream);
          model.add_file(std::unique_ptr<VW::io::reader>(stream));
          m_vw = VW::initialize(string, &model, false, trace_listener, trace_context);
          settings->ModelStream = nullptr;
        }
      }

    }
    catch (...)
    { // memory leak, but better than crashing
      m_vw = nullptr;
      throw;
    }
  }
  CATCHRETHROW
}

VowpalWabbitBase::~VowpalWabbitBase()
{ this->!VowpalWabbitBase();
  if (m_traceListener.IsAllocated)
	  m_traceListener.Free();
}

VowpalWabbitBase::!VowpalWabbitBase()
{ if (m_instanceCount <= 0)
  { this->InternalDispose();
  }
}

void VowpalWabbitBase::IncrementReference()
{ // thread-safe increase of model reference counter
  System::Threading::Interlocked::Increment(m_instanceCount);
}

void VowpalWabbitBase::DecrementReference()
{ // thread-safe decrease of model reference counter
  if (System::Threading::Interlocked::Decrement(m_instanceCount) <= 0)
  { this->InternalDispose();
  }
}

void VowpalWabbitBase::DisposeExample(VowpalWabbitExample^ ex)
{
  delete ex->m_example;

  // cleanup pointers in example chain
  auto inner = ex;
  while ((inner = inner->InnerExample) != nullptr)
  { inner->m_owner = nullptr;
    inner->m_example = nullptr;
  }

  ex->m_example = nullptr;

  // avoid that this example is returned again
  ex->m_owner = nullptr;
}

void VowpalWabbitBase::InternalDispose()
{ if (m_vw != nullptr)
  { // de-allocate example pools that are managed for each even shared instances
    if (m_examples != nullptr)
    { for each (auto ex in m_examples->RemoveAll())
        DisposeExample(ex);

      m_examples = nullptr;
    }
  }

  try
  { if (m_vw != nullptr)
    {
      VW::details::reset_source(*m_vw, m_vw->initial_weights_config.num_bits);

      // make sure don't try to free m_vw twice in case VW::finish throws.
      VW::workspace* vw_tmp = m_vw;
      m_vw = nullptr;
      VW::finish(*vw_tmp);
    }

    // don't add code here as in the case of VW::finish throws an exception it won't be called
  }
  CATCHRETHROW

  // Release the model reference AFTER finishing this workspace. The seeded workspace
  // shares weights and shared_data with the model via shallow_copy/shared_ptr.
  // If we decrement before finishing, the model may be destroyed while this workspace
  // still needs the shared resources, causing an AccessViolationException.
  if (m_model != nullptr)
  { m_model->DecrementReference();
    m_model = nullptr;
  }
}

VowpalWabbitSettings^ VowpalWabbitBase::Settings::get()
{ return m_settings;
}

VowpalWabbitArguments^ VowpalWabbitBase::Arguments::get()
{ if (m_arguments == nullptr)
  { m_arguments = gcnew VowpalWabbitArguments(m_vw);
  }

  return m_arguments;
}

void VowpalWabbitBase::Reload([System::Runtime::InteropServices::Optional] String^ args)
{ if (m_settings->ParallelOptions != nullptr)
  { throw gcnew NotSupportedException("Cannot reload model if AllReduce is enabled.");
  }

  if (args == nullptr)
    args = String::Empty;

  auto stringArgs = msclr::interop::marshal_as<std::string>(args);

  try
  {
    VW::details::reset_source(*m_vw, m_vw->initial_weights_config.num_bits);

    auto buffer = std::make_shared<std::vector<char>>();
    {
      VW::io_buf write_buffer;
      write_buffer.add_file(VW::io::create_vector_writer(buffer));
      VW::save_predictor(*m_vw, write_buffer);
    }

    // make sure don't try to free m_vw twice in case VW::finish throws.
    VW::workspace* vw_tmp = m_vw;
    m_vw = nullptr;
    VW::finish(*vw_tmp);

    // reload from model
    // seek to beginning
    VW::io_buf reader_view_of_buffer;
    reader_view_of_buffer.add_file(VW::io::create_buffer_view(buffer->data(), buffer->size()));
    m_vw = VW::initialize(stringArgs.c_str(), &reader_view_of_buffer);
  }
  CATCHRETHROW
}

String^ VowpalWabbitBase::AreFeaturesCompatible(VowpalWabbitBase^ other)
{ auto diff = VW::are_features_compatible(*m_vw, *other->m_vw);

  return diff == nullptr ? nullptr : gcnew String(diff);
}

String^ VowpalWabbitBase::ID::get()
{ return gcnew String(m_vw->id.c_str());
}

void VowpalWabbitBase::ID::set(String^ value)
{ m_vw->id = msclr::interop::marshal_as<std::string>(value);
}

void VowpalWabbitBase::SaveModel()
{ std::string name = m_vw->output_model_config.final_regressor_name;
  if (name.empty())
  { return;
  }

  // this results in extra marshaling but should be fine here
  this->SaveModel(gcnew String(name.c_str()));
}

void VowpalWabbitBase::SaveModel(String^ filename)
{ if (String::IsNullOrEmpty(filename))
    throw gcnew ArgumentException("Filename must not be null or empty");

  String^ directoryName = System::IO::Path::GetDirectoryName(filename);
  if (!String::IsNullOrEmpty(directoryName))
  {
    auto dir = msclr::interop::marshal_as<std::string>(directoryName);
    // CreateDirectoryA requires a LPCSTR (long, pointer to c-string) so .c_str() should work.
    // The second argument, lpSecurityAttributes, is optional.
    CreateDirectoryA(dir.c_str(), nullptr);
  }

  auto name = msclr::interop::marshal_as<std::string>(filename);

  try
  { VW::save_predictor(*m_vw, name);
  }
  CATCHRETHROW
}

void VowpalWabbitBase::SaveModel(Stream^ stream)
{ if (stream == nullptr)
    throw gcnew ArgumentException("stream");

  try
  {
    VW::io_buf buf;
    auto* stream_adapter = new clr_stream_adapter(stream);
    buf.add_file(std::unique_ptr<VW::io::writer>(stream_adapter));
    VW::save_predictor(*m_vw, buf);
  }
  CATCHRETHROW
}
}  // namespace VW
