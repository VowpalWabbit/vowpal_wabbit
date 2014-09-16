// vw_explore.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MWT.h"

using namespace std;

MWTAction Stateful_Default_Policy(int* stateContext, Context& applicationContext)
{
	return MWTAction(*stateContext);
}

MWTAction Stateless_Default_Policy(Context& applicationContext)
{
	return MWTAction(99);
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Sample driver code
	string appId = "myapp";
	
	// Create a new MWT instance
	MWT* mwt = new MWT(appId, 10);

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

	// Now let MWT explore & choose an action
	pair<MWTAction, u64> chosen_action_join_key = mwt->Choose_Action_Join_Key(*ctx);

	char* unique_key = "1001";
	MWTAction chosen_action = mwt->Choose_Action(*ctx, unique_key, 4);
	
	cout << "Chosen Action ID with join key is: " << chosen_action_join_key.first.Get_Id() << endl;
	cout << "Chosen Action ID is: " << chosen_action.Get_Id() << endl;
	cout << mwt->Get_All_Interactions() << endl;

	delete ctx;
	delete mwt;
	return 0;
}