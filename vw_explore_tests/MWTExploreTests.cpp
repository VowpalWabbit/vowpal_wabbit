#include "stdafx.h"
#include "CppUnitTest.h"
#include "MWT.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace vw_explore_tests
{		
	TEST_CLASS(VWExploreUnitTests)
	{
	public:
		TEST_METHOD(EpsilonGreedyStateful)
		{
			m_mwt->Initialize_Epsilon_Greedy<int>(m_epsilon, Stateful_Default_Policy, &m_policy_func_arg);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_Join_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, (u32)m_policy_func_arg);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Unique_Key(), this->Unique_Key_Length());
			Assert::AreEqual(chosen_action, (u32)m_policy_func_arg);
		}

		TEST_METHOD(EpsilonGreedyStateless)
		{
			m_mwt->Initialize_Epsilon_Greedy(m_epsilon, Stateless_Default_Policy);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_Join_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Unique_Key(), this->Unique_Key_Length());
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));
		}

		TEST_METHOD(TauFirstStateful)
		{
			m_mwt->Initialize_Tau_First<int>(m_tau, Stateful_Default_Policy, &m_policy_func_arg);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_Join_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, (u32)m_policy_func_arg);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Unique_Key(), this->Unique_Key_Length());
			Assert::AreEqual(chosen_action, (u32)m_policy_func_arg);
		}

		TEST_METHOD(TauFirstStateless)
		{
			m_mwt->Initialize_Tau_First(m_tau, Stateless_Default_Policy);

			pair<u32, u64> chosen_action_join_key = m_mwt->Choose_Action_Join_Key(*m_context);
			Assert::AreEqual(chosen_action_join_key.first, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Unique_Key(), this->Unique_Key_Length());
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));
		}

		TEST_METHOD_INITIALIZE(TestInitialize)
		{
			m_num_actions = 10;
			m_app_id = "MWT Test App";
			m_mwt = new MWT(m_app_id, m_num_actions);
		
			// Initialize with 0 to test deterministic result
			m_epsilon = 0;
			m_tau = 0;
			m_policy_func_arg = 101;

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
		static u32 Stateful_Default_Policy(int* stateContext, Context* applicationContext)
		{
			return *stateContext;
		}

		static u32 Stateless_Default_Policy(Context* applicationContext)
		{
			return 99;
		}

	private:
		char* Unique_Key() { return const_cast<char*>(m_unique_key.c_str()); }
		u32 Unique_Key_Length() { return (u32)m_unique_key.length(); }

	private:
		string m_app_id;
		MWT* m_mwt;

		float m_epsilon;
		u32 m_tau;
		int m_policy_func_arg;

		Context* m_context;
		int m_num_features;
		feature* m_features;

		int m_num_actions;
		string m_unique_key;
		int m_unique_key_length;
	};
}