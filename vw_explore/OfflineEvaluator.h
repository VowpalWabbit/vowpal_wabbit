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
	PolicyStats* Add_Example(Interaction* interaction)
	{
		// TODO: Do offline eval computation, update pStats...
	}

private:
	Policy* m_policy;
	PolicyStats* m_stats;
};
