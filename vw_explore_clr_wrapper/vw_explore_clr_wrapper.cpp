// vw_explore_clr_wrapper.cpp : Defines the exported functions for the DLL application.
//

#include "stdafx.h"

#include "vw_explore_clr_wrapper.h"

using namespace System;
using namespace System::Collections;
using namespace System::Collections::Generic;

namespace MultiWorldTesting {

	MWTWrapper::MWTWrapper(System::String^ appId, System::UInt32 numActions)
	{
		m_mwt = new MWT(msclr::interop::marshal_as<std::string>(appId), numActions);
	}

	MWTWrapper::~MWTWrapper()
	{
		delete m_mwt;
	}
}