#include <memory>
#include <codecvt>
#include <locale>
#include <string>

#include "vwdll.h"
#include "parser.h"

// This interface now provides "wide" functions for compatibility with .NET interop
// The default functions assume a wide (16 bit char pointer) that is converted to a utf8-string and passed to
// a function which takes a narrow (8 bit char pointer) function. Both are exposed in the c/c++ API 
// so that programs using 8 bit wide characters can use the direct call without conversion and 
//  programs using 16 bit characters can use the default wide versions of the functions.
// "Ansi versions  (FcnA instead of Fcn) have only been written for functions which handle strings.

// a future optimization would be to write an inner version of hash feature which either hashed the
// wide string directly (and live with the different hash values) or incorporate the UTF-16 to UTF-8 conversion
// in the hashing to avoid allocating an intermediate string.
 
extern "C"
{

	VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_Initialize(const char16_t * pstrArgs)
	{
		std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> convert;
		std::string sa(convert.to_bytes(pstrArgs));
		return VW_InitializeA(sa.c_str());
	}

	
	VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_InitializeA(const char * pstrArgs)
	{
		std::auto_ptr<vw> inst(new vw);
		string s(pstrArgs);
		*(inst.get()) = VW::initialize(s);
		initialize_parser_datastructures(*(inst.get()));
		return static_cast<VW_HANDLE>(inst.release());
	}
	
	VW_DLL_MEMBER void      VW_CALLING_CONV VW_Finish(VW_HANDLE handle)
	{
		vw * pointer = static_cast<vw*>(handle);
		if (pointer->numpasses > 1)
			{
			adjust_used_index(*pointer);
			pointer->do_reset_source = true;
			start_parser(*pointer,false);
			pointer->l.driver((void*)pointer, pointer->l.data);
			end_parser(*pointer); 
			}
		else
			release_parser_datastructures(*pointer);
		VW::finish(*pointer);
		delete pointer;
	}

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ImportExample(VW_HANDLE handle, VW_FEATURE_SPACE * features, size_t len)
	{
		vw * pointer = static_cast<vw*>(handle);
		VW::primitive_feature_space * f = reinterpret_cast<VW::primitive_feature_space*>( features );
		return static_cast<VW_EXAMPLE>(VW::import_example(*pointer, f, len));
	}
	

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExample(VW_HANDLE handle, const char16_t * line)
	{
		std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> convert;
		std::string sa(convert.to_bytes(line));
		return VW_ReadExampleA(handle, sa.c_str());
	}



	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExampleA(VW_HANDLE handle, const char * line)
	{
		vw * pointer = static_cast<vw*>(handle);
		// BUGBUG: I really dislike this const_cast. should VW really change the input string?
		return static_cast<VW_EXAMPLE>(VW::read_example(*pointer, const_cast<char*>(line)));
	}
	
	VW_DLL_MEMBER void VW_CALLING_CONV VW_FinishExample(VW_HANDLE handle, VW_EXAMPLE e)
	{
		vw * pointer = static_cast<vw*>(handle);
		VW::finish_example(*pointer, static_cast<example*>(e));
	}

	VW_DLL_MEMBER uint32_t VW_CALLING_CONV VW_HashSpace(VW_HANDLE handle, const char16_t * s)
	{
		std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> convert;
		std::string sa(convert.to_bytes(s));
		return VW_HashSpaceA(handle,sa.c_str());
	}

	VW_DLL_MEMBER uint32_t VW_CALLING_CONV VW_HashSpaceA(VW_HANDLE handle, const char * s)
	{
		vw * pointer = static_cast<vw*>(handle);
		string str(s);
		return VW::hash_space(*pointer, str);
	}


	VW_DLL_MEMBER uint32_t VW_CALLING_CONV VW_HashFeature(VW_HANDLE handle, const char16_t * s, unsigned long u)
	{
		std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> convert;
		std::string sa(convert.to_bytes(s));
		return VW_HashFeatureA(handle,sa.c_str(),u);
	}


	VW_DLL_MEMBER uint32_t VW_CALLING_CONV VW_HashFeatureA(VW_HANDLE handle, const char * s, unsigned long u)
	{
		vw * pointer = static_cast<vw*>(handle);
		string str(s);
		return VW::hash_feature(*pointer, str, u);
	}
	
	VW_DLL_MEMBER float VW_CALLING_CONV VW_Learn(VW_HANDLE handle, VW_EXAMPLE e)
	{
		vw * pointer = static_cast<vw*>(handle);
		example * ex = static_cast<example*>(e);
		pointer->learn(pointer, ex);
		return ex->final_prediction;
	}
}