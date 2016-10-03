/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/

#pragma once

#include "vw.h"
#include "v_array.h"

namespace VW
{
	// can't type as it forces C++/CLI part to include rapidjson, which leads to name clashes...
	typedef example* (*example_factory_t)(void*);

	void read_line_json(vw& all, v_array<example*>& examples, char* line, example_factory_t example_factory, void* ex_factory_context);
}