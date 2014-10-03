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

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, (u32)m_policy_func_arg);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(chosen_action, (u32)m_policy_func_arg);

			this->Test_Logger(2);
		}

		TEST_METHOD(EpsilonGreedyStateless)
		{
			m_mwt->Initialize_Epsilon_Greedy(m_epsilon, Stateless_Default_Policy, m_num_actions);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			this->Test_Logger(2);
		}

		TEST_METHOD(TauFirstStateful)
		{
			m_mwt->Initialize_Tau_First<int>(m_tau, Stateful_Default_Policy, &m_policy_func_arg, m_num_actions);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, (u32)m_policy_func_arg);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(chosen_action, (u32)m_policy_func_arg);

			this->Test_Logger(0); // tau = 0 means no randomization and no logging
		}

		TEST_METHOD(TauFirstStateless)
		{
			m_mwt->Initialize_Tau_First(m_tau, Stateless_Default_Policy, m_num_actions);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			this->Test_Logger(0);
		}

		TEST_METHOD(SoftmaxStateful)
		{
			m_mwt->Initialize_Softmax<int>(m_lambda, Stateful_Default_Scorer, &m_policy_scorer_arg, NUM_ACTIONS);
			u32 num_decisions = NUM_ACTIONS * log(NUM_ACTIONS) + C * NUM_ACTIONS;
			u8 actions[NUM_ACTIONS] = { 0 };
			u32 i;

			for (i = 0; i < num_decisions; i++)
			{
				pair<u32, u64> action_and_key = m_mwt->Choose_Action_And_Key(*m_context);
				actions[action_and_key.first]++;
			}
			// Ensure all actions are covered
			for (i = 0; i < NUM_ACTIONS; i++)
			{
				Assert::IsTrue(actions[i] > 0);
			}
			//this->Test_Logger
		}

		TEST_METHOD(SoftmaxStateless)
		{
			m_mwt->Initialize_Softmax(m_lambda, Stateless_Default_Scorer, NUM_ACTIONS);
			u32 num_decisions = NUM_ACTIONS * log(NUM_ACTIONS) + C * NUM_ACTIONS;
			u8 actions[NUM_ACTIONS] = { 0 };
			u32 i;

			for (i = 0; i < num_decisions; i++)
			{
				pair<u32, u64> action_and_key = m_mwt->Choose_Action_And_Key(*m_context);
				actions[action_and_key.first]++;
			}
			// Ensure all actions are covered
			for (i = 0; i < m_num_actions; i++)
			{
				Assert::IsTrue(actions[i] > 0);
			}
		}

		TEST_METHOD(PRGCoverage)
		{
			//We're going to throw balls in bins, so 8 bits should be sufficient
			u8 bins[NUM_ACTIONS] = { 0 };
			u32 num_balls = NUM_ACTIONS * log(NUM_ACTIONS) + C * NUM_ACTIONS;
			PRG<u32> prg;
			u32 i;
			for (i = 0; i < num_balls; i++)
			{
				bins[prg.Uniform_Int(0, NUM_ACTIONS - 1)]++;
			}
			// Ensure all actions are covered
			for (i = 0; i < NUM_ACTIONS; i++)
			{
				//Assert::IsTrue(bins[i] > 0);
			}
		}

		TEST_METHOD_INITIALIZE(TestInitialize)
		{
			// Constant for coverage tests: using Pr(T > nlnn + cn) < 1 - exp(-e^(-c)) for the time
			// T of the coupon collector problem, C = 0.5 yields a failure probability of ~0.45. 
			C = 0.5;

			m_num_actions = 10;
			m_app_id = "MWT Test App";
			m_mwt = new MWTExplorer(m_app_id);

			//TODO: We should eventually test randomization, else we are missing code paths
			// Initialize with 0 to test deterministic result
			m_epsilon = 0;
			m_tau = 0;
			m_policy_func_arg = 101;
			m_policy_scorer_arg = 7;
			m_lambda = 0;

			m_num_features = 1;
			m_features = new feature[m_num_features];
			m_features[0].weight_index = 1;
			m_features[0].x = 0.5;
			m_context = new Context(m_features, m_num_features);

			m_unique_key = "1001";
		}

		TEST_METHOD_CLEANUP(TestCleanup)
		{
			if (m_features)
				delete m_features;
			if (m_context)
				delete m_context;
			if (m_mwt)
				delete m_mwt;

			m_features = nullptr;
			m_context = nullptr;
			m_mwt = nullptr;
		}

	public:
		static u32 Stateful_Default_Policy(int* policy_params, Context* applicationContext)
		{
			return *policy_params;
		}

		static u32 Stateless_Default_Policy(Context* applicationContext)
		{
			return 99;
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
		inline void Test_Logger(int num_interactions_expected)
		{
			size_t num_interactions = 0;
			Interaction** interactions = nullptr;
			m_mwt->Get_All_Interactions(num_interactions, interactions);

			Assert::AreEqual(num_interactions_expected, (int)num_interactions);

			delete[] interactions;
		}

	private:
		// Constants for action space coverage tests (balls-in-bins)
		static const u32 NUM_ACTIONS = 100;
		float C;

		string m_app_id;
		MWTExplorer* m_mwt;

		float m_epsilon;
		u32 m_tau;
		float m_lambda;
		int m_policy_func_arg;
		int m_policy_scorer_arg;

		Context* m_context;
		int m_num_features;
		feature* m_features;

		int m_num_actions;
		string m_unique_key;
		int m_unique_key_length;
	};
}