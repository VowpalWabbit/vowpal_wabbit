// vw_explore.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MWT.h"

using namespace std;

u32 Stateful_Default_Policy(int* stateContext, Context* applicationContext)
{
	return *stateContext;
}

u32 Stateless_Default_Policy(Context* applicationContext)
{
	return 99;
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Sample driver code
	string appId = "myapp";
	
	// Create a new MWT instance
	MWT* mwt = new MWT(appId, 10);

	float epsilon = .2f;
	float exploreBudget = .05f;

	int policyFuncArgument = 101;
	bool useStatefulFunc = true;
	if (useStatefulFunc)
	{
		mwt->Initialize_Epsilon_Greedy<int>(epsilon, Stateful_Default_Policy, &policyFuncArgument);
	}
	else
	{
		mwt->Initialize_Epsilon_Greedy(epsilon, Stateless_Default_Policy);
	}

	// Create Features & Context
	feature f[1];
	f[0].weight_index = 1;
	f[0].x = 0.5;

	Context* ctx = new Context(f, 1);

	// Now let MWT explore & choose an action
	pair<u32, u64> chosen_action_join_key = mwt->Choose_Action_Join_Key(*ctx);

	char* unique_key = "1001";
	u32 chosen_action = mwt->Choose_Action(*ctx, unique_key, 4);
	
	cout << "Chosen Action ID with join key is: " << chosen_action_join_key.first << endl;
	cout << "Chosen Action ID is: " << chosen_action << endl;
	cout << mwt->Get_All_Interactions() << endl;

	delete ctx;
	delete mwt;
	return 0;
}