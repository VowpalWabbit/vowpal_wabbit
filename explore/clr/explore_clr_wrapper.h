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

namespace MultiWorldTesting {

	[StructLayout(LayoutKind::Sequential)]
	public value struct Feature
	{
		float Value;
		UInt32 Id;
	};

	public ref class BaseContext
	{
	public:
		virtual cli::array<Feature>^ GetFeatures() abstract;

		~BaseContext()
		{
			if (FeatureHandle.IsAllocated)
			{
				FeatureHandle.Free();
			}
		}

	internal:
		GCHandle FeatureHandle;
	};

	public ref class OldSimpleContext : public BaseContext
	{
	public:
		OldSimpleContext()
		{
			Features = nullptr;
			OtherContext = nullptr;
		}

		OldSimpleContext(cli::array<Feature>^ features, String^ otherContext)
		{
			Features = features;
			OtherContext = otherContext;
		}

	public:
		cli::array<Feature>^ GetFeatures() override { return Features; }
		String^ GetOtherContext() { return OtherContext; }

	internal:
		cli::array<Feature>^ Features;
		String^ OtherContext;
	};

	public ref class Interaction
	{
	public:
		String^ GetId() { return Id; }
		UInt64^ GetIdHash() { return IdHash; }
		UInt32 GetAction() { return ChosenAction; }
		float GetProbability() { return Probability; }
		BaseContext^ GetContext() { return ApplicationContext; }
		float GetReward() { return Reward; }
		void SetReward(float reward) { Reward = reward; }

	internal:
		BaseContext^ ApplicationContext;
		UInt32 ChosenAction;
		float Probability;
		float Reward;
		String^ Id;
		UInt64 IdHash;
	};

	public ref class ActionID
	{
	public:
		static UInt32 Make_OneBased(UInt32 id) { return NativeMultiWorldTesting::MWTAction::Make_OneBased(id); }
		static UInt32 Make_ZeroBased(UInt32 id) { return NativeMultiWorldTesting::MWTAction::Make_ZeroBased(id); }
	};

	generic <class T>
	public delegate UInt32 StatefulPolicyDelegate(T, BaseContext^);
	public delegate UInt32 StatelessPolicyDelegate(BaseContext^);

	generic <class T>
	public delegate void StatefulScorerDelegate(T, BaseContext^, cli::array<float>^ scores);
	public delegate void StatelessScorerDelegate(BaseContext^, cli::array<float>^ scores);

	// Internal delegate denifition
	private delegate UInt32 InternalStatefulPolicyDelegate(IntPtr, IntPtr);
	private delegate void InternalStatefulScorerDelegate(IntPtr, IntPtr, IntPtr scores, UInt32 size);

	interface class IFunctionWrapper
	{
		public:
			virtual UInt32 InvokeFunction(BaseContext^) abstract;
			virtual void InvokeScorer(BaseContext^, cli::array<float>^) abstract;
	};

	generic <class T>
	public ref class DefaultPolicyWrapper : IFunctionWrapper
	{
		public:
			DefaultPolicyWrapper(StatefulPolicyDelegate<T>^ policyFunc, T policyParams)
			{
				defaultPolicy = policyFunc;
				parameters = policyParams;
			}

			DefaultPolicyWrapper(StatefulScorerDelegate<T>^ scorerFunc, T policyParams)
			{
				defaultScorer = scorerFunc;
				parameters = policyParams;
			}

			DefaultPolicyWrapper(StatelessPolicyDelegate^ policyFunc)
			{
				statelessPolicy = policyFunc;
			}

			DefaultPolicyWrapper(StatelessScorerDelegate^ scorerFunc)
			{
				statelessScorer = scorerFunc;
			}

			virtual UInt32 InvokeFunction(BaseContext^ c) override
			{
				if (defaultPolicy != nullptr)
				{
					return defaultPolicy(parameters, c);
				}
				else
				{
					return statelessPolicy(c);
				}
			}

			virtual void InvokeScorer(BaseContext^ c, cli::array<float>^ scores) override
			{
				if (defaultScorer != nullptr)
				{
					defaultScorer(parameters, c, scores);
				}
				else
				{
					statelessScorer(c, scores);
				}
			}
		private:
			T parameters;
			StatefulPolicyDelegate<T>^ defaultPolicy;
			StatelessPolicyDelegate^ statelessPolicy;
			StatefulScorerDelegate<T>^ defaultScorer;
			StatelessScorerDelegate^ statelessScorer;
	};

	private ref class MwtHelper
	{
	public:
		static NativeMultiWorldTesting::OldSimpleContext* PinNativeContext(BaseContext^ context);
	};

