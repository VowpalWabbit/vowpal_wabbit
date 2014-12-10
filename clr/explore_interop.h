#pragma once

#define MANAGED_CODE

#include "explore_interface.h"
#include "MWTExplorer.h"

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Collections::Generic;
using namespace System::IO;
using namespace System::Runtime::InteropServices;
using namespace System::Xml::Serialization;
using namespace msclr::interop;

namespace MultiWorldTesting {

// Policy callback
private delegate UInt32 ClrPolicyCallback(IntPtr explorerPtr, IntPtr contextPtr, int index);
typedef u32 Native_Policy_Callback(void* explorer, void* context, int index);

// Scorer callback
private delegate void ClrScorerCallback(IntPtr explorerPtr, IntPtr contextPtr, IntPtr scores, IntPtr size);
typedef void Native_Scorer_Callback(void* explorer, void* context, float* scores[], u32* size);

// Recorder callback
private delegate void ClrRecorderCallback(IntPtr mwtPtr, IntPtr contextPtr, UInt32 action, float probability, IntPtr uniqueKey);
typedef void Native_Recorder_Callback(void* mwt, void* context, u32 action, float probability, void* unique_key);

// ToString callback
private delegate void ClrToStringCallback(IntPtr contextPtr, IntPtr stringValue);
typedef void Native_To_String_Callback(void* explorer, void* string_value);

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

class NativeStringContext
{
public:
	NativeStringContext(void* clr_context, Native_To_String_Callback* func)
	{
		m_clr_context = clr_context;
		m_func = func;
	}

	string To_String()
	{
		string value;
		m_func(m_clr_context, &value);
		return value;
	}
private:
	void* m_clr_context;
	Native_To_String_Callback* m_func;
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
	NativePolicy(Native_Policy_Callback* func, int index = -1)
	{
		m_func = func;
		m_index = index;
	}

	u32 Choose_Action(NativeContext& context)
	{
		return m_func(context.Get_Clr_Explorer(), context.Get_Clr_Context(), m_index);
	}

private:
	Native_Policy_Callback* m_func;
	int m_index;
};

class NativeScorer : public NativeMultiWorldTesting::IScorer<NativeContext>
{
public:
	NativeScorer(Native_Scorer_Callback* func)
	{
		m_func = func;
	}

