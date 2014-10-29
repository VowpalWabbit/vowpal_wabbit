// vw_explore_clr_wrapper.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "vw_explore_clr_wrapper.h"

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace msclr::interop;
using namespace NativeMultiWorldTesting;

namespace MultiWorldTesting {

	MwtExplorer::MwtExplorer(String^ app_id)
	{
		std::string native_app_id = marshal_as<std::string>(app_id);
		m_mwt = new NativeMultiWorldTesting::MWTExplorer(native_app_id);
		m_bagging_funcs = nullptr;
		m_bagging_func_params = nullptr;
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
		if (policyFuncHandle.IsAllocated)
		{
			policyFuncHandle.Free();
		}

		if (baggingFuncHandles != nullptr)
		{
			for (int i = 0; i < baggingFuncHandles->Length; i++)
			{
				if (baggingFuncHandles[i].IsAllocated)
				{
					baggingFuncHandles[i].Free();
				}
			}
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
		delete m_bagging_funcs;
		m_bagging_funcs = nullptr;
		delete m_bagging_func_params;
		m_bagging_func_params = nullptr;
	}

	generic <class T>
	void MwtExplorer::InitializeEpsilonGreedy(float epsilon, StatefulPolicyDelegate<T>^ defaultPolicyFunc, T defaultPolicyFuncParams, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<T>(defaultPolicyFunc, defaultPolicyFuncParams);
		selfHandle = GCHandle::Alloc(this);
		InternalStatefulPolicyDelegate^ spDelegate = gcnew InternalStatefulPolicyDelegate(&MwtExplorer::InternalStatefulPolicy);
		
		this->InitializeEpsilonGreedy(epsilon, spDelegate, (IntPtr)selfHandle, numActions);
	}

	void MwtExplorer::InitializeEpsilonGreedy(float epsilon, StatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<int>(defaultPolicyFunc);
		selfHandle = GCHandle::Alloc(this);
		InternalStatefulPolicyDelegate^ spDelegate = gcnew InternalStatefulPolicyDelegate(&MwtExplorer::InternalStatefulPolicy);

		this->InitializeEpsilonGreedy(epsilon, spDelegate, (IntPtr)selfHandle, numActions);
	}

	generic <class T>
	void MwtExplorer::InitializeTauFirst(UInt32 tau, StatefulPolicyDelegate<T>^ defaultPolicyFunc, T defaultPolicyFuncParams, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<T>(defaultPolicyFunc, defaultPolicyFuncParams);
		selfHandle = GCHandle::Alloc(this);
		InternalStatefulPolicyDelegate^ spDelegate = gcnew InternalStatefulPolicyDelegate(&MwtExplorer::InternalStatefulPolicy);

		this->InitializeTauFirst(tau, spDelegate, (IntPtr)selfHandle, numActions);
	}

	void MwtExplorer::InitializeTauFirst(UInt32 tau, StatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<int>(defaultPolicyFunc);
		selfHandle = GCHandle::Alloc(this);
		InternalStatefulPolicyDelegate^ spDelegate = gcnew InternalStatefulPolicyDelegate(&MwtExplorer::InternalStatefulPolicy);

		this->InitializeTauFirst(tau, spDelegate, (IntPtr)selfHandle, numActions);
	}

	generic <class T>
	void MwtExplorer::InitializeBagging(UInt32 bags, cli::array<StatefulPolicyDelegate<T>^>^ defaultPolicyFuncs, cli::array<T>^ defaultPolicyArgs, UInt32 numActions)
	{
		policyWrappers = gcnew cli::array<IFunctionWrapper^>(defaultPolicyFuncs->Length);
		cli::array<InternalStatefulPolicyDelegate^>^ spDelegates = gcnew cli::array<InternalStatefulPolicyDelegate^>(policyWrappers->Length);
		baggingParameters = gcnew cli::array<IntPtr>(policyWrappers->Length);

		for (int i = 0; i < policyWrappers->Length; i++)
		{
			policyWrappers[i] = gcnew DefaultPolicyWrapper<T>(defaultPolicyFuncs[i], defaultPolicyArgs[i]);
			spDelegates[i] = gcnew InternalStatefulPolicyDelegate(&MwtExplorer::BaggingStatefulPolicy);

			BaggingParameter bp;
			bp.Mwt = this;
			bp.BagIndex = i;

			baggingParameters[i] = (IntPtr)GCHandle::Alloc(bp);
		}

		this->InitializeBagging(bags, spDelegates, baggingParameters, numActions);
	}

	void MwtExplorer::InitializeBagging(UInt32 bags, cli::array<StatelessPolicyDelegate^>^ defaultPolicyFuncs, UInt32 numActions)
	{
		policyWrappers = gcnew cli::array<IFunctionWrapper^>(defaultPolicyFuncs->Length);
		cli::array<InternalStatefulPolicyDelegate^>^ spDelegates = gcnew cli::array<InternalStatefulPolicyDelegate^>(policyWrappers->Length);
		baggingParameters = gcnew cli::array<IntPtr>(policyWrappers->Length);

		for (int i = 0; i < policyWrappers->Length; i++)
		{
			policyWrappers[i] = gcnew DefaultPolicyWrapper<int>(defaultPolicyFuncs[i]);
			spDelegates[i] = gcnew InternalStatefulPolicyDelegate(&MwtExplorer::BaggingStatefulPolicy);

			BaggingParameter bp;
			bp.Mwt = this;
			bp.BagIndex = i;

			baggingParameters[i] = (IntPtr)GCHandle::Alloc(bp);
		}

		this->InitializeBagging(bags, spDelegates, baggingParameters, numActions);
	}

	generic <class T>
	void MwtExplorer::InitializeSoftmax(float lambda, StatefulScorerDelegate<T>^ defaultScorerFunc, T defaultScorerFuncParams, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<T>(defaultScorerFunc, defaultScorerFuncParams);
		selfHandle = GCHandle::Alloc(this);
		
		InternalStatefulScorerDelegate^ ssDelegate = gcnew InternalStatefulScorerDelegate(&MwtExplorer::InternalScorerFunction);

		this->InitializeSoftmax(lambda, ssDelegate, (IntPtr)selfHandle, numActions);
	}

	void MwtExplorer::InitializeSoftmax(float lambda, StatelessScorerDelegate^ defaultScorerFunc, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<int>(defaultScorerFunc);
		selfHandle = GCHandle::Alloc(this);

		InternalStatefulScorerDelegate^ ssDelegate = gcnew InternalStatefulScorerDelegate(&MwtExplorer::InternalScorerFunction);

		this->InitializeSoftmax(lambda, ssDelegate, (IntPtr)selfHandle, numActions);
	}

	generic <class T>
	void MwtExplorer::InitializeGeneric(StatefulScorerDelegate<T>^ defaultScorerFunc, T defaultScorerFuncParams, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<T>(defaultScorerFunc, defaultScorerFuncParams);
		selfHandle = GCHandle::Alloc(this);

		InternalStatefulScorerDelegate^ ssDelegate = gcnew InternalStatefulScorerDelegate(&MwtExplorer::InternalScorerFunction);

		this->InitializeGeneric(ssDelegate, (IntPtr)selfHandle, numActions);
	}

	void MwtExplorer::InitializeGeneric(StatelessScorerDelegate^ defaultScorerFunc, UInt32 numActions)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<int>(defaultScorerFunc);
		selfHandle = GCHandle::Alloc(this);

		InternalStatefulScorerDelegate^ ssDelegate = gcnew InternalStatefulScorerDelegate(&MwtExplorer::InternalScorerFunction);

		this->InitializeGeneric(ssDelegate, (IntPtr)selfHandle, numActions);
	}

