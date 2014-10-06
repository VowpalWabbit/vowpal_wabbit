#include "stdafx.h"
#include "CppUnitTest.h"
#include "MWT.h"
#include "utility.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace vw_explore_tests
{		
	TEST_CLASS(VWExploreUnitTests)
	{
	public:
		TEST_METHOD(EpsilonGreedyStateful)
		{
			m_mwt->Initialize_Epsilon_Greedy<int>(m_epsilon, Stateful_Default_Policy, &m_policy_func_arg, m_num_actions);

			u32 expected_action = VWExploreUnitTests::Stateful_Default_Policy(&m_policy_func_arg, m_context);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(expected_action, chosen_action_join_key.first);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(expected_action, chosen_action);

			float expected_probs[2] = { 1.f, 1.f };
			this->Test_Logger(2, expected_probs);
		}

		TEST_METHOD(EpsilonGreedyStateless)
		{
			m_mwt->Initialize_Epsilon_Greedy(m_epsilon, Stateless_Default_Policy, m_num_actions);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			float expected_probs[2] = { 1.f, 1.f };
			this->Test_Logger(2, expected_probs);
		}

		TEST_METHOD(TauFirstStateful)
		{
			m_mwt->Initialize_Tau_First<int>(m_tau, Stateful_Default_Policy, &m_policy_func_arg, m_num_actions);

			u32 expected_action = VWExploreUnitTests::Stateful_Default_Policy(&m_policy_func_arg, m_context);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(expected_action, chosen_action_join_key.first);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(expected_action, chosen_action);

			// tau = 0 means no randomization and no logging
			// TODO: test probabilities when randomization is covered in another fix
			this->Test_Logger(0, nullptr);
		}

		TEST_METHOD(TauFirstStateless)
		{
			m_mwt->Initialize_Tau_First(m_tau, Stateless_Default_Policy, m_num_actions);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			// TODO: test probabilities when randomization is covered in another fix
			this->Test_Logger(0, nullptr);
		}

		TEST_METHOD(BaggingStateful)
		{
			m_mwt->Initialize_Bagging<int>(m_bags, m_policy_funcs_stateful, m_policy_params, m_num_actions);

			// Every bag uses the same default policy function so expected chosen action is its return value
			u32 expected_action = VWExploreUnitTests::Stateful_Default_Policy(&m_policy_func_arg, m_context);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(expected_action, chosen_action_join_key.first);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(expected_action, chosen_action);
			
			// All bags choose the same action, so prob = 1
			float expected_probs[2] = { 1.f, 1.f };
			this->Test_Logger(2, expected_probs);
		}

		TEST_METHOD(BaggingStateless)
		{
			m_mwt->Initialize_Bagging(m_bags, m_policy_funcs_stateless, m_num_actions);

			// Every bag uses the same default policy function so expected chosen action is its return value
			u32 expected_action = VWExploreUnitTests::Stateless_Default_Policy(m_context);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(expected_action, chosen_action_join_key.first);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(expected_action, chosen_action);

			// All bags choose the same action, so prob = 1
			float expected_probs[2] = { 1.f, 1.f };
			this->Test_Logger(2, expected_probs);
		}

		TEST_METHOD(SoftmaxStateful)
		{
			m_mwt->Initialize_Softmax<int>(m_lambda, Stateful_Default_Scorer, &m_policy_scorer_arg, m_num_actions);
			// Scale C up since we have fewer interactions
			u32 num_decisions = m_num_actions * log(m_num_actions * 1.0) + log(NUM_ACTIONS_COVER * 1.0 / m_num_actions) * C * m_num_actions;
			// The () following the array should ensure zero-initialization
			u32* actions = new u32[m_num_actions]();
			u32 i;
			for (i = 0; i < num_decisions; i++)
			{
				pair<u32, u64> action_and_key = m_mwt->Choose_Action_And_Key(*m_context);
				// Action IDs are 1-based
				actions[action_and_key.first - 1]++;
			}
			// Ensure all actions are covered
			for (i = 0; i < m_num_actions; i++)
			{
				Assert::IsTrue(actions[i] > 0);
			}		
			float* expected_probs = new float[num_decisions];
			for (i = 0; i < num_decisions; i++)
			{
				// Our default scorer currently assigns equal weight to each action
				expected_probs[i] = 1.0 / m_num_actions;
			}
			this->Test_Logger(num_decisions, expected_probs);

			delete actions;
			delete expected_probs;
		}

		TEST_METHOD(SoftmaxStateless)
		{
			m_mwt->Initialize_Softmax(m_lambda, Stateless_Default_Scorer, m_num_actions);
			u32 num_decisions = m_num_actions * log(m_num_actions * 1.0) + log(NUM_ACTIONS_COVER * 1.0 / m_num_actions) * C * m_num_actions;
			// The () following the array should ensure zero-initialization
			u32* actions = new u32[m_num_actions]();
			u32 i;
			for (i = 0; i < num_decisions; i++)
			{
				pair<u32, u64> action_and_key = m_mwt->Choose_Action_And_Key(*m_context);
				// Action IDs are 1-based
				actions[action_and_key.first - 1]++;
			}
			// Ensure all actions are covered
			for (i = 0; i < m_num_actions; i++)
			{
				Assert::IsTrue(actions[i] > 0);
			}
			float* expected_probs = new float[num_decisions];
			for (i = 0; i < num_decisions; i++)
			{
				// Our default scorer currently assigns equal weight to each action
				expected_probs[i] = 1.0 / m_num_actions;
			}
			this->Test_Logger(num_decisions, expected_probs);

			delete actions;
			delete expected_probs;
		}

		/*
		TEST_METHOD(RewardReporter)
		{
			size_t num_interactions = 0;
			Interaction** interactions = nullptr;
			m_mwt->Get_All_Interactions(num_interactions, interactions);

			//TODO: These are completely bogus calls for now just to force compilation of templated methods
			MWTRewardReporter* rew = new MWTRewardReporter(num_interactions, interactions);
			MWTOptimizer* opt = new MWTOptimizer(num_interactions, interactions);
			rew->ReportReward(1, 2.0);
			int temp = 5;
			opt->Evaluate_Policy<int>(Stateful_Default_Policy, &temp);
		}
		*/

		TEST_METHOD(PRGCoverage)
		{
			m_mwt->Initialize_Softmax(m_lambda, Stateless_Default_Scorer, m_num_actions);
			// We could use many fewer bits (e.g. u8) per bin since we're throwing uniformly at
			// random, but this is safer in case we change things
			u32 bins[NUM_ACTIONS_COVER] = { 0 };
			u32 num_balls = NUM_ACTIONS_COVER * log(NUM_ACTIONS_COVER) + C * NUM_ACTIONS_COVER;
			PRG<u32> prg;
			u32 i;
			for (i = 0; i < num_balls; i++)
			{
				bins[prg.Uniform_Int(0, NUM_ACTIONS_COVER - 1)]++;
			}
			// Ensure all actions are covered
			for (i = 0; i < NUM_ACTIONS_COVER; i++)
			{
				Assert::IsTrue(bins[i] > 0);
			}
			this->Test_Logger(0, nullptr);
		}

		TEST_METHOD_INITIALIZE(TestInitialize)
		{
			// Constant for coverage tests: using Pr(T > nlnn + cn) < 1 - exp(-e^(-c)) for the time
			// T of the coupon collector problem, C = 0.5 yields a failure probability of ~0.45. 
			C = 5.0;

			m_num_actions = 10;
			m_app_id = "MWT Test App";
			m_mwt = new MWTExplorer(m_app_id);

			//TODO: We should eventually test randomization, else we are missing code paths
			// Initialize with 0 to test deterministic result
			m_epsilon = 0;
			m_tau = 0;
			m_bags = 2;
			m_policy_func_arg = 101;
			m_policy_scorer_arg = 7;
			m_lambda = 0;

			m_num_features = 1;
			m_features = new feature[m_num_features];
			m_features[0].weight_index = 1;
			m_features[0].x = 0.5;
			m_context = new Context(m_features, m_num_features);

			m_unique_key = "1001";

			m_policy_funcs_stateful = new StatefulFunctionWrapper<int>::Policy_Func*[m_bags];
			m_policy_funcs_stateless = new StatelessFunctionWrapper::Policy_Func*[m_bags];
			m_policy_params = new int*[m_bags];
			for (u32 i = 0; i < m_bags; i++)
			{
				m_policy_funcs_stateful[i] = Stateful_Default_Policy;
				m_policy_funcs_stateless[i] = Stateless_Default_Policy;
				m_policy_params[i] = &m_policy_func_arg;
			}
		}

		TEST_METHOD_CLEANUP(TestCleanup)
		{
			delete m_features;
			delete m_context;
			delete m_mwt;

			m_features = nullptr;
			m_context = nullptr;
			m_mwt = nullptr;

			delete[] m_policy_funcs_stateful;
			delete[] m_policy_params;
		}

	public:
		static u32 Stateful_Default_Policy(int* policy_params, Context* applicationContext)
		{
			return *policy_params % m_num_actions + 1; // 1-based index
		}

		static u32 Stateless_Default_Policy(Context* applicationContext)
		{
			return 99 % m_num_actions + 1; // 1-based index
		}

		//TODO: For now assume the size of the score array is the number of action scores to
		// report, but we need a more general way to determine per-action features (opened github issue)
		static void Stateful_Default_Scorer(int* policy_params, Context* applicationContext, float scores[], u32 size)
		{
			for (u32 i = 0; i < size; i++)
			{
				// Specify uniform weights using the app-supplied policy parameter (for testing, we
				// could just as easily give every action a score of 1 or 0 or whatever) 
				scores[i] = *policy_params;
			}
		}

		static void Stateless_Default_Scorer(Context* applicationContext, float scores[], u32 size)
		{
			for (u32 i = 0; i < size; i++)
			{
				scores[i] = 1;
			}
		}

	private: 
		inline void Test_Logger(int num_interactions_expected, float* probs_expected)
		{
			size_t num_interactions = 0;
			Interaction** interactions = nullptr;
			m_mwt->Get_All_Interactions(num_interactions, interactions);

			Assert::AreEqual(num_interactions_expected, (int)num_interactions);
			for (int i = 0; i < num_interactions; i++)
			{
				Assert::AreEqual(probs_expected[i], interactions[i]->Get_Prob());
			}

			for (int i = 0; i < num_interactions_expected; i++)
			{
				delete interactions[i];
			}
			delete[] interactions;
		}

	private:
		// Constants for action space coverage tests (balls-in-bins)
		static const u32 NUM_ACTIONS_COVER = 100;
		float C;

		string m_app_id;
		MWTExplorer* m_mwt;

		float m_epsilon;
		u32 m_tau;
		u32 m_bags;
		float m_lambda;
		int m_policy_func_arg;
		int m_policy_scorer_arg;

		Context* m_context;
		int m_num_features;
		feature* m_features;

		static int m_num_actions;
		string m_unique_key;
		int m_unique_key_length;

		StatefulFunctionWrapper<int>::Policy_Func** m_policy_funcs_stateful;
		StatelessFunctionWrapper::Policy_Func** m_policy_funcs_stateless;
		int** m_policy_params;
	};
	// Static variables need to be initialized externally in a .cpp file for linker to work
	int VWExploreUnitTests::m_num_actions = 0;
}