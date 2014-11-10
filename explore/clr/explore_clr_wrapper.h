#pragma once
#include "explore_interop.h"

namespace MultiWorldTesting {

	generic <class Ctx>
	public ref class EpsilonGreedyExplorer : public IExplorer<Ctx>, public PolicyCallback<Ctx>
	{
	public:
		EpsilonGreedyExplorer(IPolicy<Ctx>^ defaultPolicy, float epsilon, UInt32 numActions)
		{
			this->defaultPolicy = defaultPolicy;
			m_explorer = new NativeMultiWorldTesting::EpsilonGreedyExplorer<NativeContext>(*GetNativePolicy(), epsilon, (u32)numActions);
		}

		~EpsilonGreedyExplorer()
		{
			delete m_explorer;
		}

	internal:
		virtual UInt32 InvokePolicyCallback(Ctx context, int index) override
		{
			return defaultPolicy->ChooseAction(context);
		}

		NativeMultiWorldTesting::EpsilonGreedyExplorer<NativeContext>* Get()
		{
			return m_explorer;
		}

	private:
		IPolicy<Ctx>^ defaultPolicy;
		NativeMultiWorldTesting::EpsilonGreedyExplorer<NativeContext>* m_explorer;
	};

	generic <class Ctx>
	public ref class TauFirstExplorer : public IExplorer<Ctx>, public PolicyCallback<Ctx>
	{
	public:
		TauFirstExplorer(IPolicy<Ctx>^ defaultPolicy, UInt32 tau, UInt32 numActions)
		{
			this->defaultPolicy = defaultPolicy;
			m_explorer = new NativeMultiWorldTesting::TauFirstExplorer<NativeContext>(*GetNativePolicy(), tau, (u32)numActions);
		}

		~TauFirstExplorer()
		{
			delete m_explorer;
		}

	internal:
		virtual UInt32 InvokePolicyCallback(Ctx context, int index) override
		{
			return defaultPolicy->ChooseAction(context);
		}

		NativeMultiWorldTesting::TauFirstExplorer<NativeContext>* Get()
		{
			return m_explorer;
		}

	private:
		IPolicy<Ctx>^ defaultPolicy;
		NativeMultiWorldTesting::TauFirstExplorer<NativeContext>* m_explorer;
	};

	generic <class Ctx>
	public ref class SoftmaxExplorer : public IExplorer<Ctx>, public ScorerCallback<Ctx>
	{
	public:
		SoftmaxExplorer(IScorer<Ctx>^ defaultScorer, float lambda, UInt32 numActions)
		{
			this->defaultScorer = defaultScorer;
			m_explorer = new NativeMultiWorldTesting::SoftmaxExplorer<NativeContext>(*GetNativeScorer(), lambda, (u32)numActions);
		}

		~SoftmaxExplorer()
		{
			delete m_explorer;
		}

	internal:
		virtual List<float>^ InvokeScorerCallback(Ctx context) override
		{
			return defaultScorer->ScoreActions(context);
		}

		NativeMultiWorldTesting::SoftmaxExplorer<NativeContext>* Get()
		{
			return m_explorer;
		}

	private:
		IScorer<Ctx>^ defaultScorer;
		NativeMultiWorldTesting::SoftmaxExplorer<NativeContext>* m_explorer;
	};

	generic <class Ctx>
	public ref class GenericExplorer : public IExplorer<Ctx>, public ScorerCallback<Ctx>
	{
	public:
		GenericExplorer(IScorer<Ctx>^ defaultScorer, UInt32 numActions)
		{
			this->defaultScorer = defaultScorer;
			m_explorer = new NativeMultiWorldTesting::GenericExplorer<NativeContext>(*GetNativeScorer(), (u32)numActions);
		}

		~GenericExplorer()
		{
			delete m_explorer;
		}

	internal:
		virtual List<float>^ InvokeScorerCallback(Ctx context) override
		{
			return defaultScorer->ScoreActions(context);
		}

		NativeMultiWorldTesting::GenericExplorer<NativeContext>* Get()
		{
			return m_explorer;
		}

	private:
		IScorer<Ctx>^ defaultScorer;
		NativeMultiWorldTesting::GenericExplorer<NativeContext>* m_explorer;
	};

	generic <class Ctx>
	public ref class BaggingExplorer : public IExplorer<Ctx>, public PolicyCallback<Ctx>
	{
	public:
		BaggingExplorer(cli::array<IPolicy<Ctx>^>^ defaultPolicies, UInt32 numBags, UInt32 numActions)
		{
			this->defaultPolicies = defaultPolicies;
			m_explorer = new NativeMultiWorldTesting::BaggingExplorer<NativeContext>(*GetNativePolicies(numBags), (u32)numBags, (u32)numActions);
		}

		~BaggingExplorer()
		{
			delete m_explorer;
		}

	internal:
		virtual UInt32 InvokePolicyCallback(Ctx context, int index) override
		{
			if (index < 0 || index >= defaultPolicies->Length)
			{
				throw gcnew InvalidDataException("Internal error: Index of interop bag is out of range.");
			}
			return defaultPolicies[index]->ChooseAction(context);
		}

		NativeMultiWorldTesting::BaggingExplorer<NativeContext>* Get()
		{
			return m_explorer;
		}

	private:
		cli::array<IPolicy<Ctx>^>^ defaultPolicies;
		NativeMultiWorldTesting::BaggingExplorer<NativeContext>* m_explorer;
	};

