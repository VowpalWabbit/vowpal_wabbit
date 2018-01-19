// include in header
%{
 #include "ds_api.h"

 using namespace Microsoft::DecisionService;
%}

%nspace *::RankResponse;
%nspace *::DecisionServiceClient;
%nspace *::DecisionServiceConfiguration;
%nspace *::DecisionServiceLogger;
%nspace *::DecisionServicePrediction;
%nspace *::DecisionServicePredictors;

%include <std_string.i>
#include <exception.i>
%include <std_vector.i>

// didn't work out
// %include <std_shared_ptr.i>
// %shared_ptr(Microsoft::DecisionService::DecisionServiceLogger)
// %shared_ptr(DecisionServiceLogger)
// %template(SmartPtrFoo) SmartPtr<Foo>;

// disable default constructor
%nodefaultctor *::RankResponse;
%nodefaultctor RankResponse;

%feature("director") *::DecisionServiceLogger;
%feature("director") DecisionServiceLogger;

%feature("director") *::DecisionServicePredictors;
%feature("director") DecisionServicePredictors;

#ifndef ARRAYS_OPTIMIZED
	%ignore Microsoft::DecisionService::RankResponse::probabilities();
	%ignore Microsoft::DecisionService::RankResponse::ranking();

	%ignore Microsoft::DecisionService::Array<int>;
	%ignore Microsoft::DecisionService::Array<int>;
	// %ignore Microsoft::DecisionService::DecisionServiceClient::rank_struct;
	// %ignore Microsoft::DecisionService::DecisionServiceClient::rank_vector;

	// %ignore Microsoft::DecisionService::DecisionServiceClient::rank(const char* features, const char* event_id, const Microsoft::DecisionService::Array<int>& default_ranking);
	// %ignore Microsoft::DecisionService::DecisionServiceClient::rank(const char* features, const char* event_id, const std::vector<int>& default_ranking);
#endif
