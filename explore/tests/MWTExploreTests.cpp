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

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Unique_Key(), this->Unique_Key_Length());
			Assert::AreEqual(chosen_action, (u32)m_policy_func_arg);

			this->Test_Logger(2);
		}

		TEST_METHOD(EpsilonGreedyStateless)
		{
			m_mwt->Initialize_Epsilon_Greedy(m_epsilon, Stateless_Default_Policy, m_num_actions);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Unique_Key(), this->Unique_Key_Length());
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			this->Test_Logger(2);
		}

		TEST_METHOD(TauFirstStateful)
		{
			m_mwt->Initialize_Tau_First<int>(m_tau, Stateful_Default_Policy, &m_policy_func_arg, m_num_actions);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, (u32)m_policy_func_arg);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Unique_Key(), this->Unique_Key_Length());
			Assert::AreEqual(chosen_action, (u32)m_policy_func_arg);

			this->Test_Logger(0); // tau = 0 means no randomization and no logging
		}

		TEST_METHOD(TauFirstStateless)
		{
			m_mwt->Initialize_Tau_First(m_tau, Stateless_Default_Policy, m_num_actions);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Unique_Key(), this->Unique_Key_Length());
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			this->Test_Logger(0);
		}

		/*
		TEST_METHOD(SoftmaxStateful)
		{
			m_mwt->Initialize_Softmax<int>(m_lambda, Stateful_Default_Scorer, &m_policy_scorer_arg);
			u32 num_decisions = m_num_actions * log(NUM_ACTIONS) + c * NUM_ACTIONS;
			u8 actions[m_num_actions] = { 0 };
			u32 i;

			for (i = 0; i < num_decisions; i++)
			{
				pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
				actions[chosen_action_join_key.first]++;
			}
			// Ensure all actions are covered
			for (i = 0; i < m_num_actions; i++)
			{
				Assert::IsTrue(actions[i] > 0);
			}
		}

		TEST_METHOD(SoftmaxStateless)
		{
			m_mwt->Initialize_Epsilon_Greedy(m_epsilon, Stateless_Default_Policy);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_And_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Unique_Key(), this->Unique_Key_Length());
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));
		}
		*/

		TEST_METHOD(PRGCoverage)
		{
			u32 const NUM_ACTIONS = 1000;
			// Using Pr(T > nlnn + cn) < 1 - exp(-e^(-c)) for the time T of the coupon collector
			// problem, setting c = 0.5 yields a failure probability of ~0.45. 
			u8 const c = 0.5;

			// We're going to throw balls in bins, so 8 bits should be sufficient
			u8 actions[NUM_ACTIONS] = { 0 };
			u32 num_balls = NUM_ACTIONS * log(NUM_ACTIONS) + c * NUM_ACTIONS;
			PRG<u32> prg;
			u32 i;
			for (i = 0; i < num_balls; i++)
			{
				actions[prg.Uniform_Int(0, NUM_ACTIONS - 1)]++;
			}
			// Ensure all actions are covered
			for (i = 0; i < NUM_ACTIONS; i++)
			{
				Assert::IsTrue(actions[i] > 0);
			}
		}

		TEST_METHOD_INITIALIZE(TestInitialize)
		{
			m_num_actions = 10;
			m_app_id = "MWT Test App";
			m_mwt = new MWTExplorer(m_app_id);

			//TODO: We should eventually test randomization, else we are missing code paths
			// Initialize with 0 to test deterministic result
			m_epsilon = 0;
			m_tau = 0;
			m_policy_func_arg = 101;
			m_policy_scorer_arg = 102;
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
			delete m_features;
			delete m_context;
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

		/*
		static u32 Stateful_Default_Scorer(int* policy_params, Context* applicationContext)
		{
			return *policy_params;
		}

		static u32 Stateless_Default_Scorer(Context* applicationContext)
		{
			return 199;
		}
		*/

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
		char* Unique_Key() { return const_cast<char*>(m_unique_key.c_str()); }
		u32 Unique_Key_Length() { return (u32)m_unique_key.length(); }

	private:
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