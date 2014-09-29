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

	MWTWrapper::MWTWrapper(System::String^ appId, System::UInt32 numActions)
	{
		m_mwt = new MWT(msclr::interop::marshal_as<std::string>(appId), numActions);
	}

	MWTWrapper::~MWTWrapper()
	{
		delete m_mwt;
	}

	void MWTWrapper::InitializeEpsilonGreedy(float epsilon, StatefulPolicyDelegate^ defaultPolicyFunc, System::IntPtr defaultPolicyFuncContext)
	{
		GCHandle gch = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateful_Policy_Func* nativeFunc = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
		m_mwt->Initialize_Epsilon_Greedy(epsilon, nativeFunc, defaultPolicyFuncContext.ToPointer());

		gch.Free();
	}

	void MWTWrapper::InitializeEpsilonGreedy(float epsilon, StatelessPolicyDelegate^ defaultPolicyFunc)
	{
		GCHandle gch = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateless_Policy_Func* nativeFunc = static_cast<Stateless_Policy_Func*>(ip.ToPointer());
		m_mwt->Initialize_Epsilon_Greedy(epsilon, nativeFunc);

		gch.Free();
	}

	void MWTWrapper::InitializeTauFirst(UInt32 tau, StatefulPolicyDelegate^ defaultPolicyFunc, System::IntPtr defaultPolicyFuncContext)
	{
		GCHandle gch = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateful_Policy_Func* nativeFunc = static_cast<Stateful_Policy_Func*>(ip.ToPointer());
		m_mwt->Initialize_Tau_First(tau, nativeFunc, defaultPolicyFuncContext.ToPointer());

		gch.Free();
	}

	void MWTWrapper::InitializeTauFirst(UInt32 tau, StatelessPolicyDelegate^ defaultPolicyFunc)
	{
		GCHandle gch = GCHandle::Alloc(defaultPolicyFunc);
		IntPtr ip = Marshal::GetFunctionPointerForDelegate(defaultPolicyFunc);

		Stateless_Policy_Func* nativeFunc = static_cast<Stateless_Policy_Func*>(ip.ToPointer());
		m_mwt->Initialize_Tau_First(tau, nativeFunc);

		gch.Free();
	}

	UInt32 MWTWrapper::ChooseAction(cli::array<FEATURE>^ contextFeatures, String^ otherContext, String^ uniqueId)
	{
		UInt32 chosenAction = 0;

		IntPtr ptrNativeUniqueId = Marshal::StringToHGlobalAnsi(uniqueId);
		std::string nativeOtherContext = marshal_as<std::string>(otherContext);

		pin_ptr<FEATURE> pinnedContextFeatures = &contextFeatures[0]; 
		FEATURE* nativeContextFeatures = pinnedContextFeatures;

		try
		{
			size_t uniqueIdLength = (size_t)uniqueId->Length;
			char* unique_id = static_cast<char*>(ptrNativeUniqueId.ToPointer());

			chosenAction = m_mwt->Choose_Action(
				(feature*)nativeContextFeatures, (size_t)contextFeatures->Length, 
				&nativeOtherContext, 
				unique_id, uniqueIdLength);
		}
		catch (System::Exception^ exception)
		{
			Marshal::FreeHGlobal(ptrNativeUniqueId);
			throw exception;
		}
		Marshal::FreeHGlobal(ptrNativeUniqueId);

		return chosenAction;
	}

	String^ MWTWrapper::GetAllInteractions()
	{
		std::string all_interactions = m_mwt->Get_All_Interactions();
		return gcnew String(all_interactions.c_str());
	}
}