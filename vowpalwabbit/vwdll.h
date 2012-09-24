/*
Copyright (c) 2012 Microsoft.  All rights reserved.  The copyrights
embodied in the content of this file are licensed under the BSD
(revised) open source license
 */


#ifndef VWDLL_H
#define VWDLL_H

#include "vw.h"

#define VW_CALLING_CONV __stdcall

#ifdef VWDLL_EXPORTS
#define VW_DLL_MEMBER __declspec(dllexport)
#else
#define VW_DLL_MEMBER __declspec(dllimport)
#endif

extern "C"
{
	typedef void * VW_HANDLE;
	typedef void * VW_EXAMPLE;

	const VW_HANDLE INVALID_VW_HANDLE = NULL;
	const VW_HANDLE INVALID_VW_EXAMPLE = NULL;

	VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_Initialize(const char * pstrArgs);
	VW_DLL_MEMBER void      VW_CALLING_CONV VW_Finish(VW_HANDLE handle);

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExample(VW_HANDLE handle, const char * line);
	VW_DLL_MEMBER void       VW_CALLING_CONV VW_FinishExample(VW_HANDLE handle, VW_EXAMPLE e);

	VW_DLL_MEMBER float VW_CALLING_CONV VW_Learn(VW_HANDLE handle, VW_EXAMPLE e);
}

#endif /* VWDLL_H  */