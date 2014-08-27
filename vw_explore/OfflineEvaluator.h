//
// Offline evaluation interface for the MWT service.
//

#include "stdafx.h"

class PolicyStats
{
	//TODO: implement
};

class OfflineEvaluator
{
	PolicyStats* addExample(Interaction* interaction)
	{
		// TODO: Do offline eval computation, update pStats...
	}

private:
	Policy* pPolicy;
	PolicyStats* pStats;
};
