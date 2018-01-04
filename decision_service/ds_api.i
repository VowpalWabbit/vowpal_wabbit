%module DS

// include in header
%{
 #include "ds_api.h"
%}

%nspace Microsoft::DecisionService::RankResponse;
%nspace Microsoft::DecisionService::DecisionServiceClient;
%nspace Microsoft::DecisionService::DecisionServiceConfiguration;

%include "std_string.i"

#ifndef ARRAYS_OPTIMIZED
	%ignore Microsoft::DecisionService::RankResponse::probabilities();
	%ignore Microsoft::DecisionService::RankResponse::ranking();

	%ignore Microsoft::DecisionService::Array<int>;
	%ignore Microsoft::DecisionService::DecisionServiceClient::rank(const char* features, const char* event_id, Microsoft::DecisionService::Array<int> default_ranking);
#endif
