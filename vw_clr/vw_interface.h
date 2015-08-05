/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw.h"
#include "vw_clr.h"
#include "vw_prediction.h"

namespace VW
{
	ref class VowpalWabbitExample;

	public interface class IVowpalWabbitExamplePool
	{
		void ReturnExampleToPool(VowpalWabbitExample^ ex);
	};

}