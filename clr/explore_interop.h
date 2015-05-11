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

// Context callback
private delegate UInt32 ClrContextGetNumActionsCallback(IntPtr mwtPtr, IntPtr contextPtr);
typedef u32 Native_Context_Get_Num_Actions_Callback(void* mwt, void* context);

// Policy callback
private delegate void ClrPolicyCallback(IntPtr explorerPtr, IntPtr contextPtr, IntPtr actionsPtr, int index);
typedef void Native_Policy_Callback(void* explorer, void* context, void* actions, int index);

// Scorer callback
private delegate void ClrScorerCallback(IntPtr explorerPtr, IntPtr contextPtr, IntPtr scores, IntPtr size);
typedef void Native_Scorer_Callback(void* explorer, void* context, float* scores[], u32* size);

// Recorder callback
private delegate void ClrRecorderCallback(IntPtr mwtPtr, IntPtr contextPtr, IntPtr actionsPtr, float probability, IntPtr uniqueKey);
typedef void Native_Recorder_Callback(void* mwt, void* context, void* actions, float probability, void* unique_key);

// ToString callback
private delegate void ClrToStringCallback(IntPtr contextPtr, IntPtr stringValue);
typedef void Native_To_String_Callback(void* explorer, void* string_value);

// NativeContext travels through interop space and contains instances of Mwt, Explorer, Context
// used for triggering callback for Policy, Scorer, Recorder
class NativeContext : public NativeMultiWorldTesting::IVariableActionContext
{
public:
	NativeContext(void* clr_mwt, void* clr_explorer, void* clr_context,
        Native_Context_Get_Num_Actions_Callback* callback_num_actions)
	{
		m_clr_mwt = clr_mwt;
		m_clr_explorer = clr_explorer;
		m_clr_context = clr_context;

        m_callback_num_actions = callback_num_actions;
	}

    u32 Get_Number_Of_Actions()
    {
        return m_callback_num_actions(m_clr_mwt, m_clr_context);
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

    void* Get_Clr_Action_List()
    {
        return m_clr_action_list;
    }

    void Set_Clr_Action_List(void* clr_action_list)
    {
        m_clr_action_list = clr_action_list;
    }

private:
	void* m_clr_mwt;
	void* m_clr_context;
	void* m_clr_explorer;
    void* m_clr_action_list;

private:
    Native_Context_Get_Num_Actions_Callback* m_callback_num_actions;
};

class NativeStringContext
{
public:
    NativeStringContext(void* clr_context, Native_To_String_Callback* func) : m_func(func)
	{
		m_clr_context = clr_context;
	}

	string To_String()
	{
		string value;
		m_func(m_clr_context, &value);
		return value;
	}
private:
	void* m_clr_context;
	Native_To_String_Callback* const m_func;
};

// NativeRecorder listens to callback event and reroute it to the managed Recorder instance
class NativeRecorder : public NativeMultiWorldTesting::IRecorder<NativeContext>
{
public:
    NativeRecorder(Native_Recorder_Callback* native_func) : m_func(native_func)
	{
	}

    void Record(NativeContext& context, u32* actions, u32 num_actions, float probability, string unique_key)
	{
        // Normal handles are sufficient here since native code will only hold references and not access the object's data
        // https://www.microsoftpressstore.com/articles/article.aspx?p=2224054&seqNum=4
		GCHandle uniqueKeyHandle = GCHandle::Alloc(gcnew String(unique_key.c_str()));
        try
        {
            IntPtr uniqueKeyPtr = (IntPtr)uniqueKeyHandle;

            m_func(context.Get_Clr_Mwt(), context.Get_Clr_Context(), context.Get_Clr_Action_List(), probability, uniqueKeyPtr.ToPointer());
        }
        finally
        {
            if (uniqueKeyHandle.IsAllocated)
            {
                uniqueKeyHandle.Free();
            }
        }
	}
private:
	Native_Recorder_Callback* const m_func;
};

// NativePolicy listens to callback event and reroute it to the managed Policy instance
class NativePolicy : public NativeMultiWorldTesting::IPolicy<NativeContext>
{
public:
    NativePolicy(Native_Policy_Callback* func, int index = -1) : m_func(func)
	{
		m_index = index;
	}

    void Choose_Action(NativeContext& context, u32* actions, u32 num_actions)
	{
		m_func(context.Get_Clr_Explorer(), context.Get_Clr_Context(), context.Get_Clr_Action_List(), m_index);
	}

private:
	Native_Policy_Callback* const m_func;
	int m_index;
};

class NativeScorer : public NativeMultiWorldTesting::IScorer<NativeContext>
{
public:
    NativeScorer(Native_Scorer_Callback* func) : m_func(func)
	{
	}

