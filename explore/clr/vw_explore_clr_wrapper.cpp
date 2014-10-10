// vw_explore_clr_wrapper.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "vw_explore_clr_wrapper.h"

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace msclr::interop;

namespace MultiWorldTesting {

	MwtExplorer::MwtExplorer()
	{
		m_mwt = new MWTExplorer();
	}

	MwtExplorer::~MwtExplorer()
	{
		this->Unintialize();
	}

	void MwtExplorer::Unintialize()
	{
		if (selfHandle.IsAllocated)
		{
			selfHandle.Free();
		}
		if (baggingParameters != nullptr)
		{
			for (int i = 0; i < baggingParameters->Length; i++)
			{
				GCHandle handle = (GCHandle)baggingParameters[i];
				if (handle.IsAllocated)
				{
					handle.Free();
				}
			}
			baggingParameters = nullptr;
		}
		delete m_mwt;
		m_mwt = nullptr;
	}

	generic <class T>
	void MwtExplorer::InitializeEpsilonGreedy(float epsilon, TemplateStatefulPolicyDelegate<T>^ defaultPolicyFunc, T defaultPolicyFuncParams, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<T>(defaultPolicyFunc, defaultPolicyFuncParams);
		selfHandle = GCHandle::Alloc(this);
		StatefulPolicyDelegate^ spDelegate = gcnew StatefulPolicyDelegate(&MwtExplorer::InternalStatefulPolicy);
		
		this->InitializeEpsilonGreedy(epsilon, spDelegate, (IntPtr)selfHandle, numActions);
	}

	void MwtExplorer::InitializeEpsilonGreedy(float epsilon, TemplateStatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<int>(defaultPolicyFunc);
		selfHandle = GCHandle::Alloc(this);
		StatefulPolicyDelegate^ spDelegate = gcnew StatefulPolicyDelegate(&MwtExplorer::InternalStatefulPolicy);

		this->InitializeEpsilonGreedy(epsilon, spDelegate, (IntPtr)selfHandle, numActions);
	}

	generic <class T>
	void MwtExplorer::InitializeTauFirst(UInt32 tau, TemplateStatefulPolicyDelegate<T>^ defaultPolicyFunc, T defaultPolicyFuncParams, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<T>(defaultPolicyFunc, defaultPolicyFuncParams);
		selfHandle = GCHandle::Alloc(this);
		StatefulPolicyDelegate^ spDelegate = gcnew StatefulPolicyDelegate(&MwtExplorer::InternalStatefulPolicy);

		this->InitializeTauFirst(tau, spDelegate, (IntPtr)selfHandle, numActions);
	}

	void MwtExplorer::InitializeTauFirst(UInt32 tau, TemplateStatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<int>(defaultPolicyFunc);
		selfHandle = GCHandle::Alloc(this);
		StatefulPolicyDelegate^ spDelegate = gcnew StatefulPolicyDelegate(&MwtExplorer::InternalStatefulPolicy);

		this->InitializeTauFirst(tau, spDelegate, (IntPtr)selfHandle, numActions);
	}

	generic <class T>
	void MwtExplorer::InitializeBagging(UInt32 bags, cli::array<TemplateStatefulPolicyDelegate<T>^>^ defaultPolicyFuncs, cli::array<T>^ defaultPolicyArgs, UInt32 numActions)
	{
		policyWrappers = gcnew cli::array<IFunctionWrapper^>(defaultPolicyFuncs->Length);
		cli::array<StatefulPolicyDelegate^>^ spDelegates = gcnew cli::array<StatefulPolicyDelegate^>(policyWrappers->Length);
		baggingParameters = gcnew cli::array<IntPtr>(policyWrappers->Length);

		for (int i = 0; i < policyWrappers->Length; i++)
		{
			policyWrappers[i] = gcnew DefaultPolicyWrapper<T>(defaultPolicyFuncs[i], defaultPolicyArgs[i]);
			spDelegates[i] = gcnew StatefulPolicyDelegate(&MwtExplorer::BaggingStatefulPolicy);

			BaggingParameter bp;
			bp.Mwt = this;
			bp.BagIndex = i;

			baggingParameters[i] = (IntPtr)GCHandle::Alloc(bp);
		}

		this->InitializeBagging(bags, spDelegates, baggingParameters, numActions);
	}

