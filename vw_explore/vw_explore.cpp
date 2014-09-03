// vw_explore.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MWT.h"

using namespace std;

atomic_uint64_t IDGenerator::gId = 0;

Action Stateful_Default_Policy(int* stateContext, Context& applicationContext, ActionSet& actions)
{
	return Action(*stateContext);
}

Action Stateless_Default_Policy(Context& applicationContext, ActionSet& actions)
{
	return Action(99);
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Sample driver code
	string appId = "myapp";
	
	// Create a new MWT instance
	MWT* mwt = new MWT(appId);

	float epsilon = .2f;
	float exploreBudget = .05f;

	bool useStatefulFunc = false;
	if (useStatefulFunc)
	{
		int data = 101;
		mwt->InitializeEpsilonGreedy<int>(epsilon, Stateful_Default_Policy, &data, exploreBudget);
	}
	else
	{
		mwt->InitializeEpsilonGreedy(epsilon, Stateless_Default_Policy, exploreBudget);
	}

	// Create Features & Context
	vector<feature> commonFeatures;
	feature f;
	f.weight_index = 1;
	f.x = 0.5;
	commonFeatures.push_back(f);

	Context* ctx = new Context(commonFeatures);

	// Create ActionSet
	ActionSet* actset = new	ActionSet(100, 200);

	// Now let MWT explore & choose an action
	pair<Action, u64> chosenAction = mwt->ChooseAction(*ctx, *actset);
	
	cout << "Chosen Action ID is: " << chosenAction.first.GetID() << endl;

	// Create a Reward and report
	//Reward* myReward = new Reward(2.5);
	//mwt->ReportReward(chosenAction.second, myReward);

	//delete myReward;
	delete actset;
	delete ctx;
	delete mwt;
	return 0;
}

