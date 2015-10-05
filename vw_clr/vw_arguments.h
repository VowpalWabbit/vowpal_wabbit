/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include <msclr\marshal_cppstd.h>
#include "vw.h"

using namespace System;
using namespace System::Collections::Generic;

namespace VW
{
  public ref class VowpalWabbitArguments
  {
  private:
    initonly String^ m_data;
    const bool m_testonly;
    const int m_passes;

  internal:
    VowpalWabbitArguments(vw* vw) :
      m_data(gcnew String(vw->data_filename.c_str())),
      m_testonly(!vw->training),
      m_passes((int)vw->numpasses)
    {
    }

  public:
    property String^ Data
    {
      String^ get()
      {
        return m_data;
      }
    }

    property bool TestOnly
    {
      bool get()
      {
        return m_testonly;
      }
    }

    property int NumPasses
    {
      int get()
      {
        return m_passes;
      }
    }
  };
}