	void MwtExplorer::InitializeBagging(UInt32 bags, cli::array<TemplateStatelessPolicyDelegate^>^ defaultPolicyFuncs, UInt32 numActions)
	{
		policyWrappers = gcnew cli::array<IFunctionWrapper^>(defaultPolicyFuncs->Length);
		cli::array<StatefulPolicyDelegate^>^ spDelegates = gcnew cli::array<StatefulPolicyDelegate^>(policyWrappers->Length);
		baggingParameters = gcnew cli::array<IntPtr>(policyWrappers->Length);

		for (int i = 0; i < policyWrappers->Length; i++)
		{
			policyWrappers[i] = gcnew DefaultPolicyWrapper<int>(defaultPolicyFuncs[i]);
			spDelegates[i] = gcnew StatefulPolicyDelegate(&MwtExplorer::BaggingStatefulPolicy);

			BaggingParameter bp;
			bp.Mwt = this;
			bp.BagIndex = i;

			baggingParameters[i] = (IntPtr)GCHandle::Alloc(bp);
		}

		this->InitializeBagging(bags, spDelegates, baggingParameters, numActions);
	}

	generic <class T>
	void MwtExplorer::InitializeSoftmax(float lambda, TemplateStatefulScorerDelegate<T>^ defaultScorerFunc, T defaultScorerFuncParams, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<T>(defaultScorerFunc, defaultScorerFuncParams);
		selfHandle = GCHandle::Alloc(this);
		
		StatefulScorerDelegate^ ssDelegate = gcnew StatefulScorerDelegate(&MwtExplorer::InternalScorerFunction);

		this->InitializeSoftmax(lambda, ssDelegate, (IntPtr)selfHandle, numActions);
	}

	void MwtExplorer::InitializeEpsilonGreedy(float epsilon, StatefulPolicyDelegate^ defaultPolicyFunc, System::IntPtr defaultPolicyFuncContext, UInt32 numActions)
	{
		GCHandle gch = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateful_Policy_Func* nativeFunc = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
		m_mwt->Initialize_Epsilon_Greedy(epsilon, nativeFunc, defaultPolicyFuncContext.ToPointer(), numActions);

		gch.Free();
	}

	void MwtExplorer::InitializeEpsilonGreedy(float epsilon, StatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions)
	{
		GCHandle gch = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateless_Policy_Func* nativeFunc = static_cast<Stateless_Policy_Func*>(ip.ToPointer());
		m_mwt->Initialize_Epsilon_Greedy(epsilon, nativeFunc, numActions);

		gch.Free();
	}

	void MwtExplorer::InitializeTauFirst(UInt32 tau, StatefulPolicyDelegate^ defaultPolicyFunc, System::IntPtr defaultPolicyFuncContext, UInt32 numActions)
	{
		GCHandle gch = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateful_Policy_Func* nativeFunc = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
		m_mwt->Initialize_Tau_First(tau, nativeFunc, defaultPolicyFuncContext.ToPointer(), numActions);

		gch.Free();
	}

	void MwtExplorer::InitializeTauFirst(UInt32 tau, StatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions)
	{
		GCHandle gch = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateless_Policy_Func* nativeFunc = static_cast<Stateless_Policy_Func*>(ip.ToPointer());
		m_mwt->Initialize_Tau_First(tau, nativeFunc, numActions);

		gch.Free();
	}

	void MwtExplorer::InitializeBagging(UInt32 bags, cli::array<StatefulPolicyDelegate^>^ defaultPolicyFuncs, cli::array<IntPtr>^ defaultPolicyArgs, UInt32 numActions)
	{
		cli::array<GCHandle>^ gcHandles = gcnew cli::array<GCHandle>(bags);

		Stateful_Policy_Func** native_funcs = new Stateful_Policy_Func*[bags];
		void** native_args = new void*[bags];

		for (int i = 0; i < bags; i++)
		{
			gcHandles[i] = GCHandle::Alloc(defaultPolicyFuncs[i]);
			IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFuncs[i]);

			native_funcs[i] = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
			native_args[i] = defaultPolicyArgs[i].ToPointer();
		}

		m_mwt->Initialize_Bagging(bags, native_funcs, native_args, numActions);

