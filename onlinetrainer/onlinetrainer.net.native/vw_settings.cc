#pragma once
#include "vw_settings.h"

namespace online_trainer {
	vw_settings::vw_settings()
	{
		ExampleCountPerRun = 1000;
		MaxExampleCacheSize = UINT32_MAX;
		MaxExampleQueueLengthPerInstance = UINT32_MAX;
		EnableExampleCaching = false;
		ExampleDistribution = vw_example_distribution::UniformRandom;
		EnableStringExampleGeneration = false;
		EnableStringFloatCompact = false;
		//  PropertyConfiguration = ::PropertyConfiguration::Default;
		EnableThreadSafeExamplePooling = false;
		MaxExamples = INT32_MAX;
		Verbose = false;
	};

	vw_settings::vw_settings(const char * args) : vw_settings()
	{
		if (args!= nullptr) {
			this->Arguments = args;
		}
	};
}

