#include <memory>

#include "vwdll.h"
#include "parser.h"

extern "C"
{
	
	VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_Initialize(const char * pstrArgs)
	{
		std::auto_ptr<vw> inst(new vw);
		try
		{
			string s(pstrArgs);
			*(inst.get()) = VW::initialize(s);
			initialize_parser_datastructures(*(inst.get()));
			return static_cast<VW_HANDLE>(inst.release());
		}
		catch (...)
		{
			// BUGBUG: should report error here....
			return INVALID_VW_HANDLE;
		}
	}
	
	VW_DLL_MEMBER void      VW_CALLING_CONV VW_Finish(VW_HANDLE handle)
	{
		try
		{
			vw * pointer = static_cast<vw*>(handle);
			if (pointer->numpasses > 1)
			{
				adjust_used_index(*pointer);
				pointer->pass_length = pointer->p->parsed_examples;
				start_parser(*pointer,false);
				pointer->driver((void*)pointer);
				end_parser(*pointer); 
			}
			else
				release_parser_datastructures(*pointer);
			VW::finish(*pointer);
			delete pointer;
		}
		catch (...)
		{}
	}

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ImportExample(VW_HANDLE handle, VW_FEATURE_SPACE * features, size_t len)
	{
		try
		{
			vw * pointer = static_cast<vw*>(handle);
			VW::primitive_feature_space * f = reinterpret_cast<VW::primitive_feature_space*>( features );
			return static_cast<VW_EXAMPLE>(VW::import_example(*pointer, f, len));
		}
		catch (...)
		{
			return INVALID_VW_EXAMPLE;
		}
	}
	
	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExample(VW_HANDLE handle, const char * line)
	{
		try
		{
			vw * pointer = static_cast<vw*>(handle);
			// BUGBUG: I really dislike this const_cast. should VW really change the input string?
			return static_cast<VW_EXAMPLE>(VW::read_example(*pointer, const_cast<char*>(line)));
		}
		catch (...)
		{
			// BUGBUG: should report error here....
			return INVALID_VW_EXAMPLE;
		}
	}
	
	VW_DLL_MEMBER void VW_CALLING_CONV VW_FinishExample(VW_HANDLE handle, VW_EXAMPLE e)
	{
		try
		{
			vw * pointer = static_cast<vw*>(handle);
			VW::finish_example(*pointer, static_cast<example*>(e));
		}
		catch (...)
		{
			// BUGBUG: should report error here....
		}
	}

	VW_DLL_MEMBER uint32_t VW_CALLING_CONV VW_HashSpace(VW_HANDLE handle, const char * s)
	{
		try
		{
			vw * pointer = static_cast<vw*>(handle);
			string str(s);
			return VW::hash_space(*pointer, str);
		}
		catch (...)
		{
			// BUGBUG: should report error here....
		}
	}

	VW_DLL_MEMBER uint32_t VW_CALLING_CONV VW_HashFeature(VW_HANDLE handle, const char * s, unsigned long u)
	{
		try
		{
			vw * pointer = static_cast<vw*>(handle);
			string str(s);
			return VW::hash_feature(*pointer, str, u);
		}
		catch (...)
		{
			// BUGBUG: should report error here....
		}
	}
	
	VW_DLL_MEMBER float VW_CALLING_CONV VW_Learn(VW_HANDLE handle, VW_EXAMPLE e)
	{
		try
		{
			vw * pointer = static_cast<vw*>(handle);
			example * ex = static_cast<example*>(e);
			pointer->learn(pointer, ex);
			return ex->final_prediction;
		}
		catch (...)
		{
			// BUGBUG: should report error here....
			return std::numeric_limits<float>::quiet_NaN();
		}
	}
	
}