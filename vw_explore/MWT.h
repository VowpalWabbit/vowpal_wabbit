//
// Main interface for clients to the MWT service.
//

#include "stdafx.h"
#include <typeinfo>

class BaseFunctionWrapper { };
class MWTEmpty { };

template <class T>
class StatefulFunctionWrapper : public BaseFunctionWrapper
{
public:
	typedef Action PolicyFunc(T* stateContext, Context& applicationContext, ActionSet& actions);

	PolicyFunc* PolicyFunction;
};

class StatelessFunctionWrapper : public BaseFunctionWrapper
{
public:
	typedef Action PolicyFunc(Context& applicationContext, ActionSet& actions);

	PolicyFunc* PolicyFunction;
};

// TODO: for exploration budget, exploration algo should implement smth like this
class Explorer : public Policy
{
//public:
//	virtual void AdjustFrequency(float frequency) = 0;
//	virtual void StopExplore() = 0;
//	virtual void StartExplore() = 0;
};

template <class T>
class EpsilonGreedyExplorer : public Explorer
{
public:
	EpsilonGreedyExplorer(
		float epsilon, 
		BaseFunctionWrapper& defaultPolicyFuncWrapper, 
		T* defaultPolicyFuncStateContext) :
			epsilon(epsilon), 
			doExplore(true), 
			defaultPolicyWrapper(defaultPolicyFuncWrapper), 
			pDefaultPolicyStateContext(defaultPolicyFuncStateContext)
	{
		if (epsilon <= 0)
		{
			throw std::invalid_argument("Initial epsilon value must be positive.");
		}
		randomGenerator = new PRG<u32>();
	}

	~EpsilonGreedyExplorer()
	{
		delete randomGenerator;
	}

	std::pair<Action, float> ChooseAction(Context& context, ActionSet& actions)
	{
		// Invoke the default policy function to get the action
		Action* chosenAction = nullptr;
		if (typeid(defaultPolicyWrapper) == typeid(StatelessFunctionWrapper))
		{
			StatelessFunctionWrapper* statelessFunctionWrapper = (StatelessFunctionWrapper*)(&defaultPolicyWrapper);
			chosenAction = &statelessFunctionWrapper->PolicyFunction(context, actions);
		}
		else
		{
			StatefulFunctionWrapper<T>* statefulFunctionWrapper = (StatefulFunctionWrapper<T>*)(&defaultPolicyWrapper);
			chosenAction = &statefulFunctionWrapper->PolicyFunction(pDefaultPolicyStateContext, context, actions);
		}

		float actionProb = 0.f;
		float baseProb = epsilon / actions.Count(); // uniform probability
		
		// TODO: check this random generation
		if (((float)randomGenerator->uniformInt() / (2e32 - 1)) < 1.f - epsilon)
		{
			actionProb = 1.f - epsilon + baseProb;
		}
		else
		{
			// Get uniform random action ID
			u32 actionId = (uint32_t)ceil(randomGenerator->uniformInt(0, actions.Count() - 1));

			if (actionId == chosenAction->GetID())
			{
				// IF it matches the one chosen by the default policy
				// then increase the probability
				actionProb = 1.f - epsilon + baseProb;
			}
			else
			{
				// Otherwise it's just the uniform probability
				actionProb = baseProb;
			}
			chosenAction = &actions.Get(actionId);
		}

		return std::pair<Action, float>(*chosenAction, actionProb);
	}

	//void AdjustFrequency(float frequency)
	//{
	//	epsilon += frequency;
	//}

	//void StopExplore()
	//{
	//	doExplore = false;
	//}
	//
	//void StartExplore()
	//{
	//	doExplore = true;
	//}

private:
	float epsilon;
	bool doExplore;
	PRG<u32>* randomGenerator;

	BaseFunctionWrapper& defaultPolicyWrapper;
	T* pDefaultPolicyStateContext;
};