		for (int i = 0; i < bags; i++)
		{
			gcHandles[i].Free();
		}
		delete[] native_funcs;
		delete[] native_args;
	}

	void MwtExplorer::InitializeBagging(UInt32 bags, cli::array<StatelessPolicyDelegate^>^ defaultPolicyFuncs, UInt32 numActions)
	{
		cli::array<GCHandle>^ gcHandles = gcnew cli::array<GCHandle>(bags);

		Stateless_Policy_Func** native_funcs = new Stateless_Policy_Func*[bags];

		for (int i = 0; i < bags; i++)
		{
			gcHandles[i] = GCHandle::Alloc(defaultPolicyFuncs[i]);
			IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFuncs[i]);

			native_funcs[i] = static_cast<Stateless_Policy_Func*>(ip.ToPointer());
		}

		m_mwt->Initialize_Bagging(bags, native_funcs, numActions);

		for (int i = 0; i < bags; i++)
		{
			gcHandles[i].Free();
		}
		delete[] native_funcs;
	}

	void MwtExplorer::InitializeSoftmax(float lambda, StatefulScorerDelegate^ defaultScorerFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions)
	{
		GCHandle gch = GCHandle::Alloc(defaultScorerFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultScorerFunc);

		Stateful_Scorer_Func* nativeFunc = static_cast<Stateful_Scorer_Func*>(ip.ToPointer());
		m_mwt->Initialize_Softmax(lambda, nativeFunc, defaultPolicyFuncContext.ToPointer(), numActions);

		gch.Free();
	}

	void MwtExplorer::InitializeSoftmax(float lambda, StatelessScorerDelegate^ defaultScorerFunc, UInt32 numActions)
	{
		GCHandle gch = GCHandle::Alloc(defaultScorerFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultScorerFunc);

		Stateless_Scorer_Func* nativeFunc = static_cast<Stateless_Scorer_Func*>(ip.ToPointer());
		m_mwt->Initialize_Softmax(lambda, nativeFunc, numActions);

		gch.Free();
	}

	UInt32 MwtExplorer::ChooseAction(CONTEXT^ context, String^ uniqueId)
	{
		UInt32 chosenAction = 0;

		GCHandle contextHandle = GCHandle::Alloc(context);
		IntPtr contextPtr = (IntPtr)contextHandle;

		cli::array<FEATURE>^ contextFeatures = context->Features;
		String^ otherContext = context->OtherContext;

		std::string* nativeOtherContext = (otherContext != nullptr) ? &marshal_as<std::string>(otherContext) : nullptr;
		std::string nativeUniqueKey = marshal_as<std::string>(uniqueId);

		pin_ptr<FEATURE> pinnedContextFeatures = &context->Features[0];
		FEATURE* nativeContextFeatures = pinnedContextFeatures;

		Context log_context((feature*)nativeContextFeatures, (size_t)context->Features->Length, nativeOtherContext);

		size_t uniqueIdLength = (size_t)uniqueId->Length;

		chosenAction = m_mwt->Choose_Action(
			contextPtr.ToPointer(),
			nativeUniqueKey, 
			log_context);

		contextHandle.Free();

		return chosenAction;
	}

	Tuple<UInt32, UInt64>^ MwtExplorer::ChooseActionAndKey(CONTEXT^ context)
	{
		GCHandle contextHandle = GCHandle::Alloc(context);
		IntPtr contextPtr = (IntPtr)contextHandle;

		cli::array<FEATURE>^ contextFeatures = context->Features;
		String^ otherContext = context->OtherContext;

		std::string* nativeOtherContext = (otherContext != nullptr) ? &marshal_as<std::string>(otherContext) : nullptr;

		pin_ptr<FEATURE> pinnedContextFeatures = &context->Features[0];
		FEATURE* nativeContextFeatures = pinnedContextFeatures;

		Context log_context((feature*)nativeContextFeatures, (size_t)context->Features->Length, nativeOtherContext);

		std::pair<u32, u64> actionAndKey = m_mwt->Choose_Action_And_Key(
			contextPtr.ToPointer(),
			log_context);

		Tuple<UInt32, UInt64>^ chosenActionAndKey = gcnew Tuple<UInt32, UInt64>(actionAndKey.first, actionAndKey.second);

		contextHandle.Free();

		return chosenActionAndKey;
	}

	String^ MwtExplorer::GetAllInteractionsAsString()
	{
		std::string all_interactions = m_mwt->Get_All_Interactions();
		return gcnew String(all_interactions.c_str());
	}

	cli::array<INTERACTION^>^ MwtExplorer::GetAllInteractions()
	{
		size_t num_interactions = 0;
		Interaction** native_interactions = nullptr;
		m_mwt->Get_All_Interactions(num_interactions, native_interactions);

		cli::array<INTERACTION^>^ interactions = gcnew cli::array<INTERACTION^>((int)num_interactions);
		if (num_interactions > 0 && native_interactions != nullptr)
		{
			for (size_t i = 0; i < num_interactions; i++)
			{
				interactions[i] = gcnew INTERACTION();

				Context* native_context = native_interactions[i]->Get_Context();

				feature* native_features = nullptr;
				size_t native_num_features = 0;
				native_context->Get_Features(native_features, native_num_features);
				cli::array<FEATURE>^ features = gcnew cli::array<FEATURE>((int)native_num_features);
				for (int i = 0; i < features->Length; i++)
				{
					features[i].X = native_features[i].x;
					features[i].WeightIndex = native_features[i].weight_index;
				}

				std::string* native_other_context = nullptr;
				native_context->Get_Other_Context(native_other_context);
				String^ otherContext = (native_other_context == nullptr) ? nullptr : gcnew String(native_other_context->c_str());

				interactions[i]->ApplicationContext = gcnew CONTEXT(features, otherContext);
				interactions[i]->ChosenAction = native_interactions[i]->Get_Action().Get_Id();
				interactions[i]->Probability = native_interactions[i]->Get_Prob();
				interactions[i]->JoinId = native_interactions[i]->Get_Id();

				delete native_interactions[i];
			}
			delete[] native_interactions;
		}

		return interactions;
	}

	UInt32 MwtExplorer::InvokeDefaultPolicyFunction(CONTEXT^ context)
	{
		return policyWrapper->InvokeFunction(context);
	}

	UInt32 MwtExplorer::InvokeBaggingDefaultPolicyFunction(CONTEXT^ context, int bagIndex)
	{
		return policyWrappers[bagIndex]->InvokeFunction(context);
	}

	void MwtExplorer::InvokeDefaultScorerFunction(CONTEXT^ context, cli::array<float>^ scores)
	{
		policyWrapper->InvokeScorer(context, scores);
	}

	UInt32 MwtExplorer::InternalStatefulPolicy(IntPtr mwtPtr, IntPtr contextPtr)
	{
		GCHandle mwtHandle = (GCHandle)mwtPtr;
		MwtExplorer^ mwt = (MwtExplorer^)(mwtHandle.Target);

		GCHandle contextHandle = (GCHandle)contextPtr;
		CONTEXT^ context = (CONTEXT^)(contextHandle.Target);

		return mwt->InvokeDefaultPolicyFunction(context);
	}

	UInt32 MwtExplorer::BaggingStatefulPolicy(IntPtr baggingParamPtr, IntPtr contextPtr)
	{
		GCHandle mwtHandle = (GCHandle)baggingParamPtr;
		BaggingParameter bp = (BaggingParameter)(mwtHandle.Target);

		GCHandle contextHandle = (GCHandle)contextPtr;
		CONTEXT^ context = (CONTEXT^)(contextHandle.Target);

		return bp.Mwt->InvokeBaggingDefaultPolicyFunction(context, bp.BagIndex);
	}

	void MwtExplorer::InternalScorerFunction(IntPtr mwtPtr, IntPtr contextPtr, IntPtr scoresPtr, UInt32 numScores)
	{
		GCHandle mwtHandle = (GCHandle)mwtPtr;
		MwtExplorer^ mwt = (MwtExplorer^)(mwtHandle.Target);

		GCHandle contextHandle = (GCHandle)contextPtr;
		CONTEXT^ context = (CONTEXT^)(contextHandle.Target);

		cli::array<float>^ scores = MwtExplorer::IntPtrToScoreArray(scoresPtr, numScores);

		return mwt->InvokeDefaultScorerFunction(context, scores);
	}

	cli::array<float>^ MwtExplorer::IntPtrToScoreArray(IntPtr scoresPtr, UInt32 size)
	{
		// PERF: scores are sent from native space to managed space and thus have to be copied.
		cli::array<float>^ scores = gcnew cli::array<float>(size);
		Marshal::Copy(scoresPtr, scores, 0, (int)size);
		return scores;
	}

	generic <class T>
	T MwtExplorer::FromIntPtr(IntPtr objectPtr)
	{
		GCHandle contextHandle = (GCHandle)objectPtr;
		T obj = (T)(contextHandle.Target);
		return obj;
	}

	generic <class T>
	IntPtr MwtExplorer::ToIntPtr(T obj, [Out] GCHandle% objHandle)
	{
		objHandle = GCHandle::Alloc(obj);
		return (IntPtr)objHandle;
	}
}