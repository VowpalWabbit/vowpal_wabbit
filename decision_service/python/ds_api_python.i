%module(threads="1") decision_service
%nothread;

// make sure this function can block on a thread and python threads can continue running
%thread DecisionServiceConfiguration::Download;

#define ARRAYS_OPTIMIZED
#define DISABLE_NAMESPACE

%include "../ds_api.i"
#include "exception.i"

%include "std_vector.i"

// try how the vector binding looks like... maybe it's ok
// add update_model w/o offset to support bybuffer

namespace std
{
	%template(IntVector) vector<int>;
	%template(FloatVector) vector<float>;
}

%ignore Array<int>;
%ignore DecisionServiceClient::rank_cstyle;
%ignore DecisionServiceClient::rank_struct;
%rename(rank) DecisionServiceClient::rank_vector;


%include "ds_api.h"