	vector<float> Score_Actions(NativeContext& context)
	{
		float* scores = nullptr;
		u32 num_scores = 0;
        try
        {
            m_func(context.Get_Clr_Explorer(), context.Get_Clr_Context(), &scores, &num_scores);
            // It's ok if scores is null, vector will be empty
            vector<float> scores_vector(scores, scores + num_scores);
            return scores_vector;
        }
        finally
        {
            delete[] scores;
        }
	}
private:
	Native_Scorer_Callback* const m_func;
};

// Triggers callback to the Context instance
generic <class Ctx>
public ref class ContextCallback
{
internal:
    ContextCallback(Func<Ctx, UInt32>^ func)
    {
        contextNumActionsCallback = gcnew ClrContextGetNumActionsCallback(&ContextCallback<Ctx>::InteropInvokeNumActions);
        IntPtr contextNumActionsCallbackPtr = Marshal::GetFunctionPointerForDelegate(contextNumActionsCallback);
        m_num_actions_callback = static_cast<Native_Context_Get_Num_Actions_Callback*>(contextNumActionsCallbackPtr.ToPointer());
        getNumberOfActionsFunc = func;
    }

    Native_Context_Get_Num_Actions_Callback* GetNumActionsCallback()
    {
        return m_num_actions_callback;
    }

    static UInt32 InteropInvokeNumActions(IntPtr mwtPtr, IntPtr contextPtr)
    {
        GCHandle mwtHandle = (GCHandle)mwtPtr;
        ContextCallback<Ctx>^ callback = (ContextCallback<Ctx>^)mwtHandle.Target;

        GCHandle contextHandle = (GCHandle)contextPtr;

        if (callback->getNumberOfActionsFunc == nullptr)
        {
            throw gcnew InvalidOperationException("A callback to retrieve number of actions for the current context has not been set.");
        }

        return callback->getNumberOfActionsFunc((Ctx)contextHandle.Target);
    }

private:
    initonly ClrContextGetNumActionsCallback^ contextNumActionsCallback;
    initonly Func<Ctx, UInt32>^ getNumberOfActionsFunc;

private:
    Native_Context_Get_Num_Actions_Callback* m_num_actions_callback;
};

// Triggers callback to the Policy instance to choose an action
generic <class Ctx>
public ref class PolicyCallback abstract
{
internal:
	virtual void InvokePolicyCallback(Ctx context, cli::array<UInt32>^ actions, int index) = 0;

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
  
  vector<unique_ptr<NativeMultiWorldTesting::IPolicy<NativeContext>>>* GetNativePolicies(int count)
	{
		if (m_native_policies == nullptr)
		{
            m_native_policies = new vector<unique_ptr<NativeMultiWorldTesting::IPolicy<NativeContext>>>();
			for (int i = 0; i < count; i++)
			{
                m_native_policies->push_back(unique_ptr<NativeMultiWorldTesting::IPolicy<NativeContext>>(new NativePolicy(m_callback, i)));
			}
		}

		return m_native_policies;
	}

	static void InteropInvoke(IntPtr callbackPtr, IntPtr contextPtr, IntPtr actionsPtr, int index)
	{
		GCHandle callbackHandle = (GCHandle)callbackPtr;
		PolicyCallback<Ctx>^ callback = (PolicyCallback<Ctx>^)callbackHandle.Target;

		GCHandle contextHandle = (GCHandle)contextPtr;
		Ctx context = (Ctx)contextHandle.Target;

        GCHandle actionsHandle = (GCHandle)actionsPtr;
        cli::array<UInt32>^ actions = (cli::array<UInt32>^)actionsHandle.Target;

        callback->InvokePolicyCallback(context, actions, index);
	}

private:
	initonly ClrPolicyCallback^ policyCallback;

private:
	NativePolicy* m_native_policy;
	vector<unique_ptr<NativeMultiWorldTesting::IPolicy<NativeContext>>>* m_native_policies;
	Native_Policy_Callback* m_callback;
};

// Triggers callback to the Recorder instance to record interaction data
generic <class Ctx>
public ref class RecorderCallback abstract : public ContextCallback<Ctx>
{
internal:
    virtual void InvokeRecorderCallback(Ctx context, cli::array<UInt32>^ actions, float probability, String^ unique_key) = 0;

    RecorderCallback(Func<Ctx, UInt32>^ func) : ContextCallback(func)
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

    static void InteropInvoke(IntPtr mwtPtr, IntPtr contextPtr, IntPtr actionsPtr, float probability, IntPtr uniqueKeyPtr)
	{
		GCHandle mwtHandle = (GCHandle)mwtPtr;
		RecorderCallback<Ctx>^ callback = (RecorderCallback<Ctx>^)mwtHandle.Target;

		GCHandle contextHandle = (GCHandle)contextPtr;
		Ctx context = (Ctx)contextHandle.Target;

		GCHandle uniqueKeyHandle = (GCHandle)uniqueKeyPtr;
		String^ uniqueKey = (String^)uniqueKeyHandle.Target;

        GCHandle actionsHandle = (GCHandle)actionsPtr;
        cli::array<UInt32>^ actions = (cli::array<UInt32>^)actionsHandle.Target;

        callback->InvokeRecorderCallback(context, actions, probability, uniqueKey);
	}

private:
	initonly ClrRecorderCallback^ recorderCallback;

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
	initonly ClrScorerCallback^ scorerCallback;

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
	initonly ClrToStringCallback^ toStringCallback;

private:
	Native_To_String_Callback* m_callback;
};

}