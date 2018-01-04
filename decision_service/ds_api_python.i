#define ARRAYS_OPTIMIZED

%include "ds_api.i"
// TODO: could be optimized a bit more as vector for a plain array seems a bit of an overkills
%include "std_vector.i"

// model (byte[] should probably be pybuffer)
// must be at the end
%typemap(in) (int* default_ranking, int default_ranking_size) {
	
	$2 = PyInt_AsLong($input);
}

// try how the vector binding looks like... maybe it's ok
// add update_model w/o offset to support bybuffer

		Microsoft::DecisionService::Array<int>& "swig_arrayTo_$csinput" 

%include "ds_api.h"
