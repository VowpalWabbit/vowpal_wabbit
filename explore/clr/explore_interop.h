#pragma once

#define MANAGED_CODE

#include "MWTExplorer.h"
#include "MWTRewardReporter.h"
#include "MWTOptimizer.h"

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Runtime::InteropServices;
using namespace System::Xml::Serialization;
using namespace msclr::interop;

namespace MultiWorldTesting {

// Policy callback
private delegate UInt32 ClrPolicyCallback(IntPtr explorerPtr, IntPtr contextPtr);
typedef u32 Native_Policy_Callback(void* explorer, void* context);

// Recorder callback
private delegate void ClrRecorderCallback(IntPtr mwtPtr, IntPtr contextPtr, UInt32 action, float probability, IntPtr uniqueKey);
typedef void Native_Recorder_Callback(void* mwt, void* context, u32 action, float probability, void* unique_key);

// NativeContext travels through interop space and contains instances of Mwt, Explorer, Context
// used for triggering callback for Policy, Scorer, Recorder
class NativeContext
{
public:
	NativeContext(void* clr_mwt, void* clr_explorer, void* clr_context)
	{
		m_clr_mwt = clr_mwt;
		m_clr_explorer = clr_explorer;
		m_clr_context = clr_context;
	}

	void* Get_Clr_Mwt()
	{
		return m_clr_mwt;
	}

	void* Get_Clr_Context()
	{
		return m_clr_context;
	}

	void* Get_Clr_Explorer()
	{
		return m_clr_explorer;
	}

private:
	void* m_clr_mwt;
	void* m_clr_context;
	void* m_clr_explorer;
};

// NativeRecorder listens to callback event and reroute it to the managed Recorder instance
class NativeRecorder : public NativeMultiWorldTesting::IRecorder<NativeContext>
{
public:
	NativeRecorder(Native_Recorder_Callback* native_func)
	{
		m_func = native_func;
	}

	void Record(NativeContext& context, u32 action, float probability, string unique_key)
	{
		GCHandle uniqueKeyHandle = GCHandle::Alloc(gcnew String(unique_key.c_str()));
		IntPtr uniqueKeyPtr = (IntPtr)uniqueKeyHandle;

		m_func(context.Get_Clr_Mwt(), context.Get_Clr_Context(), action, probability, uniqueKeyPtr.ToPointer());

		uniqueKeyHandle.Free();
	}
private:
	Native_Recorder_Callback* m_func;
};

// NativePolicy listens to callback event and reroute it to the managed Policy instance
class NativePolicy : public NativeMultiWorldTesting::IPolicy<NativeContext>
{
public:
	NativePolicy(Native_Policy_Callback* func)
	{
		m_func = func;
	}

	u32 Choose_Action(NativeContext& context)
	{
		return m_func(context.Get_Clr_Explorer(), context.Get_Clr_Context());
	}

private:
	Native_Policy_Callback* m_func;
};

// Triggers callback to the Policy instance to choose an action
generic <class Ctx>
public ref class PolicyCallback
{
internal:
	virtual UInt32 InvokePolicyCallback(Ctx context) = 0;

	PolicyCallback()
	{
		ClrPolicyCallback^ policyCallback = gcnew ClrPolicyCallback(&PolicyCallback<Ctx>::InteropInvoke);
		IntPtr policyCallbackPtr = Marshal::GetFunctionPointerForDelegate(policyCallback);
		Native_Policy_Callback* callback = static_cast<Native_Policy_Callback*>(policyCallbackPtr.ToPointer());
		m_native_policy = new NativePolicy(callback);
	}

	~PolicyCallback()
	{
		delete m_native_policy;
	}

	NativePolicy* GetNativePolicy()
	{
		return m_native_policy;
	}

	static UInt32 InteropInvoke(IntPtr callbackPtr, IntPtr contextPtr)
	{
		GCHandle callbackHandle = (GCHandle)callbackPtr;
		PolicyCallback<Ctx>^ callback = (PolicyCallback<Ctx>^)callbackHandle.Target;

		GCHandle contextHandle = (GCHandle)contextPtr;
		Ctx context = (Ctx)contextHandle.Target;

		return callback->InvokePolicyCallback(context);
	}

private:
	NativePolicy* m_native_policy;
};

// Triggers callback to the Recorder instance to record interaction data
generic <class Ctx>
public ref class RecorderCallback
{
internal:
	virtual void InvokeRecorderCallback(Ctx context, UInt32 action, float probability, String^ unique_key) = 0;

	RecorderCallback()
	{
		ClrRecorderCallback^ recorderCallback = gcnew ClrRecorderCallback(&RecorderCallback<Ctx>::InteropInvoke);
		IntPtr recorderCallbackPtr = Marshal::GetFunctionPointerForDelegate(recorderCallback);
		Native_Recorder_Callback* callback = static_cast<Native_Recorder_Callback*>(recorderCallbackPtr.ToPointer());
		m_native_recorder = new NativeRecorder(callback);
	}

	~RecorderCallback()
	{
		delete m_native_recorder;
	}

	NativeRecorder* GetNativeRecorder()
	{
		return m_native_recorder;
	}

	static void InteropInvoke(IntPtr mwtPtr, IntPtr contextPtr, UInt32 action, float probability, IntPtr uniqueKeyPtr)
	{
		GCHandle mwtHandle = (GCHandle)mwtPtr;
		RecorderCallback<Ctx>^ callback = (RecorderCallback<Ctx>^)mwtHandle.Target;

		GCHandle contextHandle = (GCHandle)contextPtr;
		Ctx context = (Ctx)contextHandle.Target;

		GCHandle uniqueKeyHandle = (GCHandle)uniqueKeyPtr;
		String^ uniqueKey = (String^)uniqueKeyHandle.Target;

		callback->InvokeRecorderCallback(context, action, probability, uniqueKey);
	}

private:
	NativeRecorder* m_native_recorder;
};

}