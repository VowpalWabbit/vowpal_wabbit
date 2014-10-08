#pragma once

#include "mwt.h"

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace MultiWorldTesting {
	public delegate UInt32 StatefulPolicyDelegate(IntPtr, IntPtr);
	public delegate UInt32 StatelessPolicyDelegate(IntPtr);

	public delegate void StatefulScorerDelegate(IntPtr, IntPtr, IntPtr scores, UInt32 size);
	public delegate void StatelessScorerDelegate(IntPtr, IntPtr scores, UInt32 size);

	[StructLayout(LayoutKind::Sequential)]
	public value struct FEATURE
	{
		float X;
		UInt32 WeightIndex;
	};

	public ref class CONTEXT
	{
	public:
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
		CONTEXT^ ApplicationContext;
		UInt32 ChosenAction;
		float Probability;
		UInt64 JoinId;
	};

	public ref class ActionID
	{
	public:
		static UInt32 Make_OneBased(UInt32 id) { return MWTAction::Make_OneBased(id); }
		static UInt32 Make_ZeroBased(UInt32 id) { return MWTAction::Make_ZeroBased(id); }
	};

	generic <class T>
	public delegate UInt32 TemplateStatefulPolicyDelegate(T, CONTEXT^);

	interface class IFunctionWrapper
	{
		public:
			virtual UInt32 InvokeFunction(CONTEXT^) abstract;
	};

	generic <class T>
	public ref class DefaultPolicyWrapper : IFunctionWrapper
	{
		public:
			DefaultPolicyWrapper(TemplateStatefulPolicyDelegate<T>^ policyFunc, T policyParams)
			{
				defaultPolicy = policyFunc;
				parameters = policyParams;
			}

			virtual UInt32 InvokeFunction(CONTEXT^ c) override
			{
				return defaultPolicy(parameters, c);
			}
		private:
			T parameters;
			TemplateStatefulPolicyDelegate<T>^ defaultPolicy;
	};

	public ref class MwtExplorer
	{
	private:
		MWTExplorer* m_mwt;
		IFunctionWrapper^ policyWrapper;
		GCHandle selfHandle;

	public:
		MwtExplorer();
		~MwtExplorer();

		generic <class T>
		void InitializeEpsilonGreedy(float epsilon, TemplateStatefulPolicyDelegate<T>^ defaultPolicyFunc, T defaultPolicyFuncParams, UInt32 numActions);

		void InitializeEpsilonGreedy(float epsilon, StatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions);

		generic <class T>
		void InitializeTauFirst(UInt32 tau, TemplateStatefulPolicyDelegate<T>^ defaultPolicyFunc, T defaultPolicyFuncParams, UInt32 numActions);

		void InitializeTauFirst(UInt32 tau, StatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions);

		void InitializeBagging(UInt32 bags, cli::array<StatefulPolicyDelegate^>^ defaultPolicyFuncs, cli::array<IntPtr>^ defaultPolicyArgs, UInt32 numActions);
		void InitializeBagging(UInt32 bags, cli::array<StatelessPolicyDelegate^>^ defaultPolicyFuncs, UInt32 numActions);

		void InitializeSoftmax(float lambda, StatefulScorerDelegate^ defaultScorerFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);
		void InitializeSoftmax(float lambda, StatelessScorerDelegate^ defaultScorerFunc, UInt32 numActions);

		UInt32 ChooseAction(CONTEXT^ context, String^ uniqueId);
		Tuple<UInt32, UInt64>^ ChooseActionAndKey(CONTEXT^ context);

		String^ GetAllInteractionsAsString();
		cli::array<INTERACTION^>^ GetAllInteractions();

	// Helper methods
	public:
		static cli::array<float>^ IntPtrToScoreArray(IntPtr scoresPtr, UInt32 size);

		generic <class T> where T : System::Object
		static T FromIntPtr(IntPtr objectPtr);

		generic <class T> where T : System::Object
		static IntPtr ToIntPtr(T obj, [Out] GCHandle% objHandle);

	internal:
		virtual UInt32 InvokeDefaultPolicyFunction(CONTEXT^) override;

	private:
		void InitializeEpsilonGreedy(float epsilon, StatefulPolicyDelegate^ defaultPolicyFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);
		void InitializeTauFirst(UInt32 tau, StatefulPolicyDelegate^ defaultPolicyFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);

		static UInt32 InternalStatefulPolicy(IntPtr, IntPtr);
	};
}
