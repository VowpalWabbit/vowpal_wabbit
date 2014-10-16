#include "stdafx.h"
#include "CppUnitTest.h"
#include "MWTExplorer.h"
#include "MWTRewardReporter.h"
#include "MWTOptimizer.h"
#include "utility.h"
#include <fstream>

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

namespace vw_explore_tests
{		
	TEST_CLASS(VWExploreUnitTests)
	{
	public:
		TEST_METHOD(Epsilon_Greedy_Stateful)
		{
			float epsilon = 0.f; // No randomization
			m_mwt->Initialize_Epsilon_Greedy<int>(epsilon, Stateful_Default_Policy, &m_policy_func_arg, m_num_actions);

			u32 expected_action = VWExploreUnitTests::Stateful_Default_Policy(&m_policy_func_arg, m_context);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(expected_action, chosen_action);

			chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(expected_action, chosen_action);

			float expected_probs[2] = { 1.f, 1.f };
			this->Test_Logger(2, expected_probs);
		}

		TEST_METHOD(Epsilon_Greedy_Stateless)
		{
			float epsilon = 0.f; // No randomization
			m_mwt->Initialize_Epsilon_Greedy(epsilon, Stateless_Default_Policy, m_num_actions);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			chosen_action = m_mwt->Choose_Action(*m_context, m_unique_key);
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			float expected_probs[2] = { 1.f, 1.f };
			this->Test_Logger(2, expected_probs);
		}

		TEST_METHOD(Epsilon_Greedy_Random)
		{
			float epsilon = 0.2f;
			m_mwt->Initialize_Epsilon_Greedy(epsilon, Stateless_Default_Policy, m_num_actions);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(1));
			Assert::AreEqual((u32)3, chosen_action); // explored but differed from default policy

			chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(2));
			Assert::AreEqual((u32)10, chosen_action); // did not explore, used default policy

			chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(70));
			Assert::AreEqual((u32)10, chosen_action); // explored & matched default policy

			float expected_probs[3] = { .02f, .82f, .82f };
			this->Test_Logger(3, expected_probs);
		}

		TEST_METHOD(Tau_First_Stateful)
		{
			u32 tau = 0;
			m_mwt->Initialize_Tau_First<int>(tau, Stateful_Default_Policy, &m_policy_func_arg, m_num_actions);

			u32 expected_action = VWExploreUnitTests::Stateful_Default_Policy(&m_policy_func_arg, m_context);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(1));
			Assert::AreEqual(expected_action, chosen_action);

			// tau = 0 means no randomization and no logging
			this->Test_Logger(0, nullptr);
		}

		TEST_METHOD(Tau_First_Stateless)
		{
			u32 tau = 0;
			m_mwt->Initialize_Tau_First(tau, Stateless_Default_Policy, m_num_actions);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(1));
			Assert::AreEqual(chosen_action, VWExploreUnitTests::Stateless_Default_Policy(m_context));

			this->Test_Logger(0, nullptr);
		}

		TEST_METHOD(Tau_First_Random)
		{
			u32 tau = 2;
			m_mwt->Initialize_Tau_First(tau, Stateless_Default_Policy, m_num_actions);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(1));
			Assert::AreEqual((u32)2, chosen_action);

			chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(2));
			Assert::AreEqual((u32)1, chosen_action);

			// Tau expired, did not explore
			chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(3));
			Assert::AreEqual((u32)10, chosen_action);

			// Only 2 interactions logged, 3rd one should not be stored
			float expected_probs[2] = { .1f, .1f };
			this->Test_Logger(2, expected_probs);
		}

		TEST_METHOD(Bagging_Stateful)
		{
			m_mwt->Initialize_Bagging<int>(m_bags, m_policy_funcs_stateful, m_policy_params, m_num_actions);

			// Every bag uses the same default policy function so expected chosen action is its return value
			u32 expected_action = VWExploreUnitTests::Stateful_Default_Policy(&m_policy_func_arg, m_context);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(1));
			Assert::AreEqual(expected_action, chosen_action);

			chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(2));
			Assert::AreEqual(expected_action, chosen_action);
			
			// All bags choose the same action, so prob = 1
			float expected_probs[2] = { 1.f, 1.f };
			this->Test_Logger(2, expected_probs);
		}

		TEST_METHOD(Bagging_Stateless)
		{
			m_mwt->Initialize_Bagging(m_bags, m_policy_funcs_stateless, m_num_actions);

			// Every bag uses the same default policy function so expected chosen action is its return value
			u32 expected_action = VWExploreUnitTests::Stateless_Default_Policy(m_context);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(1));
			Assert::AreEqual(expected_action, chosen_action);

			chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(2));
			Assert::AreEqual(expected_action, chosen_action);

			// All bags choose the same action, so prob = 1
			float expected_probs[2] = { 1.f, 1.f };
			this->Test_Logger(2, expected_probs);
		}

		TEST_METHOD(Bagging_Random)
		{
			u32 bags = 2;
			StatefulFunctionWrapper<int>::Policy_Func* funcs[2] = { Stateful_Default_Policy, Stateful_Default_Policy2 };
			int* params[2] = { &m_policy_func_arg, &m_policy_func_arg };

			m_mwt->Initialize_Bagging<int>(bags, funcs, params, m_num_actions);

			u32 chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(1));
			Assert::AreEqual((u32)3, chosen_action);

			chosen_action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(2));
			Assert::AreEqual((u32)2, chosen_action);

			// Two bags choosing different actions so prob of each is 1/2
			float expected_probs[2] = { .5f, .5f };
			this->Test_Logger(2, expected_probs);
		}

		TEST_METHOD(Softmax_Stateful)
		{
			m_mwt->Initialize_Softmax<int>(m_lambda, Stateful_Default_Scorer, &m_policy_scorer_arg, m_num_actions);
			// Scale C up since we have fewer interactions
			u32 num_decisions = m_num_actions * log(m_num_actions * 1.0) + log(NUM_ACTIONS_COVER * 1.0 / m_num_actions) * C * m_num_actions;
			// The () following the array should ensure zero-initialization
			u32* actions = new u32[m_num_actions]();
			u32 i;
			for (i = 0; i < num_decisions; i++)
			{
				u32 action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(i + 1));
				// Action IDs are 1-based
				actions[MWTAction::Make_ZeroBased(action)]++;
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

		TEST_METHOD(Softmax_Stateless)
		{
			m_mwt->Initialize_Softmax(m_lambda, Stateless_Default_Scorer, m_num_actions);
			u32 num_decisions = m_num_actions * log(m_num_actions * 1.0) + log(NUM_ACTIONS_COVER * 1.0 / m_num_actions) * C * m_num_actions;
			// The () following the array should ensure zero-initialization
			u32* actions = new u32[m_num_actions]();
			u32 i;
			for (i = 0; i < num_decisions; i++)
			{
				u32 action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(i + 1));
				// Action IDs are 1-based
				actions[MWTAction::Make_ZeroBased(action)]++;
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

		TEST_METHOD(Softmax_Stateful_Scores)
		{
			m_mwt->Initialize_Softmax<int>(0.5f, Non_Uniform_Stateful_Default_Scorer, &m_policy_scorer_arg, m_num_actions);
			
			u32 action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(1));
			action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(2));
			action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(3));

			size_t num_interactions = 0;
			Interaction** interactions = nullptr;
			m_mwt->Get_All_Interactions(num_interactions, interactions);

			Assert::AreEqual(3, (int)num_interactions);
			for (int i = 0; i < num_interactions; i++)
			{
				Assert::AreNotEqual(1.f / m_num_actions, interactions[i]->Get_Prob());
				delete interactions[i];
			}
			delete[] interactions;
		}

		TEST_METHOD(Softmax_Stateless_Scores)
		{
			m_mwt->Initialize_Softmax(0.5f, Non_Uniform_Stateless_Default_Scorer, m_num_actions);

			u32 action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(1));
			action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(2));
			action = m_mwt->Choose_Action(*m_context, this->Get_Unique_Key(3));

			size_t num_interactions = 0;
			Interaction** interactions = nullptr;
			m_mwt->Get_All_Interactions(num_interactions, interactions);

			Assert::AreEqual(3, (int)num_interactions);
			for (int i = 0; i < num_interactions; i++)
			{
				Assert::AreNotEqual(1.f / m_num_actions, interactions[i]->Get_Prob());
				delete interactions[i];
			}
			delete[] interactions;
		}

		TEST_METHOD(Reward_Reporter)
		{
			float epsilon = 0.f; // No randomization
			m_mwt->Initialize_Epsilon_Greedy(epsilon, Stateless_Default_Policy, m_num_actions);
			u32 num_decisions = m_num_actions;
			std::string* ids = new std::string[num_decisions];
			u32 i;
			for (i = 0; i < num_decisions; i++)
			{
				ids[i] = this->Get_Unique_Key(i + 1);
				u32 action = m_mwt->Choose_Action(*m_context, ids[i]);
			}

			size_t num_interactions = 0;
			Interaction** interactions = nullptr;
			m_mwt->Get_All_Interactions(num_interactions, interactions);
			Assert::AreEqual(num_decisions, (u32)num_interactions);

			MWTRewardReporter rew = MWTRewardReporter(num_interactions, interactions);
			float reward = 2.0;
			// Report a single interaction's reward
			rew.Report_Reward(ids[0], reward);
			for (i = 0; i < num_interactions; i++)
			{
				// We need to find the interaction since it's not guaranteed the interactiosn are
				// returned to us in the same order we passed them in
				float expected_reward = (interactions[i]->Get_Id() == ids[0]) ? reward : NO_REWARD;
				Assert::AreEqual(interactions[i]->Get_Reward(), expected_reward);
			}
			// Report rewards for all interactions, which should overwrite the single reward above
			reward = 3.0;
			float* all_rewards = new float[num_interactions];
			// This initializes all rewards to the value above
			std::fill_n(all_rewards, num_interactions, reward);
			rew.Report_Reward(num_decisions, ids, all_rewards);
			for (i = 0; i < num_interactions; i++)
			{
				Assert::AreEqual(interactions[i]->Get_Reward(), reward);
			}
			delete[] all_rewards;
			delete[] ids;
			delete[] interactions;
		}

		TEST_METHOD(Offline_Evaluation)
		{
			m_mwt->Initialize_Softmax(m_lambda, Stateless_Default_Scorer, m_num_actions);
			u32 num_decisions = m_num_actions * log(m_num_actions * 1.0) + log(NUM_ACTIONS_COVER * 1.0 / m_num_actions) * C * m_num_actions;
			std::string* ids = new std::string[num_decisions];
			u32 i;
			for (i = 0; i < num_decisions; i++)
			{
				ids[i] = this->Get_Unique_Key(i + 1);
				u32 action = m_mwt->Choose_Action(*m_context, ids[i]);
			}

			size_t num_interactions = 0;
			Interaction** interactions = nullptr;
			m_mwt->Get_All_Interactions(num_interactions, interactions);
			Assert::AreEqual(num_decisions, (u32)num_interactions);

			MWTRewardReporter rew = MWTRewardReporter(num_interactions, interactions);
			float reward = 0.0;
			// Offline evaluate the default policy; we assume it always returns the same
			// action
			u32 policy_action = Stateless_Default_Policy(m_context);
			float policy_weighted_sum = 0.0;
			u32 policy_matches = 0;
			for (i = 0; i < num_interactions; i++)
			{
				rew.Report_Reward(ids[i], reward);
				if (interactions[i]->Get_Action().Get_Id() == policy_action)
				{
					policy_weighted_sum += reward * (1.0 / interactions[i]->Get_Prob());
					policy_matches++;
				}
				reward += 0.1;
			}
			float policy_perf = policy_weighted_sum / policy_matches;
			MWTOptimizer opt = MWTOptimizer(num_interactions, interactions, m_num_actions);
			Assert::AreEqual(opt.Evaluate_Policy(Stateless_Default_Policy), policy_perf);
		}

		TEST_METHOD(Offline_Optimization_VW_CSOAA)
		{
			// Test using the example data below, which is designed to yield perfect classification
			// when predicting on the same (elided) data. Details of this example can be found at:
			// https://github.com/JohnLangford/vowpal_wabbit/wiki/Cost-Sensitive-One-Against-All-(csoaa)-multi-class-example
			/*
	  			1:1.0 a1_expect_1 | a
				2:1.0 b1_expect_2 | b
				3:1.0 c1_expect_3 | c
				1:2.0 2:1.0 ab1_expect_2 | a b
				2:1.0 3:3.0 bc1_expect_2 | b c
				1:3.0 3:1.0 ac1_expect_3 | a c
				2:3.0 d1_expect_2 | d
			*/

			// Create exploration data based on the cost-sensitive multi-class data above; 
			// essentially we will only use one class-label per example (the chosen action) ]
			Interaction* interactions[7];
			std::string ids[7];
			Context* context;
			float prob = 1.0 / 3;
			float feature_val = 1.0;
			u64 feature_a = Interaction::Compute_Id_Hash("a");
			u64 feature_b = Interaction::Compute_Id_Hash("b");
			u64 feature_c = Interaction::Compute_Id_Hash("c");
			u64 feature_d = Interaction::Compute_Id_Hash("d");
			u32 i;
			// Example 1
			i = 0;
			MWTFeature* features = new MWTFeature[3];
			// This indicates feature "a" is present (the specific value used is not important)
			features[0].Index = feature_a;
			features[0].X = feature_val; 
			context = new Context(features, 1, true);
			// Indicating this is a copy hands responsibility for freeing the context to the
			// Interaction class (note: we may remove this interface later)
			ids[i] = "a1_expect_1";
			interactions[i] = new Interaction(context, MWTAction(1), prob, ids[i], true);
			// Example 2
			features = new MWTFeature[3];
			features[0].Index = feature_b;
			features[0].X = feature_val;
			context = new Context(features, 1, true);
			i++;
			ids[i] = "b1_expects_2";
			interactions[i] = new Interaction(context, MWTAction(2), prob, ids[i], true);
			// Example 3
			features = new MWTFeature[3];
			features[0].Index = feature_c;
			features[0].X = feature_val;
			context = new Context(features, 1, true);
			i++;
			ids[i] = "c1_expects_3";
			interactions[i] = new Interaction(context, MWTAction(3), prob, ids[i], true);
			// Example 4
			features = new MWTFeature[3];
			features[0].Index = feature_a;
			features[0].X = feature_val;
			features[1].Index = feature_b;
			features[1].X = feature_val;
			context = new Context(features, 2, true);
			i++;
			ids[i] = "ab1_expect_2";
			interactions[i] = new Interaction(context, MWTAction(2), prob, ids[i], true);
			// Example 5
			features = new MWTFeature[3];
			features[0].Index = feature_b;
			features[0].X = feature_val;
			features[1].Index = feature_c;
			features[1].X = feature_val;
			context = new Context(features, 2, true);
			i++;
			ids[i] = "bc1_expect_2";
			interactions[i] = new Interaction(context, MWTAction(2), prob, ids[i], true);
			// Example 6
			features = new MWTFeature[3];
			features[0].Index = feature_a;
			features[0].X = feature_val;
			features[1].Index = feature_c;
			features[1].X = feature_val;
			context = new Context(features, 2, true);
			i++;
			ids[i] = "ac1_expect_3";
			interactions[i] = new Interaction(context, MWTAction(3), prob, ids[i], true);
			// Example 7
			features = new MWTFeature[3];
			features[0].Index = feature_d;
			features[0].X = feature_val;
			context = new Context(features, 1, true);
			i++;
			ids[i] = "d1_expect_2";
			interactions[i] = new Interaction(context, MWTAction(2), prob, ids[i], true);

			// Join reward information to the dataset
			MWTRewardReporter rew = MWTRewardReporter(7, interactions);
			float all_rewards[7];
			float reward = 1.0;
			// This initializes all rewards to the value above
			std::fill_n(all_rewards, 7, reward);
			rew.Report_Reward(7, ids, all_rewards);

			ofstream myfile;
			myfile.open("sidtest.out");
			std::ostringstream serialized_stream;
			for (u32 i = 0; i < 7; i++)
			{
				interactions[i]->Serialize_VW_CSOAA(serialized_stream);
				serialized_stream << "\n";
			}
			myfile << serialized_stream.str();
			myfile.close();

			// Test VW's CSOAA offline optimization
			MWTOptimizer opt = MWTOptimizer(7, interactions, 3);
			opt.Optimize_Policy_VW_CSOAA("vw_csoaa.model");
			float policy_perf = opt.Evaluate_Policy_VW_CSOAA("vw_csoaa.model");
			// The optimized policy should yield perfect predictions in this case
			Assert::AreEqual((double)policy_perf, (7 * reward * (1.0 / prob)) / 7);
		}

		TEST_METHOD(PRG_Coverage)
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

		TEST_METHOD_INITIALIZE(Test_Initialize)
		{
			// Constant for coverage tests: using Pr(T > nlnn + cn) < 1 - exp(-e^(-c)) for the time
			// T of the coupon collector problem, C = 0.5 yields a failure probability of ~0.45. 
			C = 5.0;

			m_num_actions = 10;
			m_mwt = new MWTExplorer();

			//TODO: We should eventually test randomization, else we are missing code paths
			// Initialize with 0 to test deterministic result
			m_bags = 2;
			m_policy_func_arg = 101;
			m_policy_scorer_arg = 7;
			m_lambda = 0;

			m_num_features = 1;
			m_features = new MWTFeature[m_num_features];
			m_features[0].Index = 1;
			m_features[0].X = 0.5;
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

		TEST_METHOD_CLEANUP(Test_Cleanup)
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
			return MWTAction::Make_OneBased(*policy_params % m_num_actions);
		}
		static u32 Stateful_Default_Policy2(int* policy_params, Context* applicationContext)
		{
			return MWTAction::Make_OneBased(*policy_params % m_num_actions) + 1;
		}

		static u32 Stateless_Default_Policy(Context* applicationContext)
		{
			return MWTAction::Make_OneBased(99 % m_num_actions);
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

		static void Non_Uniform_Stateful_Default_Scorer(int* policy_params, Context* applicationContext, float scores[], u32 size)
		{
			for (u32 i = 0; i < size; i++)
			{
				// Specify uniform weights using the app-supplied policy parameter (for testing, we
				// could just as easily give every action a score of 1 or 0 or whatever) 
				scores[i] = *policy_params + i;
			}
		}

		static void Non_Uniform_Stateless_Default_Scorer(Context* applicationContext, float scores[], u32 size)
		{
			for (u32 i = 0; i < size; i++)
			{
				scores[i] = i;
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

		string Get_Unique_Key(u32 seed)
		{
			PRG<u32> prg(seed);

			std::ostringstream unique_key_container;
			unique_key_container << prg.Uniform_Unit_Interval();

			return unique_key_container.str();
		}

	private:
		// Constants for action space coverage tests (balls-in-bins)
		static const u32 NUM_ACTIONS_COVER = 100;
		float C;

		MWTExplorer* m_mwt;

		u32 m_bags;
		float m_lambda;
		int m_policy_func_arg;
		int m_policy_scorer_arg;

		Context* m_context;
		int m_num_features;
		MWTFeature* m_features;

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