	vector<float> Score_Actions(NativeContext& context)
	{
		float* scores = nullptr;
		u32 num_scores = 0;
		m_func(context.Get_Clr_Explorer(), context.Get_Clr_Context(), &scores, &num_scores);

		// It's ok if scores is null, vector will be empty
		vector<float> scores_vector(scores, scores + num_scores);
		delete[] scores;

		return scores_vector;
	}
private:
	Native_Scorer_Callback* m_func;
};

// Triggers callback to the Policy instance to choose an action
generic <class Ctx>
public ref class PolicyCallback abstract
{
internal:
	virtual UInt32 InvokePolicyCallback(Ctx context, int index) = 0;

	PolicyCallback()
	{
		policyCallback = gcnew ClrPolicyCallback(&PolicyCallback<Ctx>::InteropInvoke);
		IntPtr policyCallbackPtr = Marshal::GetFunctionPointerForDelegate(policyCallback);
		m_callback = static_cast<Native_Policy_Callback*>(policyCallbackPtr.ToPointer());
		m_native_policy = nullptr;
		m_native_policies = nullptr;
	}

	~PolicyCallback()
	{
		delete m_native_policy;
		delete m_native_policies;
	}

	NativePolicy* GetNativePolicy()
	{
		if (m_native_policy == nullptr)
		{
			m_native_policy = new NativePolicy(m_callback);
		}
		return m_native_policy;
	}
  
  NativeMultiWorldTesting::IPolicy<NativeContext>::Vector* GetNativePolicies(int count)
	{
		if (m_native_policies == nullptr)
		{
      m_native_policies = new NativeMultiWorldTesting::IPolicy<NativeContext>::Vector();
			for (int i = 0; i < count; i++)
			{
        m_native_policies->push_back(NativeMultiWorldTesting::IPolicy<NativeContext>::Ptr(new NativePolicy(m_callback, i)));
			}
		}

		return m_native_policies;
	}

	static UInt32 InteropInvoke(IntPtr callbackPtr, IntPtr contextPtr, int index)
	{
		GCHandle callbackHandle = (GCHandle)callbackPtr;
		PolicyCallback<Ctx>^ callback = (PolicyCallback<Ctx>^)callbackHandle.Target;

		GCHandle contextHandle = (GCHandle)contextPtr;
		Ctx context = (Ctx)contextHandle.Target;

		return callback->InvokePolicyCallback(context, index);
	}

private:
	ClrPolicyCallback^ policyCallback;

private:
	NativePolicy* m_native_policy;
  NativeMultiWorldTesting::IPolicy<NativeContext>::Vector* m_native_policies;
	Native_Policy_Callback* m_callback;
};

// Triggers callback to the Recorder instance to record interaction data
generic <class Ctx>
public ref class RecorderCallback abstract
{
internal:
	virtual void InvokeRecorderCallback(Ctx context, UInt32 action, float probability, String^ unique_key) = 0;

	RecorderCallback()
	{
		recorderCallback = gcnew ClrRecorderCallback(&RecorderCallback<Ctx>::InteropInvoke);
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
	ClrRecorderCallback^ recorderCallback;

private:
	NativeRecorder* m_native_recorder;
};

// Triggers callback to the Recorder instance to record interaction data
generic <class Ctx>
public ref class ScorerCallback abstract
{
internal:
	virtual List<float>^ InvokeScorerCallback(Ctx context) = 0;

	ScorerCallback()
	{
		scorerCallback = gcnew ClrScorerCallback(&ScorerCallback<Ctx>::InteropInvoke);
		IntPtr scorerCallbackPtr = Marshal::GetFunctionPointerForDelegate(scorerCallback);
		Native_Scorer_Callback* callback = static_cast<Native_Scorer_Callback*>(scorerCallbackPtr.ToPointer());
		m_native_scorer = new NativeScorer(callback);
	}

	~ScorerCallback()
	{
		delete m_native_scorer;
	}

	NativeScorer* GetNativeScorer()
	{
		return m_native_scorer;
	}

	static void InteropInvoke(IntPtr callbackPtr, IntPtr contextPtr, IntPtr scoresPtr, IntPtr sizePtr)
	{
		GCHandle callbackHandle = (GCHandle)callbackPtr;
		ScorerCallback<Ctx>^ callback = (ScorerCallback<Ctx>^)callbackHandle.Target;

		GCHandle contextHandle = (GCHandle)contextPtr;
		Ctx context = (Ctx)contextHandle.Target;

		List<float>^ scoreList = callback->InvokeScorerCallback(context);
		
		if (scoreList == nullptr || scoreList->Count == 0)
		{
			return;
		}

		u32* num_scores = (u32*)sizePtr.ToPointer();
		*num_scores = (u32)scoreList->Count;

		float* scores = new float[*num_scores];
		for (u32 i = 0; i < *num_scores; i++)
		{
			scores[i] = scoreList[i];
		}

		float** native_scores = (float**)scoresPtr.ToPointer();
		*native_scores = scores;
	}

private:
	ClrScorerCallback^ scorerCallback;

private:
	NativeScorer* m_native_scorer;
};

// Triggers callback to the Context instance to perform ToString() operation
generic <class Ctx> where Ctx : IStringContext
public ref class ToStringCallback
{
internal:
	ToStringCallback()
	{
		toStringCallback = gcnew ClrToStringCallback(&ToStringCallback<Ctx>::InteropInvoke);
		IntPtr toStringCallbackPtr = Marshal::GetFunctionPointerForDelegate(toStringCallback);
		m_callback = static_cast<Native_To_String_Callback*>(toStringCallbackPtr.ToPointer());
	}

	Native_To_String_Callback* GetCallback()
	{
		return m_callback;
	}

	static void InteropInvoke(IntPtr contextPtr, IntPtr stringPtr)
	{
		GCHandle contextHandle = (GCHandle)contextPtr;
		Ctx context = (Ctx)contextHandle.Target;

		string* out_string = (string*)stringPtr.ToPointer();
		*out_string = marshal_as<string>(context->ToString());
	}

private:
	ClrToStringCallback^ toStringCallback;

private:
	Native_To_String_Callback* m_callback;
};

}