class MWT
{
public:
	MWT(std::string& appId)
	{
		IDGenerator::Initialize();

		if (appId.empty())
		{
			appId = this->GenerateAppId();
		}

		pLogger = new Logger(appId);
	}

	~MWT()
	{
		delete pLogger;
		delete pExplorer;
	}

	// TODO: should we restrict explorationBudget to some small numbers to prevent users from unwanted effect?
	template <class T>
	void InitializeEpsilonGreedy(
		float epsilon, 
		typename StatefulFunctionWrapper<T>::PolicyFunc defaultPolicyFunc, 
		T* defaultPolicyFuncStateContext, 
		float explorationBudget)
	{
		StatefulFunctionWrapper<T>* funcWrapper = new StatefulFunctionWrapper<T>();
		funcWrapper->PolicyFunction = &defaultPolicyFunc;
		
		pExplorer = new EpsilonGreedyExplorer<T>(epsilon, *funcWrapper, defaultPolicyFuncStateContext);
		
		pDefaultFuncWrapper = funcWrapper;
	}

	void InitializeEpsilonGreedy(
		float epsilon, 
		StatelessFunctionWrapper::PolicyFunc defaultPolicyFunc, 
		float explorationBudget)
	{
		StatelessFunctionWrapper* funcWrapper = new StatelessFunctionWrapper();
		funcWrapper->PolicyFunction = defaultPolicyFunc;
		
		pExplorer = new EpsilonGreedyExplorer<MWTEmpty>(epsilon, *funcWrapper, nullptr);
		
		pDefaultFuncWrapper = funcWrapper;
	}

	// TODO: should include defaultPolicy here? From users view, it's much more intuitive
	std::pair<Action, u64> ChooseAction(Context& context, ActionSet& actions)
	{
		std::pair<Action, float> actionProb = pExplorer->ChooseAction(context, actions);
		Interaction* pInteraction = new Interaction(context, actionProb.first, actionProb.second);
		pLogger->Store(pInteraction);
		
		// TODO: Anything else to do here?

		return std::pair<Action, u64>(actionProb.first, pInteraction->GetId());
	}

private:
	// TODO: App ID + Interaction ID is the unique identifier
	// Users can specify a seed and we use it to generate app id for them
	// so we can guarantee uniqueness.
	std::string GenerateAppId()
	{
		return ""; // TODO: implement
	}

	void ReportReward(u64 id, Reward* reward)
	{
		pLogger->Join(id, reward);
		// TODO: Update performance measures of current and default policy (estimated via offline eval)
		// TODO: Evaluate how we're doing relative to default policy 
	}

	/// <summary>
	/// Initializes learner with parameters specified in config.
	/// </summary>
	/// <param name="config"></param>
	/// <returns>Returns true if initialization succeeds, and false otherwise.</returns> 
	//bool Initialize(Config config);

	/// <summary>
	/// Prints out parameter and other information to a string for output for logging purposes.
	/// </summary>
	/// <returns></returns>
	//string Profile(string sep = "\n");

	/// <summary>
	/// Takes actions either in the learning flight (explore/exploit algAction) or in the deployment flight (exploit-only greedyAction).
	/// </summary>
	/// <param name="context"></param>
	/// <param name="algAction"></param>
	/// <param name="greedyAction"></param>
	//void ChooseAction(Context context, out BanditCommon.Action algAction, out BanditCommon.Action greedyAction);

	/// <summary>
	/// Updates internal policy with a new batch of interaction data.
	/// </summary>
	/// <param name="interactions"></param>
	//void UpdatePolicy(List<Interaction> interactions);

	/// <summary>
	/// Internal bookkeeping when an action (algAction in TakeAction) match occurs.
	/// </summary>
	//void NotifyMatch();

	/// <summary>
	/// Final logging information of the learner, to be called after learning finishes.
	/// </summary>
	/// <returns></returns>
	//string FinalLogInfo();

private:
	std::string appId;
	Explorer* pExplorer;
	Logger* pLogger;
	BaseFunctionWrapper* pDefaultFuncWrapper;
};