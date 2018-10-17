#pragma once
#include "vw_settings.h"

namespace online_trainer {


	vw_settings::vw_settings(const char * args) : vw_settings()
	{
		if (args!= nullptr) {
			this->Arguments = args;
		}
	};
}

