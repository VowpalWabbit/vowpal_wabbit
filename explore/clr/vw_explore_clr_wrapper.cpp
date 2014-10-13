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

	void MwtExplorer::InitializeEpsilonGreedy(float epsilon, InternalStatefulPolicyDelegate^ defaultPolicyFunc, System::IntPtr defaultPolicyFuncContext, UInt32 numActions)
	{
		GCHandle gch = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateful_Policy_Func* nativeFunc = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
		m_mwt->Initialize_Epsilon_Greedy(epsilon, nativeFunc, defaultPolicyFuncContext.ToPointer(), numActions);

		gch.Free();
	}

	void MwtExplorer::InitializeTauFirst(UInt32 tau, InternalStatefulPolicyDelegate^ defaultPolicyFunc, System::IntPtr defaultPolicyFuncContext, UInt32 numActions)
	{
		GCHandle gch = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateful_Policy_Func* nativeFunc = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
		m_mwt->Initialize_Tau_First(tau, nativeFunc, defaultPolicyFuncContext.ToPointer(), numActions);

		gch.Free();
	}

	void MwtExplorer::InitializeBagging(UInt32 bags, cli::array<InternalStatefulPolicyDelegate^>^ defaultPolicyFuncs, cli::array<IntPtr>^ defaultPolicyArgs, UInt32 numActions)
	{
		cli::array<GCHandle>^ gcHandles = gcnew cli::array<GCHandle>(bags);

		m_bagging_funcs = new Stateful_Policy_Func*[bags];
		m_bagging_func_params = new void*[bags];

		for (int i = 0; i < bags; i++)
		{
			gcHandles[i] = GCHandle::Alloc(defaultPolicyFuncs[i]);
			IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFuncs[i]);

			m_bagging_funcs[i] = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
			m_bagging_func_params[i] = defaultPolicyArgs[i].ToPointer();
		}

		m_mwt->Initialize_Bagging(bags, m_bagging_funcs, m_bagging_func_params, numActions);

		for (int i = 0; i < bags; i++)
		{
			gcHandles[i].Free();
		}
	}

	void MwtExplorer::InitializeSoftmax(float lambda, InternalStatefulScorerDelegate^ defaultScorerFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions)
	{
		GCHandle gch = GCHandle::Alloc(defaultScorerFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultScorerFunc);

		Stateful_Scorer_Func* nativeFunc = static_cast<Stateful_Scorer_Func*>(ip.ToPointer());
		m_mwt->Initialize_Softmax(lambda, nativeFunc, defaultPolicyFuncContext.ToPointer(), numActions);

		gch.Free();
	}

	UInt32 MwtExplorer::ChooseAction(CONTEXT^ context, String^ uniqueId)
	{
		UInt32 chosenAction = 0;

		GCHandle contextHandle = GCHandle::Alloc(context);
		IntPtr contextPtr = (IntPtr)contextHandle;

		std::string nativeUniqueKey = marshal_as<std::string>(uniqueId);

		Context* log_context = MwtHelper::ToNativeContext(context);

		size_t uniqueIdLength = (size_t)uniqueId->Length;

		chosenAction = m_mwt->Choose_Action(
			contextPtr.ToPointer(),
			nativeUniqueKey, 
			*log_context);

		contextHandle.Free();
		delete log_context;

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
				interactions[i]->Id = gcnew String(native_interactions[i]->Get_Id().c_str());
				interactions[i]->IdHash = native_interactions[i]->Get_Id_Hash();

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

		cli::array<float>^ scores = gcnew cli::array<float>(numScores);

		mwt->InvokeDefaultScorerFunction(context, scores);

		float* native_scores = (float*)scoresPtr.ToPointer();
		for (int i = 0; i < numScores; i++)
		{
			native_scores[i] = scores[i];
		}
	}

	MwtRewardReporter::MwtRewardReporter(cli::array<INTERACTION^>^ interactions)
	{
		m_num_native_interactions = interactions->Length;
		m_native_interactions = new Interaction*[m_num_native_interactions];
		for (int i = 0; i < m_num_native_interactions; i++)
		{
			Context* native_context = MwtHelper::ToNativeContext(interactions[i]->ApplicationContext);
			
			String^ interaction_id = interactions[i]->Id;
			m_native_interactions[i] = new Interaction(native_context,
				interactions[i]->ChosenAction,
				interactions[i]->Probability,
				marshal_as<std::string>(interaction_id));
		}
		size_t native_num_interactions = (size_t)m_num_native_interactions;
		m_mwt_reward_reporter = new MWTRewardReporter(native_num_interactions, m_native_interactions);
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
		return m_mwt_reward_reporter->ReportReward(marshal_as<std::string>(id), reward);
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

		return m_mwt_reward_reporter->ReportReward((size_t)ids->Length, (std::string*)pinnedIds, (float*)pinnedRewards);
	}
	
	String^ MwtRewardReporter::GetAllInteractions()
	{
		return gcnew String(m_mwt_reward_reporter->Get_All_Interactions().c_str());
	}

	MwtOptimizer::MwtOptimizer(cli::array<INTERACTION^>^ interactions, UInt32 numActions)
	{
		m_num_native_interactions = interactions->Length;
		m_native_interactions = new Interaction*[m_num_native_interactions];
		for (int i = 0; i < m_num_native_interactions; i++)
		{
			Context* native_context = MwtHelper::ToNativeContext(interactions[i]->ApplicationContext);
			String^ interaction_id = interactions[i]->Id;
			m_native_interactions[i] = new Interaction(native_context,
				interactions[i]->ChosenAction,
				interactions[i]->Probability,
				marshal_as<std::string>(interaction_id));
		}
		size_t native_num_interactions = (size_t)m_num_native_interactions;
		m_mwt_optimizer = new MWTOptimizer(native_num_interactions, m_native_interactions, (u32)numActions);
	}

	MwtOptimizer::~MwtOptimizer()
	{
		this->Uninitialize();
	}

	void MwtOptimizer::Uninitialize()
	{
		selfHandle.Free();

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
		float value = m_mwt_optimizer->Evaluate_Policy(nativeFunc, (void*)policyParams.ToPointer());

		gch.Free();

		return value;
	}

	UInt32 MwtOptimizer::InvokeDefaultPolicyFunction(CONTEXT^ context)
	{
		return policyWrapper->InvokeFunction(context);
	}

	UInt32 MwtOptimizer::InternalStatefulPolicy(IntPtr mwtOptimizerPtr, IntPtr contextPtr)
	{
		GCHandle mwtHandle = (GCHandle)mwtOptimizerPtr;
		MwtOptimizer^ mwtOpt = (MwtOptimizer^)(mwtHandle.Target);

		GCHandle contextHandle = (GCHandle)contextPtr;
		CONTEXT^ context = (CONTEXT^)(contextHandle.Target);

		return mwtOpt->InvokeDefaultPolicyFunction(context);
	}

	Context* MwtHelper::ToNativeContext(CONTEXT^ context)
	{
		cli::array<FEATURE>^ contextFeatures = context->Features;
		String^ otherContext = context->OtherContext;

		std::string* nativeOtherContext = (otherContext != nullptr) ? &marshal_as<std::string>(otherContext) : nullptr;

		pin_ptr<FEATURE> pinnedContextFeatures = &context->Features[0];
		FEATURE* nativeContextFeatures = pinnedContextFeatures;

		return new Context((feature*)nativeContextFeatures, (size_t)context->Features->Length, nativeOtherContext);
	}
}