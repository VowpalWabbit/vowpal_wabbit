
%module DS
%{

 /* Includes the header in the wrapper code */
 #include "ds_api.h"
%}

%include "std_vector.i"

namespace std {
   #ifdef SWIGCSHARP
       %template(ListFloat) vector<float>;
       %template(ListInt) vector<int>;
   #else
       %template(vector_int) vector<int>;
       %template(vector_float) vector<float>;
   #endif
}

%include "std_string.i"

#ifdef SWIGCSHARP
        %rename("%(camelcase)s", notregexmatch$name=".*_template") "";


        %include "arrays_csharp.i"
        /* %apply float FIXED[] {float *x_array} */

        %apply unsigned char FIXED[] {unsigned char* model}
        %csmethodmodifiers ds::DecisionServiceClient::update_model "public unsafe"; 

        %apply int FIXED[] {int* default_ranking}
        %csmethodmodifiers ds::DecisionServiceClient::rank "public unsafe"; 
#endif

#ifdef SWIGJAVA
	%rename("%(camelcase)s", notregexmatch$name="examples_writer_template|vector") "";

	%rename("%(lowercamelcase)s", %$isfunction, %$ismember) "";
	// TODO: rename methods seperately
#endif
/*
 %apply(char *STRING, size_t LENGTH) { (char *str, size_t len) };
*/
 %include "ds_api.h"
