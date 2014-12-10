// vw_explore.cpp : Defines the entry point for the console application.
//

#include "MWTExplorer.h"
#include <chrono>
#include <tuple>
#include <iostream>

using namespace std;
using namespace std::chrono;
using namespace MultiWorldTesting;

/// Example of a custom context.
class MyContext
{

};

/// Example of a custom policy which implements the IPolicy<MyContext>,
/// declaring that this policy only interacts with MyContext objects.
class MyPolicy : public IPolicy<MyContext>
{
public:
	u32 Choose_Action(MyContext& context)
	{
		// Always returns the same action regardless of context
		return (u32)1;
	}
};

/// Example of a custom policy which implements the IPolicy<SimpleContext>,
/// declaring that this policy only interacts with SimpleContext objects.
class MySimplePolicy : public IPolicy<SimpleContext>
{
public:
	u32 Choose_Action(SimpleContext& context)
	{
		// Always returns the same action regardless of context
		return (u32)1;
	}
};

/// Example of a custom scorer which implements the IScorer<MyContext>,
/// declaring that this scorer only interacts with MyContext objects.
class MyScorer : public IScorer<MyContext>
{
public:
	MyScorer(u32 num_actions) : m_num_actions(num_actions)
	{
	
	}
	vector<float> Score_Actions(MyContext& context)
	{
		vector<float> scores;
		for (size_t i = 0; i < m_num_actions; i++)
		{
			// Gives every action the same score (which results in a uniform distribution).
			scores.push_back(.1f);
		}
		return scores;
	}
private:
	u32 m_num_actions;
};

///
/// Represents a tuple <context, action, probability, key>.
///
template <class Ctx>
struct MyInteraction
{
	Ctx Context;
	u32 Action;
	float Probability;
	string Unique_Key;
};

/// Example of a custom recorder which implements the IRecorder<MyContext>,
/// declaring that this recorder only interacts with MyContext objects.
class MyRecorder : public IRecorder<MyContext>
{
public:
	virtual void Record(MyContext& context, u32 action, float probability, string unique_key)
	{
		// Stores the tuple internally in a vector that could be used later for other purposes.
		m_interactions.push_back({ context, action, probability, unique_key });
	}
private:
	vector<MyInteraction<MyContext>> m_interactions;
};

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		cerr << "arguments: {greedy,tau-first,bootstrap,softmax,generic}" << endl;
		exit(1);
	}
	
	// Arguments for individual explorers
	if (strcmp(argv[1], "greedy") == 0)
	{
		// Initialize Epsilon-Greedy explore algorithm using MyPolicy

		// Creates a recorder of built-in StringRecorder type for string serialization
		StringRecorder<SimpleContext> recorder;

		// Creates a policy that interacts with SimpleContext type
		MySimplePolicy default_policy;

		// Creates an MwtExplorer instance using the recorder above
		MwtExplorer<SimpleContext> mwt("appid", recorder);

		u32 num_actions = 10;
		float epsilon = .2f;
		// Creates an Epsilon-Greedy explorer using the specified settings
		EpsilonGreedyExplorer<SimpleContext> explorer(default_policy, epsilon, num_actions);

		// Creates a context of built-in SimpleContext type
		vector<Feature> features;
		features.push_back({ 0.5f, 1 });
		features.push_back({ 1.3f, 11 });
		features.push_back({ -.95f, 413 });

		SimpleContext context(features);

		// Performs exploration by passing an instance of the Epsilon-Greedy exploration algorithm into MwtExplorer
		// using a sample string to uniquely identify this event
		string unique_key = "eventid";
		u32 action = mwt.Choose_Action(explorer, unique_key, context);

		cout << "Chosen action = " << action << endl;
		cout << "Exploration record = " << recorder.Get_Recording();
	}
	else if (strcmp(argv[1], "tau-first") == 0)
	{
		// Initialize Tau-First explore algorithm using MyPolicy
		MyRecorder recorder;
		MwtExplorer<MyContext> mwt("appid", recorder);

		int num_actions = 10;
		u32 tau = 5;
		MyPolicy default_policy;
		TauFirstExplorer<MyContext> explorer(default_policy, tau, num_actions);
		MyContext ctx;
		string unique_key = "eventid";
		u32 action = mwt.Choose_Action(explorer, unique_key, ctx);

		cout << "action = " << action << endl;
	}
	else if (strcmp(argv[1], "bootstrap") == 0)
	{
		// Initialize Bootstrap explore algorithm using MyPolicy
		MyRecorder recorder;
		MwtExplorer<MyContext> mwt("appid", recorder);

		u32 num_bags = 2;

		// Create a vector of smart pointers to default policies using the built-in type PolicyPtr<Context>
		IPolicy<MyContext>::Vector policy_functions;
		for (size_t i = 0; i < num_bags; i++)
		{
		  policy_functions.push_back(IPolicy<MyContext>::Ptr(new MyPolicy()));
		}
		int num_actions = 10;
		BootstrapExplorer<MyContext> explorer(policy_functions, num_actions);
		MyContext ctx;
		string unique_key = "eventid";
		u32 action = mwt.Choose_Action(explorer, unique_key, ctx);

		cout << "action = " << action << endl;
	}
	else if (strcmp(argv[1], "softmax") == 0)
	{
		// Initialize Softmax explore algorithm using MyScorer 
		MyRecorder recorder;
		MwtExplorer<MyContext> mwt("salt", recorder);

		u32 num_actions = 10;
		MyScorer scorer(num_actions);
		float lambda = 0.5f;
		SoftmaxExplorer<MyContext> explorer(scorer, lambda, num_actions);

		MyContext ctx;
		string unique_key = "eventid";
		u32 action = mwt.Choose_Action(explorer, unique_key, ctx);

		cout << "action = " << action << endl;
	}
	else if (strcmp(argv[1], "generic") == 0)
	{
		// Initialize Generic explore algorithm using MyScorer 
		MyRecorder recorder;
		MwtExplorer<MyContext> mwt("appid", recorder);

		int num_actions = 10;
		MyScorer scorer(num_actions);
		GenericExplorer<MyContext> explorer(scorer, num_actions);
		MyContext ctx;
		string unique_key = "eventid";
		u32 action = mwt.Choose_Action(explorer, unique_key, ctx);

		cout << "action = " << action << endl;
	}
	else
	{
		cerr << "unknown exploration type: " << argv[1] << endl;
		exit(1);
	}

	return 0;
}
