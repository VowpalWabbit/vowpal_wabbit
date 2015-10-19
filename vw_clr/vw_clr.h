/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "vw.h"
#include "vw_settings.h"

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace VW
{
  /// <summary>
  /// Collected performance statistics.
  /// </summary>
  public ref class VowpalWabbitPerformanceStatistics
  {
  public:
    property uint64_t TotalNumberOfFeatures;

    property double WeightedExampleSum;

    property uint64_t NumberOfExamplesPerPass;

    property double WeightedLabelSum;

    property double AverageLoss;

    property double BestConstant;

    property double BestConstantLoss;
  };

  /// <summary>
  /// A managed wrapper for native vowpal wabbit exceptions.
  /// </summary>
  /// <remarks>
  /// As the default managed exception wrapping any native exception doesn't even capture exception::what()
  /// this wrapper was created.
  /// </remarks>
  [Serializable]
  public ref class VowpalWabbitException : Exception
  {
  private:
    /// <summary>
    /// The source filename in which the wrapped exception occurred.
    /// </summary>
    initonly String^ m_filename;

    /// <summary>
    /// The line number in which the wrapped exception occurred.
    /// </summary>
    initonly Int32 m_lineNumber;

  public:
    /// <summary>
    /// Initializes a new instance of <see cref="VowpalWabbitException"/>.
    /// </summary>
    /// <param name="ex">The native vowpal wabbit exception</param>
    VowpalWabbitException(const vw_exception& ex);

    /// <summary>
    /// Gets the source filename in which the wrapped exception occurred.
    /// </summary>
    property String^ Filename
    {
      String^ get();
    }

    /// <summary>
    /// Gets the line number in which the wrapped exception occurred.
    /// </summary>
    property Int32 LineNumber
    {
      Int32 get();
    }
  };
}

#define CATCHRETHROW \
catch (VW::vw_exception const& ex) \
{ throw gcnew VW::VowpalWabbitException(ex); } \
catch (std::exception const& ex) \
{ throw gcnew System::Exception(gcnew System::String(ex.what())); }
