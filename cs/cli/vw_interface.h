// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw.h"
#include "vw_clr.h"
#include "vw_prediction.h"

namespace VW
{
ref class VowpalWabbitExample;
ref class VowpalWabbitBase;

/// <summary>
/// Owners of example must implement this interface.
/// </summary>
public interface class IVowpalWabbitExamplePool : public System::IDisposable
{
  /// <summary>
  /// Gets or creates a new native examples.
  /// </summary>
  VowpalWabbitExample^ GetOrCreateNativeExample();

  property VowpalWabbit^ Native
  { VowpalWabbit^ get();
  }

  /// <summary>
  /// Puts a native example data structure back into the pool.
  /// </summary>
  /// <param name="example">The example to be returned.</param>
  void ReturnExampleToPool(VowpalWabbitExample^ example);
};
}
