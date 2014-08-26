//
// Offline evaluation interface for the MWT service.
//

#include "stdafx.h"

class OfflineEvaluator
{
	PolicyStats* addExample(Interaction* interaction)
	{
		// Do offline eval computation, update pStats...
	}

private:
	Policy* pPolicy;
	PolicyStats* pStats;
};
