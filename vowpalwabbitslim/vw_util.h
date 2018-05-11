#pragma once

#include <string>
#include "example_predict.h" 
#include "vw_slim_predict.h"
#include <fstream>

template<typename W>
int load_model_from_file(vw_slim::vw_predict<W>& vw, const char* filename)
{
	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);

	std::ifstream::pos_type model_size = ifs.tellg();

	std::vector<char> model_file(model_size);

	ifs.seekg(0, std::ios::beg);
	ifs.read(&model_file[0], model_size);

	return vw.load(&model_file[0], model_size);
}

#ifdef BOOST_FOUND

bool try_parse_examples(std::string line, safe_example_predict& ex);

#endif
