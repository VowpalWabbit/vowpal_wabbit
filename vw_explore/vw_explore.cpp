// vw_explore.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MWT.h"

using namespace std;

atomic_uint64_t IDGenerator::gId = 0;

Action Default_Policy(int* stateContext, Context& applicationContext, ActionSet& actions)
{
	return Action(101);
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Sample driver code
	string appId = "myapp";
	
	// Create a new MWT instance
	MWT* mwt = new MWT(appId);

	int data = 10;
	mwt->InitializeEpsilonGreedy<int>(0.2f, Default_Policy, &data, 0.05f);

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

