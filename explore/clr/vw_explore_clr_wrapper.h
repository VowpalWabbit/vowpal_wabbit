#pragma once

#include "mwt.h"

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace MultiWorldTesting {
	public delegate UInt32 StatefulPolicyDelegate(IntPtr, IntPtr);
	public delegate UInt32 StatelessPolicyDelegate(IntPtr);

	public delegate void StatefulScorerDelegate(IntPtr, IntPtr, cli::array<float>^ scores, UInt32 size);
	public delegate void StatelessScorerDelegate(IntPtr, cli::array<float>^ scores, UInt32 size);

	[StructLayout(LayoutKind::Sequential)]
	public value struct FEATURE
	{
		float X;
		UInt32 WeightIndex;
	};

	public ref class MWTWrapper
	{
	private:
		MWTExplorer* m_mwt;

	public:
		MWTWrapper(String^ appId);
		~MWTWrapper();

		void InitializeEpsilonGreedy(float epsilon, StatefulPolicyDelegate^ defaultPolicyFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);
		void InitializeEpsilonGreedy(float epsilon, StatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions);

		void InitializeTauFirst(UInt32 tau, StatefulPolicyDelegate^ defaultPolicyFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);
		void InitializeTauFirst(UInt32 tau, StatelessPolicyDelegate^ defaultPolicyFunc, UInt32 numActions);

		void InitializeBagging(UInt32 bags, cli::array<StatefulPolicyDelegate^>^ defaultPolicyFuncs, cli::array<IntPtr>^ defaultPolicyArgs, UInt32 numActions);
		void InitializeBagging(UInt32 bags, cli::array<StatelessPolicyDelegate^>^ defaultPolicyFuncs, UInt32 numActions);

		void InitializeSoftmax(float lambda, StatefulScorerDelegate^ defaultScorerFunc, IntPtr defaultPolicyFuncContext, UInt32 numActions);
		void InitializeSoftmax(float lambda, StatelessScorerDelegate^ defaultScorerFunc, UInt32 numActions);

		UInt32 ChooseAction(cli::array<FEATURE>^ contextFeatures, String^ otherContext, String^ uniqueId);

		String^ GetAllInteractions();
	};
}
