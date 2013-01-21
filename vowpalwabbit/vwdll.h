/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
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
	typedef void * VW_FEATURE_SPACE;
 
	const VW_HANDLE INVALID_VW_HANDLE = NULL;
	const VW_HANDLE INVALID_VW_EXAMPLE = NULL;

	VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_Initialize(const char16_t * pstrArgs);
	VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_InitializeA(const char * pstrArgs);

	VW_DLL_MEMBER void      VW_CALLING_CONV VW_Finish(VW_HANDLE handle);

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ImportExample(VW_HANDLE handle, VW_FEATURE_SPACE * features, size_t len);

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExample(VW_HANDLE handle, const char16_t * line);
	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExampleA(VW_HANDLE handle, const char * line);

	VW_DLL_MEMBER void       VW_CALLING_CONV VW_FinishExample(VW_HANDLE handle, VW_EXAMPLE e);

	VW_DLL_MEMBER uint32_t VW_CALLING_CONV VW_HashSpace(VW_HANDLE handle, const char16_t * s);
	VW_DLL_MEMBER uint32_t VW_CALLING_CONV VW_HashSpaceA(VW_HANDLE handle, const char * s);

	VW_DLL_MEMBER uint32_t VW_CALLING_CONV VW_HashFeature(VW_HANDLE handle, const char16_t * s, unsigned long u);
	VW_DLL_MEMBER uint32_t VW_CALLING_CONV VW_HashFeatureA(VW_HANDLE handle, const char * s, unsigned long u);

	VW_DLL_MEMBER float VW_CALLING_CONV VW_Learn(VW_HANDLE handle, VW_EXAMPLE e);
}

#endif /* VWDLL_H  */