	void MwtExplorer::InitializeEpsilonGreedy(float epsilon, InternalStatefulPolicyDelegate^ defaultPolicyFunc, System::IntPtr defaultPolicyFuncContext, UInt32 numActions)
	{
		policyFuncHandle = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateful_Policy_Func* nativeFunc = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
		m_mwt->Internal_Initialize_Epsilon_Greedy(epsilon, nativeFunc, defaultPolicyFuncContext.ToPointer(), numActions);
	}

	void MwtExplorer::InitializeTauFirst(UInt32 tau, InternalStatefulPolicyDelegate^ defaultPolicyFunc, System::IntPtr defaultPolicyFuncContext, UInt32 numActions)
	{
		policyFuncHandle = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateful_Policy_Func* nativeFunc = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
		m_mwt->Internal_Initialize_Tau_First(tau, nativeFunc, defaultPolicyFuncContext.ToPointer(), numActions);
	}

	void MwtExplorer::InitializeBagging(UInt32 bags, cli::array<InternalStatefulPolicyDelegate^>^ defaultPolicyFuncs, cli::array<IntPtr>^ defaultPolicyArgs, UInt32 numActions)
	{
		baggingFuncHandles = gcnew cli::array<GCHandle>(bags);

		m_bagging_funcs = new Stateful_Policy_Func*[bags];
		m_bagging_func_params = new void*[bags];

		for (int i = 0; i < bags; i++)
		{
			baggingFuncHandles[i] = GCHandle::Alloc(defaultPolicyFuncs[i]);
			IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFuncs[i]);

			m_bagging_funcs[i] = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
			m_bagging_func_params[i] = defaultPolicyArgs[i].ToPointer();
		}