	public ref class MwtExplorer
	{
	private:
		NativeMultiWorldTesting::MWTExplorer* m_mwt;
		IFunctionWrapper^ policyWrapper;
		cli::array<IFunctionWrapper^>^ policyWrappers;

		// Garbage-collector handles to keep alive
		GCHandle selfHandle;
		GCHandle policyFuncHandle;
		List<GCHandle>^ contextHandles;
		cli::array<GCHandle>^ baggingFuncHandles;

		// Bagging-specific
		cli::array<IntPtr>^ baggingParameters;
		NativeMultiWorldTesting::Stateful_Policy_Func** m_bagging_funcs;
		void** m_bagging_func_params;

		List<IntPtr>^ nativeContexts;

	public:
		MwtExplorer(String^ app_id);
		~MwtExplorer();

		generic <class T>
		void InitializeEpsilonGreedy(float epsilon, StatefulPolicyDelegate<T>^ defaultPolicyFunc, T defaultPolicyFuncParams, UInt32 numActions);
		void InitializeEpsilonGreedy(float epsilon, StatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions);

		generic <class T>
		void InitializeTauFirst(UInt32 tau, StatefulPolicyDelegate<T>^ defaultPolicyFunc, T defaultPolicyFuncParams, UInt32 numActions);
		void InitializeTauFirst(UInt32 tau, StatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions);

		generic <class T>
		void InitializeBagging(UInt32 bags, cli::array<StatefulPolicyDelegate<T>^>^ defaultPolicyFuncs, cli::array<T>^ defaultPolicyArgs, UInt32 numActions);
		void InitializeBagging(UInt32 bags, cli::array<StatelessPolicyDelegate^>^ defaultPolicyFuncs, UInt32 numActions);

		generic <class T>
		void InitializeSoftmax(float lambda, StatefulScorerDelegate<T>^ defaultScorerFunc, T defaultScorerFuncParams, UInt32 numActions);
		void InitializeSoftmax(float lambda, StatelessScorerDelegate^ defaultScorerFunc, UInt32 numActions);

		generic <class T>
		void InitializeGeneric(StatefulScorerDelegate<T>^ defaultScorerFunc, T defaultScorerFuncParams, UInt32 numActions);
		void InitializeGeneric(StatelessScorerDelegate^ defaultScorerFunc, UInt32 numActions);

		void Uninitialize();

		UInt32 ChooseAction(String^ uniqueId, BaseContext^ context);

		String^ GetAllInteractionsAsString();
		cli::array<Interaction^>^ GetAllInteractions();

	internal:
		UInt32 InvokeDefaultPolicyFunction(BaseContext^);
		UInt32 InvokeBaggingDefaultPolicyFunction(BaseContext^, int);
		void InvokeDefaultScorerFunction(BaseContext^, cli::array<float>^);

	private: // Internal Initialize APIs
		void InitializeEpsilonGreedy(float epsilon, InternalStatefulPolicyDelegate^ defaultPolicyFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);
		void InitializeTauFirst(UInt32 tau, InternalStatefulPolicyDelegate^ defaultPolicyFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);
		void InitializeBagging(UInt32 bags, cli::array<InternalStatefulPolicyDelegate^>^ defaultPolicyFuncs, cli::array<IntPtr>^ defaultPolicyArgs, UInt32 numActions);
		void InitializeSoftmax(float lambda, InternalStatefulScorerDelegate^ defaultScorerFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);
		void InitializeGeneric(InternalStatefulScorerDelegate^ defaultScorerFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);

	private: // Internal Callback Methods
		static UInt32 InternalStatefulPolicy(IntPtr, IntPtr);
		static UInt32 BaggingStatefulPolicy(IntPtr, IntPtr);
		static void InternalScorerFunction(IntPtr, IntPtr, IntPtr, UInt32);
	};

	private value struct BaggingParameter
	{
	public:
		MwtExplorer^ Mwt;
		int BagIndex;
	};

	public ref class MwtRewardReporter
	{
	private:
		NativeMultiWorldTesting::MWTRewardReporter* m_mwt_reward_reporter;
		NativeMultiWorldTesting::Interaction** m_native_interactions;
		int m_num_native_interactions;

	public:
		MwtRewardReporter(cli::array<Interaction^>^ interactions);
		~MwtRewardReporter();

		bool ReportReward(String^ id, float reward);
		bool ReportReward(cli::array<String^>^ ids, cli::array<float>^ rewards);
		String^ GetAllInteractionsAsString();
		//SIDTEMP:
		cli::array<Interaction^>^ GetAllInteractions();
	};

	public ref class MwtOptimizer
	{
	private:
		NativeMultiWorldTesting::MWTOptimizer* m_mwt_optimizer;
		NativeMultiWorldTesting::Interaction** m_native_interactions;
		int m_num_native_interactions;
		IFunctionWrapper^ policyWrapper;
		GCHandle selfHandle;
		cli::array<GCHandle>^ contextHandles;
		
	public: 
		MwtOptimizer(cli::array<Interaction^>^ interactions, UInt32 numActions);
		~MwtOptimizer();

		generic <class T>
		float EvaluatePolicy(StatefulPolicyDelegate<T>^ policyFunc, T policyParams);
		float EvaluatePolicy(StatelessPolicyDelegate^ policy_func);
		float EvaluatePolicyVWCSOAA(String^ model_input_file);
		void OptimizePolicyVWCSOAA(String^ model_output_file);
		void Uninitialize();

	internal:
		UInt32 InvokeDefaultPolicyFunction(OldSimpleContext^);

	private:
		float EvaluatePolicy(InternalStatefulPolicyDelegate^ policyFunc, IntPtr policyParams);

	private:
		static UInt32 InternalStatefulPolicy(IntPtr, IntPtr);
	};
}
