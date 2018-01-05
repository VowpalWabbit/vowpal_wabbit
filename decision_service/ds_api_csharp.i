%module decision_service

#define ARRAYS_OPTIMIZED

%include "ds_api.i"
%include "arrays_csharp.i"

%rename("%(camelcase)s", notregexmatch$name=".*_template") "";

%define CSHARP_OUT_ARRAYS( CTYPE, CSTYPE )
		// fast copy of std::vector back into C#
	%typemap(cstype) std::vector<CTYPE>& "CSTYPE[]"
	%typemap(csout) std::vector<CTYPE>& {
    std::vector<CTYPE>& temp = $imcall;
    CSTYPE[] result = new CSTYPE[temp.size()];
        
    fixed(CSTYPE* pResult = result)
    {
        std::copy(temp.begin(), temp.end(), pResult);	
    }
        
    return result;
  }
%enddef // CSHARP_OUT_ARRAYS

CSHARP_OUT_ARRAYS(float, float)
CSHARP_OUT_ARRAYS(int, int)

%apply unsigned char FIXED[] {unsigned char* model}
%csmethodmodifiers Microsoft::DecisionService::DecisionServiceClient::update_model "public unsafe"; 

%typemap(cstype) const Microsoft::DecisionService::Array<int>& "int[]"
%typemap(csin,
	     pre=       "    fixed ( int* swig_ptrTo_$csinput = $csinput ) {\n" 
					"        Array<T> swig_arrayTo_$csinput = { swig_ptrTo_$csinput, $csinput.Length }; ",
         terminator="    }") 
		const Microsoft::DecisionService::Array<int>& "swig_arrayTo_$csinput" 

%csmethodmodifiers Microsoft::DecisionService::DecisionServiceClient::rank "public unsafe";

// required for fast vector copying
%csmethodmodifiers Microsoft::DecisionService::RankResponse::probabilities "public unsafe";
%csmethodmodifiers Microsoft::DecisionService::RankResponse::ranking "public unsafe";

%ignore Microsoft::DecisionService::Array<int>;
%ignore Microsoft::DecisionService::DecisionServiceClient::rank_cstyle;
%ignore Microsoft::DecisionService::DecisionServiceClient::rank_vector;
%rename(rank) Microsoft::DecisionService::DecisionServiceClient::rank_struct;

// must be at the end
%include "ds_api.h"
