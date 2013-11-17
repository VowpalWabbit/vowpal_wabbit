/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#ifndef VWDLL_H
#define VWDLL_H

#define VW_CALLING_CONV __stdcall

#ifdef VWDLL_EXPORTS
#define VW_DLL_MEMBER __declspec(dllexport)
#else
#define VW_DLL_MEMBER __declspec(dllimport)
#endif

#ifdef __cplusplus
extern "C"
{
#endif

	typedef void * VW_HANDLE;
	typedef void * VW_EXAMPLE;
	typedef void * VW_LABEL;
	typedef void * VW_FEATURE_SPACE;
	typedef void * VW_FLAT_EXAMPLE;
 
	const VW_HANDLE INVALID_VW_HANDLE = NULL;
	const VW_HANDLE INVALID_VW_EXAMPLE = NULL;

	VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_Initialize(const char16_t * pstrArgs);
	VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_InitializeA(const char * pstrArgs);

	VW_DLL_MEMBER void VW_CALLING_CONV VW_Finish(VW_HANDLE handle);

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ImportExample(VW_HANDLE handle, VW_FEATURE_SPACE * features, size_t len);

	VW_DLL_MEMBER VW_FEATURE_SPACE VW_CALLING_CONV VW_ExportExample(VW_HANDLE handle, VW_EXAMPLE e, size_t* plen);
	VW_DLL_MEMBER void VW_CALLING_CONV VW_ReleaseFeatureSpace(VW_FEATURE_SPACE * features, size_t len);

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExample(VW_HANDLE handle, const char16_t * line);
	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExampleA(VW_HANDLE handle, const char * line);

	VW_DLL_MEMBER void VW_CALLING_CONV VW_StartParser(VW_HANDLE handle, bool do_init);
	VW_DLL_MEMBER void VW_CALLING_CONV VW_EndParser(VW_HANDLE handle);

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_GetExample(VW_HANDLE handle);
	VW_DLL_MEMBER VW_LABEL VW_CALLING_CONV VW_GetLabelExample(VW_HANDLE handle, VW_EXAMPLE e);
	VW_DLL_MEMBER void VW_CALLING_CONV VW_FinishExample(VW_HANDLE handle, VW_EXAMPLE e);

	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashSpace(VW_HANDLE handle, const char16_t * s);
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashSpaceA(VW_HANDLE handle, const char * s);

	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashFeature(VW_HANDLE handle, const char16_t * s, unsigned long u);
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashFeatureA(VW_HANDLE handle, const char * s, unsigned long u);

	VW_DLL_MEMBER float VW_CALLING_CONV VW_Learn(VW_HANDLE handle, VW_EXAMPLE e);
	VW_DLL_MEMBER void VW_CALLING_CONV VW_AddLabel(VW_EXAMPLE e, float label, float weight, float base);

	VW_DLL_MEMBER float VW_CALLING_CONV VW_Get_Weight(VW_HANDLE handle, size_t index, size_t offset);
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_Num_Weights(VW_HANDLE handle);
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_Get_Stride(VW_HANDLE handle);

	VW_DLL_MEMBER VW_FLAT_EXAMPLE VW_CALLING_CONV VW_FlattenExample(VW_HANDLE handle, VW_EXAMPLE e);
	VW_DLL_MEMBER void VW_CALLING_CONV VW_FreeFlattenExample(VW_FLAT_EXAMPLE fec);

#ifdef __cplusplus	
}
#endif

#endif /* VWDLL_H  */
