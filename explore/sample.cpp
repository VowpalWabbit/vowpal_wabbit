// vw_explore.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "MWT.h"

using namespace std;

u32 Stateful_Default_Policy1(int* policy_params, Context* applicationContext)
{
	return *policy_params;
}
u32 Stateful_Default_Policy2(int* policy_params, Context* applicationContext)
{
	return -*policy_params;
}
void Stateful_Default_Scorer(int* policy_params, Context* application_Context, float scores[], u32 size)
{
	for (u32 i = 0; i < size; i++)
	{
		scores[i] = *policy_params + i;
	}
}

u32 Stateless_Default_Policy1(Context* applicationContext)
{
	return 99;
}
u32 Stateless_Default_Policy2(Context* applicationContext)
{
	return 98;
}
void Stateless_Default_Scorer(Context* application_Context, float scores[], u32 size)
{
	for (u32 i = 0; i < size; i++)
	{
		scores[i] = 97 + i;
	}
}

int _tmain(int argc, _TCHAR* argv[])
{
	// Sample driver code
	string appId = "myapp";
	
	u32 num_actions = 10;

	// Create a new MWT instance
	MWTExplorer mwt(appId);

	float epsilon = .2f;
	u32 tau = 5;
	u32 bags = 10;
	float lambda = 0.5f;

	bool useStatefulFunc = true;
	if (useStatefulFunc)
	{
		int policy_params = 101;

		mwt.Initialize_Epsilon_Greedy<int>(epsilon, Stateful_Default_Policy1, &policy_params, num_actions);
		
		/*** Initialize Tau-First explore algorithm using a default policy function that accepts parameters ***/
		//mwt.Initialize_Tau_First<int>(tau, Stateful_Default_Policy1, &policy_params, num_actions);
		
		/*** Initialize Bagging explore algorithm using a default policy function that accepts parameters ***/
		//StatefulFunctionWrapper<int>::Policy_Func* funcs[2] = { Stateful_Default_Policy1, Stateful_Default_Policy2 };
		//int* params[2] = { &policy_params, &policy_params };
		//mwt.Initialize_Bagging<int>(bags, funcs, params, num_actions);

		/*** Initialize Softmax explore algorithm using a default scorer function that accepts parameters ***/
		//mwt.Initialize_Softmax<int>(lambda, Stateful_Default_Scorer, &policy_params, num_actions);
	}
	else
	{
		mwt.Initialize_Epsilon_Greedy(epsilon, Stateless_Default_Policy1, num_actions);
		
		/*** Initialize Tau-First explore algorithm using a stateless default policy function ***/
		//mwt.Initialize_Tau_First(tau, Stateless_Default_Policy1, num_actions);

		/*** Initialize Bagging explore algorithm using a stateless default policy function ***/
		//StatelessFunctionWrapper::Policy_Func* funcs[2] = { Stateless_Default_Policy1, Stateless_Default_Policy2 };
		//mwt.Initialize_Bagging(bags, funcs, num_actions);

		/*** Initialize Softmax explore algorithm using a stateless default scorer function ***/
		//mwt.Initialize_Softmax(lambda, Stateless_Default_Scorer, num_actions);
	}

	// Create Features & Context
	feature features[1];
	features[0].weight_index = 1;
	features[0].x = 0.5;

	Context context(features, 1);

	// Now let MWT explore & choose an action
	pair<u32, u64> action_and_key = mwt.Choose_Action_And_Key(context);

	string unique_key = "1001";
	u32 chosen_action = mwt.Choose_Action(context, unique_key);
	
	// Get the logged data
	cout << mwt.Get_All_Interactions() << endl;

	return 0;
}