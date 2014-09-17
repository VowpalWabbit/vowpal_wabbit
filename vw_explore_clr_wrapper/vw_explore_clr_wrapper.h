#pragma once

#include "mwt.h"

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace MultiWorldTesting {
	public delegate UInt32 StatefulPolicyDelegate(IntPtr, IntPtr);
	public delegate UInt32 StatelessPolicyDelegate(IntPtr);

	[StructLayout(LayoutKind::Sequential)]
	public value struct FEATURE
	{
		float X;
		UInt32 WeightIndex;
	};

	public ref class MWTWrapper
	{
	private:
		MWT* m_mwt;

	public:
		MWTWrapper(String^ appId, UInt32 numActions);
		~MWTWrapper();

		void InitializeEpsilonGreedy(float epsilon, StatefulPolicyDelegate^ defaultPolicyFunc, IntPtr defaultPolicyFuncContext);
		void InitializeEpsilonGreedy(float epsilon, StatelessPolicyDelegate^ defaultPolicyFunc);

		UInt32 ChooseAction(cli::array<FEATURE>^ contextFeatures, String^ otherContext, String^ uniqueId);

		String^ GetAllInteractions();
	};
}
