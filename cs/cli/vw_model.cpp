// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw_clr.h"
#include "vw_model.h"
#include "parse_regressor.h"
#include "parse_args.h"
#include "clr_io.h"

namespace VW
{
VowpalWabbitSettings^ AddTestOnly(VowpalWabbitSettings^ settings)
{ // VowpalWabbitModel and VowpalWabbit instances seeded from VowpalWabbitModel
  // need to have the same "test" setting, otherwise the stride shift is different
  // and all hell breaks loose.
  if (!settings->Arguments->Contains("-t ") &&
      !settings->Arguments->Contains("--testonly ") &&
      !settings->Arguments->EndsWith("-t") &&
      !settings->Arguments->EndsWith("--testonly"))
  { settings->Arguments += " -t";
  }

  return settings;
}

VowpalWabbitModel::VowpalWabbitModel(VowpalWabbitSettings^ settings)
  : VowpalWabbitBase(AddTestOnly(settings))
{ if (settings == nullptr)
    throw gcnew ArgumentNullException("settings");

  if (settings->Model != nullptr)
    throw gcnew ArgumentNullException("VowpalWabbitModel cannot be initialized from another model");
}

VowpalWabbitModel::VowpalWabbitModel(String^ args)
  : VowpalWabbitModel(gcnew VowpalWabbitSettings(args))
{
}
}
