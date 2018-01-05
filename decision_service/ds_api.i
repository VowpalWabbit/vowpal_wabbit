// include in header
%{
 #include "ds_api.h"

 using namespace Microsoft::DecisionService;
%}

%nspace *::RankResponse;
%nspace *::DecisionServiceClient;
%nspace *::DecisionServiceConfiguration;

%include <std_string.i>
#include <exception.i>
%include <std_vector.i>

// %include <std_shared_ptr.i>
// %shared_ptr(Microsoft::DecisionService::DecisionServiceListener)
// %shared_ptr(DecisionServiceListener)
// %template(SmartPtrFoo) SmartPtr<Foo>;

// disable default constructor
%nodefaultctor *::RankResponse;
%nodefaultctor RankResponse;

%feature("director") *::DecisionServiceListener;
%feature("director") DecisionServiceListener;

#ifndef ARRAYS_OPTIMIZED
	%ignore Microsoft::DecisionService::RankResponse::probabilities();
	%ignore Microsoft::DecisionService::RankResponse::ranking();

	%ignore Microsoft::DecisionService::Array<int>;
	%ignore Microsoft::DecisionService::Array<int>;
	%ignore Microsoft::DecisionService::DecisionServiceClient::rank_struct;
	%ignore Microsoft::DecisionService::DecisionServiceClient::rank_vector;

	%ignore Microsoft::DecisionService::DecisionServiceClient::rank(const char* features, const char* event_id, const Microsoft::DecisionService::Array<int>& default_ranking);
	%ignore Microsoft::DecisionService::DecisionServiceClient::rank(const char* features, const char* event_id, const std::vector<int>& default_ranking);
#endif
