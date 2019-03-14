#pragma once

#include "vw_slim_predict.h"

namespace vw_slim {

	class example_predict_builder
	{
		example_predict* _ex;
		namespace_index _namespace_idx;
		uint64_t _namespace_hash;

		void add_namespace(namespace_index feature_group);

	public:
		example_predict_builder(example_predict* ex, const char* namespace_name);
		example_predict_builder(example_predict* ex, namespace_index namespace_idx);

		void push_feature_string(const char* feature_idx, feature_value value);
		void push_feature(feature_index feature_idx, feature_value value);
	};
} //end-of-namespace