		m_mwt->Internal_Initialize_Bagging(bags, m_bagging_funcs, m_bagging_func_params, numActions);
	}

	void MwtExplorer::InitializeSoftmax(float lambda, InternalStatefulScorerDelegate^ defaultScorerFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions)
	{
		policyFuncHandle = GCHandle::Alloc(defaultScorerFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultScorerFunc);

		Stateful_Scorer_Func* nativeFunc = static_cast<Stateful_Scorer_Func*>(ip.ToPointer());
		m_mwt->Internal_Initialize_Softmax(lambda, nativeFunc, defaultPolicyFuncContext.ToPointer(), numActions);
	}

	void MwtExplorer::InitializeGeneric(InternalStatefulScorerDelegate^ defaultScorerFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions)
	{
		policyFuncHandle = GCHandle::Alloc(defaultScorerFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultScorerFunc);

		Stateful_Scorer_Func* nativeFunc = static_cast<Stateful_Scorer_Func*>(ip.ToPointer());
		m_mwt->Internal_Initialize_Generic(nativeFunc, defaultPolicyFuncContext.ToPointer(), numActions);
	}

	UInt32 MwtExplorer::ChooseAction(String^ uniqueId, Context^ context)
	{
		UInt32 chosenAction = 0;

		GCHandle contextHandle = GCHandle::Alloc(context);
		IntPtr contextPtr = (IntPtr)contextHandle;

		std::string nativeUniqueKey = marshal_as<std::string>(uniqueId);

		NativeMultiWorldTesting::Context* log_context = MwtHelper::PinNativeContext(context);

		size_t uniqueIdLength = (size_t)uniqueId->Length;

		chosenAction = m_mwt->Internal_Choose_Action(
			contextPtr.ToPointer(),
			nativeUniqueKey, 
			*log_context);

		//TODO: This leaks memory. The problem is we now need contexts to stay alive for duration of MWTExplorer
		//since we are not (deep) copying them anymore in Internal_Choose_Action.
		//contextHandle.Free();
		//delete log_context;

		return chosenAction;
	}

	String^ MwtExplorer::GetAllInteractionsAsString()
	{
		std::string all_interactions = m_mwt->Get_All_Interactions();
		return gcnew String(all_interactions.c_str());
	}

	cli::array<Interaction^>^ MwtExplorer::GetAllInteractions()
	{
		size_t num_interactions = 0;
		NativeMultiWorldTesting::Interaction** native_interactions = nullptr;
		m_mwt->Get_All_Interactions(num_interactions, native_interactions);

		cli::array<Interaction^>^ interactions = gcnew cli::array<Interaction^>((int)num_interactions);
		if (num_interactions > 0 && native_interactions != nullptr)
		{
			for (size_t i = 0; i < num_interactions; i++)
			{
				interactions[i] = gcnew Interaction();

				//TODO: We're casting a BaseContext object to a derived type (Context) for now, but we actually
				//need is a definition of the BaseContext interface in C# land.
				NativeMultiWorldTesting::Context* native_context = (NativeMultiWorldTesting::Context*)native_interactions[i]->Get_Context();

				NativeMultiWorldTesting::Feature* native_features = nullptr;
				size_t native_num_features = 0;
				native_context->Get_Features(native_num_features, native_features);
				cli::array<Feature>^ features = gcnew cli::array<Feature>((int)native_num_features);
				for (int i = 0; i < features->Length; i++)
				{
					features[i].Value = native_features[i].Value;
					features[i].Id = native_features[i].Id;
				}

				std::string native_other_context;
				native_context->Get_Other_Context(native_other_context);
				String^ otherContext = (native_other_context.empty()) ? nullptr : gcnew String(native_other_context.c_str());

				interactions[i]->ApplicationContext = gcnew Context(features, otherContext);
				interactions[i]->ChosenAction = native_interactions[i]->Get_Action().Get_Id();
				interactions[i]->Probability = native_interactions[i]->Get_Prob();
				interactions[i]->Id = gcnew String(native_interactions[i]->Get_Id().c_str());
				interactions[i]->IdHash = native_interactions[i]->Get_Id_Hash();

				delete native_interactions[i];
			}
			delete[] native_interactions;
		}

		return interactions;
	}

	UInt32 MwtExplorer::InvokeDefaultPolicyFunction(Context^ context)
	{
		return policyWrapper->InvokeFunction(context);
	}

	UInt32 MwtExplorer::InvokeBaggingDefaultPolicyFunction(Context^ context, int bagIndex)
	{
		return policyWrappers[bagIndex]->InvokeFunction(context);
	}

	void MwtExplorer::InvokeDefaultScorerFunction(Context^ context, cli::array<float>^ scores)
	{
		policyWrapper->InvokeScorer(context, scores);
	}

	UInt32 MwtExplorer::InternalStatefulPolicy(IntPtr mwtPtr, IntPtr contextPtr)
	{
		GCHandle mwtHandle = (GCHandle)mwtPtr;
		MwtExplorer^ mwt = (MwtExplorer^)(mwtHandle.Target);

		GCHandle contextHandle = (GCHandle)contextPtr;
		Context^ context = (Context^)(contextHandle.Target);

		return mwt->InvokeDefaultPolicyFunction(context);
	}

	UInt32 MwtExplorer::BaggingStatefulPolicy(IntPtr baggingParamPtr, IntPtr contextPtr)
	{
		GCHandle mwtHandle = (GCHandle)baggingParamPtr;
		BaggingParameter bp = (BaggingParameter)(mwtHandle.Target);

		GCHandle contextHandle = (GCHandle)contextPtr;
		Context^ context = (Context^)(contextHandle.Target);

		return bp.Mwt->InvokeBaggingDefaultPolicyFunction(context, bp.BagIndex);
	}

	void MwtExplorer::InternalScorerFunction(IntPtr mwtPtr, IntPtr contextPtr, IntPtr scoresPtr, UInt32 numScores)
	{
		GCHandle mwtHandle = (GCHandle)mwtPtr;
		MwtExplorer^ mwt = (MwtExplorer^)(mwtHandle.Target);

		GCHandle contextHandle = (GCHandle)contextPtr;
		Context^ context = (Context^)(contextHandle.Target);

		cli::array<float>^ scores = gcnew cli::array<float>(numScores);

		mwt->InvokeDefaultScorerFunction(context, scores);

		float* native_scores = (float*)scoresPtr.ToPointer();
		for (int i = 0; i < numScores; i++)
		{
			native_scores[i] = scores[i];
		}
	}

	MwtRewardReporter::MwtRewardReporter(cli::array<Interaction^>^ interactions)
	{
		m_num_native_interactions = interactions->Length;
		m_native_interactions = new NativeMultiWorldTesting::Interaction*[m_num_native_interactions];
		for (int i = 0; i < m_num_native_interactions; i++)
		{
			NativeMultiWorldTesting::Context* native_context = MwtHelper::PinNativeContext(interactions[i]->ApplicationContext);
			
			String^ interaction_id = interactions[i]->Id;
			m_native_interactions[i] = new NativeMultiWorldTesting::Interaction(native_context,
				interactions[i]->ChosenAction,
				interactions[i]->Probability,
				marshal_as<std::string>(interaction_id),
				/* is_copy = */ true);
		}
		size_t native_num_interactions = (size_t)m_num_native_interactions;
		m_mwt_reward_reporter = new NativeMultiWorldTesting::MWTRewardReporter(native_num_interactions, m_native_interactions);
	}

	MwtRewardReporter::~MwtRewardReporter()
	{
		for (int i = 0; i < m_num_native_interactions; i++)
		{
			delete m_native_interactions[i];
		}
		delete[] m_native_interactions;
		delete m_mwt_reward_reporter;
	}

	bool MwtRewardReporter::ReportReward(String^ id, float reward)
	{
		return m_mwt_reward_reporter->Report_Reward(marshal_as<std::string>(id), reward);
	}

	bool MwtRewardReporter::ReportReward(cli::array<String^>^ ids, cli::array<float>^ rewards)
	{
		pin_ptr<float> pinnedRewards = &rewards[0];

		std::string* native_ids = new std::string[ids->Length];
		for (int i = 0; i < ids->Length; i++)
		{
			String^ interaction_id = ids[i];
			native_ids[i] = marshal_as<std::string>(interaction_id);
		}
		pin_ptr<std::string> pinnedIds = &native_ids[0];

		return m_mwt_reward_reporter->Report_Reward((size_t)ids->Length, (std::string*)pinnedIds, (float*)pinnedRewards);
	}
	
	String^ MwtRewardReporter::GetAllInteractionsAsString()
	{
		return gcnew String(m_mwt_reward_reporter->Get_All_Interactions().c_str());
	}

	//SIDTEMP:
	cli::array<Interaction^>^ MwtRewardReporter::GetAllInteractions()
	{
		cli::array<Interaction^>^ interactions = gcnew cli::array<Interaction^>((int)m_num_native_interactions);
		if (m_num_native_interactions > 0 && m_native_interactions != nullptr)
		{
			for (size_t i = 0; i < m_num_native_interactions; i++)
			{
				interactions[i] = gcnew Interaction();

				NativeMultiWorldTesting::Context* native_context = (NativeMultiWorldTesting::Context*)m_native_interactions[i]->Get_Context();

				NativeMultiWorldTesting::Feature* native_features = nullptr;
				size_t native_num_features = 0;
				native_context->Get_Features(native_num_features, native_features);
				cli::array<Feature>^ features = gcnew cli::array<Feature>((int)native_num_features);
				for (int i = 0; i < features->Length; i++)
				{
					features[i].Value = native_features[i].Value;
					features[i].Id = native_features[i].Id;
				}

				std::string native_other_context;
				native_context->Get_Other_Context(native_other_context);
				String^ otherContext = (native_other_context.empty()) ? nullptr : gcnew String(native_other_context.c_str());

				interactions[i]->ApplicationContext = gcnew Context(features, otherContext);
				interactions[i]->ChosenAction = m_native_interactions[i]->Get_Action().Get_Id();
				interactions[i]->Probability = m_native_interactions[i]->Get_Prob();
				interactions[i]->Id = gcnew String(m_native_interactions[i]->Get_Id().c_str());
				interactions[i]->IdHash = m_native_interactions[i]->Get_Id_Hash();
				interactions[i]->Reward = m_native_interactions[i]->Get_Reward();
			}
		}

		return interactions;
	}

	MwtOptimizer::MwtOptimizer(cli::array<Interaction^>^ interactions, UInt32 numActions)
	{
		m_num_native_interactions = interactions->Length;
		m_native_interactions = new NativeMultiWorldTesting::Interaction*[m_num_native_interactions];
		contextHandles = gcnew cli::array<GCHandle>(m_num_native_interactions);
		for (int i = 0; i < m_num_native_interactions; i++)
		{
			NativeMultiWorldTesting::Context* native_context = MwtHelper::PinNativeContext(interactions[i]->ApplicationContext);
			String^ interaction_id = interactions[i]->Id;
			m_native_interactions[i] = new NativeMultiWorldTesting::Interaction(native_context,
				interactions[i]->ChosenAction,
				interactions[i]->Probability,
				marshal_as<std::string>(interaction_id),
				/* is_copy = */ true);
			m_native_interactions[i]->Set_Reward(interactions[i]->Reward);

			//SIDTEMP: Pass in the C# pointer since this class only uses it to pass back to a default
			//policy during offlineevaluation
			contextHandles[i] = GCHandle::Alloc(interactions[i]->ApplicationContext);
			IntPtr contextPtr = (IntPtr)contextHandles[i];
			m_native_interactions[i]->Set_External_Context(contextPtr.ToPointer());
		}
		size_t native_num_interactions = (size_t)m_num_native_interactions;
		m_mwt_optimizer = new NativeMultiWorldTesting::MWTOptimizer(native_num_interactions, m_native_interactions, (u32)numActions);
	}

	MwtOptimizer::~MwtOptimizer()
	{
		this->Uninitialize();
	}

	void MwtOptimizer::Uninitialize()
	{
		selfHandle.Free();

		if (contextHandles != nullptr)
		{
			for (int i = 0; i < contextHandles->Length; i++)
			{
				if (contextHandles[i].IsAllocated)
				{
					contextHandles[i].Free();
				}
			}
		}

		for (int i = 0; i < m_num_native_interactions; i++)
		{
			delete m_native_interactions[i];
		}
		delete[] m_native_interactions;
		delete m_mwt_optimizer;
	}

	generic <class T>
	float MwtOptimizer::EvaluatePolicy(StatefulPolicyDelegate<T>^ policyFunc, T policyParams)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<T>(policyFunc, policyParams);
		selfHandle = GCHandle::Alloc(this);
		InternalStatefulPolicyDelegate^ spDelegate = gcnew InternalStatefulPolicyDelegate(&MwtOptimizer::InternalStatefulPolicy);

		return this->EvaluatePolicy(spDelegate, (IntPtr)selfHandle);
	}

	float MwtOptimizer::EvaluatePolicy(StatelessPolicyDelegate^ policy_func)
	{
		policyWrapper = gcnew DefaultPolicyWrapper<int>(policy_func);
		selfHandle = GCHandle::Alloc(this);
		InternalStatefulPolicyDelegate^ spDelegate = gcnew InternalStatefulPolicyDelegate(&MwtOptimizer::InternalStatefulPolicy);

		return this->EvaluatePolicy(spDelegate, (IntPtr)selfHandle);
	}

	float MwtOptimizer::EvaluatePolicyVWCSOAA(String^ model_input_file)
	{
		return m_mwt_optimizer->Evaluate_Policy_VW_CSOAA(marshal_as<std::string>(model_input_file));
	}

	void MwtOptimizer::OptimizePolicyVWCSOAA(String^ model_output_file)
	{
		m_mwt_optimizer->Optimize_Policy_VW_CSOAA(marshal_as<std::string>(model_output_file));
	}

	float MwtOptimizer::EvaluatePolicy(InternalStatefulPolicyDelegate^ policyFunc, IntPtr policyParams)
	{
		GCHandle gch = GCHandle::Alloc(policyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(policyFunc);

		Stateful_Policy_Func* nativeFunc = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
		float value = m_mwt_optimizer->Internal_Evaluate_Policy(nativeFunc, policyParams.ToPointer());

		gch.Free();

		return value;
	}

	UInt32 MwtOptimizer::InvokeDefaultPolicyFunction(Context^ context)
	{
		return policyWrapper->InvokeFunction(context);
	}

	UInt32 MwtOptimizer::InternalStatefulPolicy(IntPtr mwtOptimizerPtr, IntPtr contextPtr)
	{
		GCHandle mwtHandle = (GCHandle)mwtOptimizerPtr;
		MwtOptimizer^ mwtOpt = (MwtOptimizer^)(mwtHandle.Target);

		GCHandle contextHandle = (GCHandle)contextPtr;
		Context^ context = (Context^)(contextHandle.Target);

		return mwtOpt->InvokeDefaultPolicyFunction(context);
	}

	NativeMultiWorldTesting::Context* MwtHelper::PinNativeContext(Context^ context)
	{
		cli::array<Feature>^ contextFeatures = context->Features;
		String^ otherContext = context->OtherContext;

		context->FeatureHandle = GCHandle::Alloc(context->Features, GCHandleType::Pinned);
		try
		{
			IntPtr featureArrayPtr = context->FeatureHandle.AddrOfPinnedObject();

			NativeMultiWorldTesting::Feature* nativeContextFeatures = (NativeMultiWorldTesting::Feature*)featureArrayPtr.ToPointer();

			if (otherContext != nullptr)
			{
				return new NativeMultiWorldTesting::Context((NativeMultiWorldTesting::Feature*)nativeContextFeatures, (size_t)context->Features->Length, marshal_as<std::string>(otherContext));
			}
			else
			{
				return new NativeMultiWorldTesting::Context((NativeMultiWorldTesting::Feature*)nativeContextFeatures, (size_t)context->Features->Length);
			}
		}
		catch (Exception^ ex)
		{
			context->FeatureHandle.Free();
			throw ex;
		}
	}
}
