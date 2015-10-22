/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vw_clr.h"
#include "vw_model.h"
#include "parse_regressor.h"
#include "parse_args.h"
#include "clr_io.h"

namespace VW
{
	VowpalWabbitModel::VowpalWabbitModel(VowpalWabbitSettings^ settings)
		: VowpalWabbitBase(settings)
	{
		if (settings == nullptr)
			throw gcnew ArgumentNullException("settings");

		if (settings->Model != nullptr)
			throw gcnew ArgumentNullException("VowpalWabbitModel cannot be initialized from another model");
	}

	VowpalWabbitModel::VowpalWabbitModel(String^ args)
		: VowpalWabbitModel(gcnew VowpalWabbitSettings(args))
	{
	}
}