	generic <class Ctx>
	public ref class MwtExplorer : public RecorderCallback<Ctx>
	{
	public:
		MwtExplorer(String^ appId, IRecorder<Ctx>^ recorder)
		{
			this->appId = appId;
			this->recorder = recorder;
		}

		UInt32 ChooseAction(IExplorer<Ctx>^ explorer, String^ unique_key, Ctx context)
		{
			String^ salt = this->appId;
			NativeMultiWorldTesting::MwtExplorer<NativeContext> mwt(marshal_as<std::string>(salt), *GetNativeRecorder());

			GCHandle selfHandle = GCHandle::Alloc(this);
			IntPtr selfPtr = (IntPtr)selfHandle;

			GCHandle contextHandle = GCHandle::Alloc(context);
			IntPtr contextPtr = (IntPtr)contextHandle;

			GCHandle explorerHandle = GCHandle::Alloc(explorer);
			IntPtr explorerPtr = (IntPtr)explorerHandle;

			NativeContext native_context(selfPtr.ToPointer(), explorerPtr.ToPointer(), contextPtr.ToPointer());
			u32 action = 0;
			if (explorer->GetType() == EpsilonGreedyExplorer<Ctx>::typeid)
			{
				EpsilonGreedyExplorer<Ctx>^ epsilonGreedyExplorer = (EpsilonGreedyExplorer<Ctx>^)explorer;
				action = mwt.Choose_Action(*epsilonGreedyExplorer->Get(), marshal_as<std::string>(unique_key), native_context);
			}
			else if (explorer->GetType() == TauFirstExplorer<Ctx>::typeid)
			{
				TauFirstExplorer<Ctx>^ tauFirstExplorer = (TauFirstExplorer<Ctx>^)explorer;
				action = mwt.Choose_Action(*tauFirstExplorer->Get(), marshal_as<std::string>(unique_key), native_context);
			}
			else if (explorer->GetType() == SoftmaxExplorer<Ctx>::typeid)
			{
				SoftmaxExplorer<Ctx>^ softmaxExplorer = (SoftmaxExplorer<Ctx>^)explorer;
				action = mwt.Choose_Action(*softmaxExplorer->Get(), marshal_as<std::string>(unique_key), native_context);
			}
			else if (explorer->GetType() == GenericExplorer<Ctx>::typeid)
			{
				GenericExplorer<Ctx>^ genericExplorer = (GenericExplorer<Ctx>^)explorer;
				action = mwt.Choose_Action(*genericExplorer->Get(), marshal_as<std::string>(unique_key), native_context);
			}
			else if (explorer->GetType() == BaggingExplorer<Ctx>::typeid)
			{
				BaggingExplorer<Ctx>^ baggingExplorer = (BaggingExplorer<Ctx>^)explorer;
				action = mwt.Choose_Action(*baggingExplorer->Get(), marshal_as<std::string>(unique_key), native_context);
			}

			explorerHandle.Free();
			contextHandle.Free();
			selfHandle.Free();

			return action;
		}

	internal:
		virtual void InvokeRecorderCallback(Ctx context, UInt32 action, float probability, String^ unique_key) override
		{
			recorder->Record(context, action, probability, unique_key);
		}

	private:
		IRecorder<Ctx>^ recorder;
		String^ appId;
	};

	[StructLayout(LayoutKind::Sequential)]
	public value struct Feature
	{
		float Value;
		UInt32 Id;
	};

	generic <class Ctx> where Ctx : IStringContext
	public ref class StringRecorder : public IRecorder<Ctx>, public ToStringCallback<Ctx>
	{
	public:
		StringRecorder()
		{
			m_string_recorder = new NativeMultiWorldTesting::StringRecorder<NativeStringContext>();
		}

		~StringRecorder()
		{
			delete m_string_recorder;
		}

		virtual void Record(Ctx context, UInt32 action, float probability, String^ uniqueKey) override
		{
			GCHandle contextHandle = GCHandle::Alloc(context);
			IntPtr contextPtr = (IntPtr)contextHandle;

			NativeStringContext native_context(contextPtr.ToPointer(), GetCallback());
			m_string_recorder->Record(native_context, (u32)action, probability, marshal_as<string>(uniqueKey));
		}

		String^ GetRecording()
		{
			return gcnew String(m_string_recorder->Get_Recording().c_str());
		}

	private:
		NativeMultiWorldTesting::StringRecorder<NativeStringContext>* m_string_recorder;
	};

	public ref class SimpleContext : public IStringContext
	{
	public:
		SimpleContext(cli::array<Feature>^ features)
		{
			Features = features;

			// TODO: add another constructor overload for native SimpleContext to avoid copying feature values
			m_features = new vector<NativeMultiWorldTesting::Feature>();
			for (int i = 0; i < features->Length; i++)
			{
				m_features->push_back({ features[i].Value, features[i].Id });
			}

			m_native_context = new NativeMultiWorldTesting::SimpleContext(*m_features);
		}

		String^ ToString() override
		{
			return gcnew String(m_native_context->To_String().c_str());
		}

		~SimpleContext()
		{
			delete m_native_context;
		}

	public:
		cli::array<Feature>^ GetFeatures() { return Features; }

	internal:
		cli::array<Feature>^ Features;

	private:
		vector<NativeMultiWorldTesting::Feature>* m_features;
		NativeMultiWorldTesting::SimpleContext* m_native_context;
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

	public:
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
