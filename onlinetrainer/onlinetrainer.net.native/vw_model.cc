#pragma once
#include "vw_model.h"

namespace online_trainer {
	vw_model::vw_model(vw_settings * settings)
	{
		if (settings == nullptr)
			throw "settings is null";
		if (settings->Model != nullptr)
			throw "VowpalWabbitModel cannot be initialized from another model";
	};
	vw_model::vw_model(const char * args) : vw_model(new vw_settings(args))
	{
	};
}

