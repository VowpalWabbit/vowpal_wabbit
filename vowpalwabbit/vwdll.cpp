#include <memory>
#include <codecvt>
#include <locale>
#include <string>

#include "vwdll.h"
#include "parser.h"
#include "parse_args.h"
#include "vw.h"

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
		string s(pstrArgs);
		vw* all = VW::initialize(s);
		return static_cast<VW_HANDLE>(all);
	}
	
	VW_DLL_MEMBER void      VW_CALLING_CONV VW_Finish(VW_HANDLE handle)
	{
		vw * pointer = static_cast<vw*>(handle);
		if (pointer->numpasses > 1)
			{
			adjust_used_index(*pointer);
			pointer->do_reset_source = true;
			VW::start_parser(*pointer,false);
			pointer->l->driver(pointer);
			VW::end_parser(*pointer); 
			}
		else
			release_parser_datastructures(*pointer);

		VW::finish(*pointer);
	}

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ImportExample(VW_HANDLE handle, VW_FEATURE_SPACE* features, size_t len)
	{
		vw * pointer = static_cast<vw*>(handle);
		VW::primitive_feature_space * f = reinterpret_cast<VW::primitive_feature_space*>( features );
		return static_cast<VW_EXAMPLE>(VW::import_example(*pointer, f, len));
	}
	
	VW_DLL_MEMBER VW_FEATURE_SPACE VW_CALLING_CONV VW_ExportExample(VW_HANDLE handle, VW_EXAMPLE e, size_t * plen)
	{
		vw* pointer = static_cast<vw*>(handle);
		example* ex = static_cast<example*>(e);
		return static_cast<VW_FEATURE_SPACE>(VW::export_example(*pointer, ex, *plen));
	}

	VW_DLL_MEMBER void VW_CALLING_CONV VW_ReleaseFeatureSpace(VW_FEATURE_SPACE* features, size_t len)
	{
		VW::primitive_feature_space * f = reinterpret_cast<VW::primitive_feature_space*>( features );
		VW::releaseFeatureSpace(f, len);
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
	
	VW_DLL_MEMBER void VW_CALLING_CONV VW_StartParser(VW_HANDLE handle, bool do_init)
	{
		vw * pointer = static_cast<vw*>(handle);
		VW::start_parser(*pointer, do_init);
	}

	VW_DLL_MEMBER void VW_CALLING_CONV VW_EndParser(VW_HANDLE handle)
	{
		vw * pointer = static_cast<vw*>(handle);
		VW::end_parser(*pointer);
	}

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_GetExample(VW_HANDLE handle)
	{
		vw * pointer = static_cast<vw*>(handle);
		parser * parser_pointer = static_cast<parser *>(pointer->p);
		return static_cast<VW_EXAMPLE>(VW::get_example(parser_pointer));
	}

	VW_DLL_MEMBER VW_LABEL VW_CALLING_CONV VW_GetLabel(VW_HANDLE handle, VW_EXAMPLE e)
	{
		vw * pointer = static_cast<vw*>(handle);
		label_data* ld = VW::get_label(static_cast<example*>(e));
		return static_cast<VW_LABEL>(ld);
	}

	VW_DLL_MEMBER void VW_CALLING_CONV VW_FinishExample(VW_HANDLE handle, VW_EXAMPLE e)
	{
		vw * pointer = static_cast<vw*>(handle);
		VW::finish_example(*pointer, static_cast<example*>(e));
	}

	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashSpace(VW_HANDLE handle, const char16_t * s)
	{
		std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> convert;
		std::string sa(convert.to_bytes(s));
		return VW_HashSpaceA(handle,sa.c_str());
	}

	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashSpaceA(VW_HANDLE handle, const char * s)
	{
		vw * pointer = static_cast<vw*>(handle);
		string str(s);
		return VW::hash_space(*pointer, str);
	}


	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashFeature(VW_HANDLE handle, const char16_t * s, unsigned long u)
	{
		std::wstring_convert<std::codecvt_utf8<char16_t>, char16_t> convert;
		std::string sa(convert.to_bytes(s));
		return VW_HashFeatureA(handle,sa.c_str(),u);
	}


	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashFeatureA(VW_HANDLE handle, const char * s, unsigned long u)
	{
		vw * pointer = static_cast<vw*>(handle);
		string str(s);
		return VW::hash_feature(*pointer, str, u);
	}
	
	VW_DLL_MEMBER void  VW_CALLING_CONV VW_AddLabel(VW_EXAMPLE e, float label, float weight, float base)
	{
		example* ex = static_cast<example*>(e);
		return VW::add_label(ex, label, weight, base);
	}

	VW_DLL_MEMBER float VW_CALLING_CONV VW_Learn(VW_HANDLE handle, VW_EXAMPLE e)
	{
		vw * pointer = static_cast<vw*>(handle);
		example * ex = static_cast<example*>(e);
		pointer->learn(ex);
		return ex->final_prediction;
	}

	VW_DLL_MEMBER float VW_CALLING_CONV VW_Get_Weight(VW_HANDLE handle, size_t index, size_t offset)
	{
		vw* pointer = static_cast<vw*>(handle);
		return VW::get_weight(*pointer, (uint32_t) index, (uint32_t) offset);
	}

	VW_DLL_MEMBER void VW_CALLING_CONV VW_Set_Weight(VW_HANDLE handle, size_t index, size_t offset, float value)
	{
		vw* pointer = static_cast<vw*>(handle);
		return VW::set_weight(*pointer, (uint32_t) index, (uint32_t)offset, value);
	}

	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_Num_Weights(VW_HANDLE handle)
	{
		vw* pointer = static_cast<vw*>(handle);
		return VW::num_weights(*pointer);
	}

	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_Get_Stride(VW_HANDLE handle)
	{
		vw* pointer = static_cast<vw*>(handle);
		return VW::get_stride(*pointer);
	}

	VW_DLL_MEMBER VW_FLAT_EXAMPLE VW_CALLING_CONV VW_FlattenExample(VW_HANDLE handle, VW_EXAMPLE e)
	{
		vw* pointer = static_cast<vw*>(handle);
		example* ex = static_cast<example*>(e);
		return static_cast<VW_FLAT_EXAMPLE>(VW::flatten_example(*pointer, ex));
	}

	VW_DLL_MEMBER void VW_CALLING_CONV VW_FreeFlattenExample(VW_FLAT_EXAMPLE fec)
	{
		VW::flat_example *p = reinterpret_cast<VW::flat_example*>(fec);
		VW::free_flatten_example(p);
	}
}
