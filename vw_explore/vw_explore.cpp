// vw_explore.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MWT.h"

using namespace std;

atomic_uint64_t Id_Generator::gId = 0;

Action Stateful_Default_Policy(int* stateContext, Context& applicationContext, Action_Set& actions)
{
	return Action(*stateContext);
}

Action Stateless_Default_Policy(Context& applicationContext, Action_Set& actions)
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
		mwt->Initialize_Epsilon_Greedy<int>(epsilon, Stateful_Default_Policy, &data);
	}
	else
	{
		mwt->Initialize_Epsilon_Greedy(epsilon, Stateless_Default_Policy);
	}

	// Create Features & Context
	vector<feature> commonFeatures;
	feature f;
	f.weight_index = 1;
	f.x = 0.5;
	commonFeatures.push_back(f);

	Context* ctx = new Context(commonFeatures);

	// Create Action_Set
	Action_Set* actset = new Action_Set(100);

	// Now let MWT explore & choose an action
	pair<Action, u64> chosenAction = mwt->Choose_Action_Join_Key(*ctx, *actset);
	
	cout << "Chosen Action ID is: " << chosenAction.first.Get_Id() << endl;

	delete actset;
	delete ctx;
	delete mwt;
	return 0;
}

