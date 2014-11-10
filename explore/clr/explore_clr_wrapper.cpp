// vw_explore_clr_wrapper.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "explore_clr_wrapper.h"

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;
using namespace System::Runtime::InteropServices;
using namespace msclr::interop;
using namespace NativeMultiWorldTesting;

namespace MultiWorldTesting {

	MwtRewardReporter::MwtRewardReporter(cli::array<Interaction^>^ interactions)
	{
		m_num_native_interactions = interactions->Length;
		m_native_interactions = new NativeMultiWorldTesting::Interaction*[m_num_native_interactions];
		for (int i = 0; i < m_num_native_interactions; i++)
		{
			NativeMultiWorldTesting::OldSimpleContext* native_context = MwtHelper::PinNativeContext(interactions[i]->ApplicationContext);
			
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

				NativeMultiWorldTesting::OldSimpleContext* native_context = (NativeMultiWorldTesting::OldSimpleContext*)m_native_interactions[i]->Get_Context();

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

				interactions[i]->ApplicationContext = gcnew OldSimpleContext(features, otherContext);
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
			NativeMultiWorldTesting::OldSimpleContext* native_context = MwtHelper::PinNativeContext(interactions[i]->ApplicationContext);
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
			m_native_interactions[i]->Set_Clr_Context(contextPtr.ToPointer());
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

	UInt32 MwtOptimizer::InvokeDefaultPolicyFunction(OldSimpleContext^ context)
	{
		return policyWrapper->InvokeFunction(context);
	}

	UInt32 MwtOptimizer::InternalStatefulPolicy(IntPtr mwtOptimizerPtr, IntPtr contextPtr)
	{
		GCHandle mwtHandle = (GCHandle)mwtOptimizerPtr;
		MwtOptimizer^ mwtOpt = (MwtOptimizer^)(mwtHandle.Target);

		GCHandle contextHandle = (GCHandle)contextPtr;
		OldSimpleContext^ context = (OldSimpleContext^)(contextHandle.Target);

		return mwtOpt->InvokeDefaultPolicyFunction(context);
	}

	NativeMultiWorldTesting::OldSimpleContext* MwtHelper::PinNativeContext(BaseContext^ context)
	{
		cli::array<Feature>^ contextFeatures = context->GetFeatures();

		if (contextFeatures == nullptr)
		{
			throw gcnew InvalidDataException("Context features cannot be null.");
		}

		context->FeatureHandle = GCHandle::Alloc(contextFeatures, GCHandleType::Pinned);
		try
		{
			IntPtr featureArrayPtr = context->FeatureHandle.AddrOfPinnedObject();

			NativeMultiWorldTesting::Feature* nativeContextFeatures = (NativeMultiWorldTesting::Feature*)featureArrayPtr.ToPointer();

			return new NativeMultiWorldTesting::OldSimpleContext((NativeMultiWorldTesting::Feature*)nativeContextFeatures, (size_t)contextFeatures->Length);
		}
		catch (Exception^ ex)
		{
			context->FeatureHandle.Free();
			throw ex;
		}
	}
}
