#pragma once

#include "mwt.h"

#include <msclr\marshal_cppstd.h>

namespace MultiWorldTesting {
	public ref class MWTWrapper
	{
	private:
		MWT* m_mwt;

	public:
		MWTWrapper(System::String^ appId, System::UInt32 numActions);
		~MWTWrapper();
	};
}
