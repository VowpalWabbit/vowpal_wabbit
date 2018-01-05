// include in header
%{
 #include "ds_api.h"

 using namespace Microsoft::DecisionService;
%}

%nspace *::RankResponse;
%nspace *::DecisionServiceClient;
%nspace *::DecisionServiceConfiguration;

%include "std_string.i"

// disable default constructor
%nodefaultctor *::RankResponse;
%nodefaultctor RankResponse;

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
