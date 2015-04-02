/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
 */
#pragma once

// Library visibility macros:
// VW_DLL_MEMBER: Function/class should be exported or imported on WinXX, publicly visible for *nix platforms.
// VW_DLL_INTERNAL: Function/class is internal to the library, not visible or import-able. Here for completeness, not
//   currently used.
// VW_CALLING_CONV: Calling convention for WinXX (future: could use GNU calling conventions)

#if defined(_WIN32) || defined(_WIN64) || defined(_MSC_VER) || defined(__CYGWIN__) || defined(__MINGW32__) || defined(__MINGW64__)
#  if defined(VWDLL_EXPORTS)
#    define VW_DLL_MEMBER __declspec(dllexport)
#  else
#    define VW_DLL_MEMBER __declspec(dllimport)
#  endif
#  define VW_DLL_INTERNAL
#  define VW_CALLING_CONV __stdcall
// Enable wide character (32 bit) versions of functions
// These are optional since other compilers may not have wide to narrow char libarray facilities built in.
#  define USE_CODECVT 
#else
#  if __GNUC__ >= 4
#    define VW_DLL_MEMBER __attribute__ ((visibility ("default")))
#    define VW_DLL_INTERNAL  __attribute__ ((visibility ("hidden")))
#  else
#    define VW_DLL_MEMBER
#    define VW_DLL_INTERNAL
#  endif
#  define VW_CALLING_CONV
#endif

#if defined(USE_CODECVT)
#if defined(_WIN32) || defined(_WIN64) || defined(MSC_VER)
#include <codecvt>
#else
// Other platforms that automagically convert from wide-to-narrow should
// insert code here.
#endif
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#ifdef __cplusplus
#define VW_TYPE_SAFE_NULL nullptr
#else
#define VW_TYPE_SAFE_NULL NULL
#endif

	typedef void * VW_HANDLE;
	typedef void * VW_EXAMPLE;
	typedef void * VW_LABEL;
	typedef void * VW_FEATURE_SPACE;
	typedef void * VW_FEATURE;
 
	const VW_HANDLE INVALID_VW_HANDLE = VW_TYPE_SAFE_NULL;
	const VW_HANDLE INVALID_VW_EXAMPLE = VW_TYPE_SAFE_NULL;

#ifdef USE_CODECVT
	VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_Initialize(const char16_t * pstrArgs);
#endif
	VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_InitializeA(const char * pstrArgs);

	VW_DLL_MEMBER void VW_CALLING_CONV VW_Finish_Passes(VW_HANDLE handle);
	VW_DLL_MEMBER void VW_CALLING_CONV VW_Finish(VW_HANDLE handle);

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ImportExample(VW_HANDLE handle, VW_FEATURE_SPACE * features, size_t len);

	VW_DLL_MEMBER VW_FEATURE_SPACE VW_CALLING_CONV VW_ExportExample(VW_HANDLE handle, VW_EXAMPLE e, size_t* plen);
	VW_DLL_MEMBER void VW_CALLING_CONV VW_ReleaseFeatureSpace(VW_FEATURE_SPACE * features, size_t len);
#ifdef USE_CODECVT
	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExample(VW_HANDLE handle, const char16_t * line);
#endif
	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExampleA(VW_HANDLE handle, const char * line);

	VW_DLL_MEMBER void VW_CALLING_CONV VW_StartParser(VW_HANDLE handle, bool do_init);
	VW_DLL_MEMBER void VW_CALLING_CONV VW_EndParser(VW_HANDLE handle);

	VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_GetExample(VW_HANDLE handle);
	VW_DLL_MEMBER void VW_CALLING_CONV VW_FinishExample(VW_HANDLE handle, VW_EXAMPLE e);
	VW_DLL_MEMBER float VW_CALLING_CONV VW_GetLabel(VW_EXAMPLE e);
	VW_DLL_MEMBER float VW_CALLING_CONV VW_GetImportance(VW_EXAMPLE e);
	VW_DLL_MEMBER float VW_CALLING_CONV VW_GetInitial(VW_EXAMPLE e);
	VW_DLL_MEMBER float VW_CALLING_CONV VW_GetPrediction(VW_EXAMPLE e);
	VW_DLL_MEMBER float VW_CALLING_CONV VW_GetCostSensitivePrediction(VW_EXAMPLE e);
	VW_DLL_MEMBER float VW_CALLING_CONV VW_GetTopicPrediction(VW_EXAMPLE e, size_t i);
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_GetTagLength(VW_EXAMPLE e);
	VW_DLL_MEMBER const char* VW_CALLING_CONV VW_GetTag(VW_EXAMPLE e);
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_GetFeatureNumber(VW_EXAMPLE e);
	VW_DLL_MEMBER VW_FEATURE VW_CALLING_CONV VW_GetFeatures(VW_HANDLE handle, VW_EXAMPLE e, size_t* plen);
	VW_DLL_MEMBER void VW_CALLING_CONV VW_ReturnFeatures(VW_FEATURE f);
#ifdef USE_CODECVT
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashSpace(VW_HANDLE handle, const char16_t * s);
#endif
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashSpaceA(VW_HANDLE handle, const char * s);
#ifdef USE_CODECVT
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashFeature(VW_HANDLE handle, const char16_t * s, unsigned long u);
#endif
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashFeatureA(VW_HANDLE handle, const char * s, unsigned long u);

	VW_DLL_MEMBER float VW_CALLING_CONV VW_Learn(VW_HANDLE handle, VW_EXAMPLE e);
	VW_DLL_MEMBER float VW_CALLING_CONV VW_Predict(VW_HANDLE handle, VW_EXAMPLE e);
	VW_DLL_MEMBER void VW_CALLING_CONV VW_AddLabel(VW_EXAMPLE e, float label, float weight, float base);

	VW_DLL_MEMBER float VW_CALLING_CONV VW_Get_Weight(VW_HANDLE handle, size_t index, size_t offset);
    VW_DLL_MEMBER void VW_CALLING_CONV VW_Set_Weight(VW_HANDLE handle, size_t index, size_t offset, float value);
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_Num_Weights(VW_HANDLE handle);
	VW_DLL_MEMBER size_t VW_CALLING_CONV VW_Get_Stride(VW_HANDLE handle);

#ifdef __cplusplus
}
#endif

#undef VW_TYPE_SAFE_NULL
