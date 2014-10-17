#pragma once

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
	public value struct FEATURE
	{
		float X;
		UInt32 WeightIndex;
	};

	public ref class CONTEXT
	{
	public:
		CONTEXT()
		{
			Features = nullptr;
			OtherContext = nullptr;
		}

		CONTEXT(cli::array<FEATURE>^ features, String^ otherContext)
		{
			Features = features;
			OtherContext = otherContext;
		}
	public:
		cli::array<FEATURE>^ Features;
		String^ OtherContext;
	};

	public ref class INTERACTION
	{
	public:
		CONTEXT^ GetContext() { return ApplicationContext; }
		UInt32 GetAction() { return ChosenAction; }
		String^ GetId() { return Id; }
		UInt64 GetIdHash() { return IdHash; }
		float GetProbability() { return Probability; }
		float GetReward() { return Reward; }
		void SetReward(float reward) { Reward = reward; }

	internal:
		CONTEXT^ ApplicationContext;
		UInt32 ChosenAction;
		float Probability;
		float Reward;
		String^ Id;
		UInt64 IdHash;
	};

	public ref class ActionID
	{
	public:
		static UInt32 Make_OneBased(UInt32 id) { return MWTAction::Make_OneBased(id); }
		static UInt32 Make_ZeroBased(UInt32 id) { return MWTAction::Make_ZeroBased(id); }
	};

	generic <class T>
	public delegate UInt32 StatefulPolicyDelegate(T, CONTEXT^);
	public delegate UInt32 StatelessPolicyDelegate(CONTEXT^);

	generic <class T>
	public delegate void StatefulScorerDelegate(T, CONTEXT^, cli::array<float>^ scores);
	public delegate void StatelessScorerDelegate(CONTEXT^, cli::array<float>^ scores);

	// Internal delegate denifition
	private delegate UInt32 InternalStatefulPolicyDelegate(IntPtr, IntPtr);
	private delegate void InternalStatefulScorerDelegate(IntPtr, IntPtr, IntPtr scores, UInt32 size);

	interface class IFunctionWrapper
	{
		public:
			virtual UInt32 InvokeFunction(CONTEXT^) abstract;
			virtual void InvokeScorer(CONTEXT^, cli::array<float>^) abstract;
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

			virtual UInt32 InvokeFunction(CONTEXT^ c) override
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

			virtual void InvokeScorer(CONTEXT^ c, cli::array<float>^ scores) override
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
		static Context* ToNativeContext(CONTEXT^ context);
	};

	generic <class T>
	public ref class MwtBaseLogger
	{
	public:
		MwtBaseLogger(String^ file)
		{
			filePath = file;
			items = gcnew List<T>();
		}

		void Flush()
		{
			if (File::Exists(filePath))
			{
				List<T>^ savedItems = this->ReadFromFile();
				items->AddRange(savedItems);
			}
			this->WriteToFile();
			items->Clear();
		}

	protected:
		void Add(System::Collections::Generic::IEnumerable<T>^ items)
		{
			this->items->AddRange(items);
		}

		void Add(T item)
		{
			items->Add(item);
		}

		cli::array<T>^ GetAllItems()
		{
			if (items->Count == 0 && File::Exists(filePath))
			{
				List<T>^ savedItems = this->ReadFromFile();
				items->AddRange(savedItems);
			}
			return items->ToArray();
		}

	private:
		List<T>^ ReadFromFile()
		{
			XmlSerializer^ serializer = gcnew XmlSerializer(items->GetType());
			StringReader^ reader = gcnew StringReader(File::ReadAllText(filePath));
			return (List<T>^)serializer->Deserialize(reader);
		}

		void WriteToFile()
		{
			if (items->Count > 0)
			{
				XmlSerializer^ serializer = gcnew XmlSerializer(items->GetType());
				StringWriter^ writer = gcnew StringWriter();
				serializer->Serialize(writer, items);
				File::WriteAllText(filePath, writer->ToString());
			}
		}

	private:
		String^ filePath;
		List<T>^ items;
	};

	public ref class MwtLogger : public MwtBaseLogger<INTERACTION^>
	{
	public:
		MwtLogger(String^ file) : MwtBaseLogger<INTERACTION^>(file) { }

		void Add(System::Collections::Generic::IEnumerable<INTERACTION^>^ interactions)
		{
			MwtBaseLogger<INTERACTION^>::Add(interactions);
		}

		void Add(INTERACTION^ interaction)
		{
			MwtBaseLogger<INTERACTION^>::Add(interaction);
		}

		cli::array<INTERACTION^>^ GetAllInteractions()
		{
			return MwtBaseLogger<INTERACTION^>::GetAllItems();
		}
	};

	public ref class RewardStore : public MwtBaseLogger<float>
	{
	public:
		RewardStore(String^ file) : MwtBaseLogger<float>(file) { }

		void Add(System::Collections::Generic::IEnumerable<float>^ rewards)
		{
			MwtBaseLogger<float>::Add(rewards);
		}

		void Add(float reward)
		{
			MwtBaseLogger<float>::Add(reward);
		}

		cli::array<float>^ GetAllRewards()
		{
			return MwtBaseLogger<float>::GetAllItems();
		}
	};

	public ref class MwtExplorer
	{
	private:
		MWTExplorer* m_mwt;
		IFunctionWrapper^ policyWrapper;
		cli::array<IFunctionWrapper^>^ policyWrappers;

		// Garbage-collector handles to keep alive
		GCHandle selfHandle;
		GCHandle policyFuncHandle;
		cli::array<GCHandle>^ baggingFuncHandles;

		// Bagging-specific
		cli::array<IntPtr>^ baggingParameters;
		Stateful_Policy_Func** m_bagging_funcs;
		void** m_bagging_func_params;

		MwtLogger^ logger;

	public:
		MwtExplorer();
		MwtExplorer(MwtLogger^ logger);
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

		void Unintialize();

		UInt32 ChooseAction(CONTEXT^ context, String^ uniqueId);

		String^ GetAllInteractionsAsString();
		cli::array<INTERACTION^>^ GetAllInteractions();

	internal:
		UInt32 InvokeDefaultPolicyFunction(CONTEXT^);
		UInt32 InvokeBaggingDefaultPolicyFunction(CONTEXT^, int);
		void InvokeDefaultScorerFunction(CONTEXT^, cli::array<float>^);

	private: // Internal Initialize APIs
		void InitializeEpsilonGreedy(float epsilon, InternalStatefulPolicyDelegate^ defaultPolicyFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);
		void InitializeTauFirst(UInt32 tau, InternalStatefulPolicyDelegate^ defaultPolicyFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);
		void InitializeBagging(UInt32 bags, cli::array<InternalStatefulPolicyDelegate^>^ defaultPolicyFuncs, cli::array<IntPtr>^ defaultPolicyArgs, UInt32 numActions);
		void InitializeSoftmax(float lambda, InternalStatefulScorerDelegate^ defaultScorerFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);

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
		MWTRewardReporter* m_mwt_reward_reporter;
		Interaction** m_native_interactions;
		int m_num_native_interactions;

	public:
		MwtRewardReporter(cli::array<INTERACTION^>^ interactions);
		~MwtRewardReporter();

		bool ReportReward(String^ id, float reward);
		bool ReportReward(cli::array<String^>^ ids, cli::array<float>^ rewards);
		String^ GetAllInteractionsAsString();
		//SIDTEMP:
		cli::array<INTERACTION^>^ GetAllInteractions();
	};

	public ref class MwtOptimizer
	{
	private:
		MWTOptimizer* m_mwt_optimizer;
		Interaction** m_native_interactions;
		int m_num_native_interactions;
		IFunctionWrapper^ policyWrapper;
		GCHandle selfHandle;
		cli::array<GCHandle>^ contextHandles;
		
	public: 
		MwtOptimizer(cli::array<INTERACTION^>^ interactions, UInt32 numActions);
		~MwtOptimizer();

		generic <class T>
		float EvaluatePolicy(StatefulPolicyDelegate<T>^ policyFunc, T policyParams);
		float EvaluatePolicy(StatelessPolicyDelegate^ policy_func);
		float EvaluatePolicyVWCSOAA(String^ model_input_file);
		void OptimizePolicyVWCSOAA(String^ model_output_file);
		void Uninitialize();

	internal:
		UInt32 InvokeDefaultPolicyFunction(CONTEXT^);

	private:
		float EvaluatePolicy(InternalStatefulPolicyDelegate^ policyFunc, IntPtr policyParams);

	private:
		static UInt32 InternalStatefulPolicy(IntPtr, IntPtr);
	};
}
