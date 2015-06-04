#include "CppUnitTest.h"
#include "MWTExploreTests.h"

using namespace Microsoft::VisualStudio::CppUnitTestFramework;

#define COUNT_INVALID(block) try { block } catch (std::invalid_argument) { num_ex++; }
#define COUNT_BAD_CALL(block) try { block } catch (std::invalid_argument) { num_ex++; }

namespace vw_explore_tests
{
	TEST_CLASS(VWExploreUnitTests)
	{
	public:
		TEST_METHOD(Epsilon_Greedy)
		{
			int num_actions = 10;
			float epsilon = 0.f; // No randomization
			string unique_key = "1001";
			int params = 101;

			TestPolicy<TestContext> my_policy(params, num_actions);
			TestContext my_context;
            TestRecorder<TestContext> my_recorder;

			MwtExplorer<TestContext> mwt("salt", my_recorder);
			EpsilonGreedyExplorer<TestContext> explorer(my_policy, epsilon, num_actions);

			u32 expected_action = my_policy.Choose_Action(my_context);

			u32 chosen_action = mwt.Choose_Action(explorer, unique_key, my_context);
			Assert::AreEqual(expected_action, chosen_action);

			chosen_action = mwt.Choose_Action(explorer, unique_key, my_context);
			Assert::AreEqual(expected_action, chosen_action);

			float expected_probs[2] = { 1.f, 1.f };
			vector<TestInteraction<TestContext>> interactions = my_recorder.Get_All_Interactions();
			this->Test_Interactions(interactions, 2, expected_probs);
		}

		TEST_METHOD(Epsilon_Greedy_Random)
		{
            int num_actions = 10;
            float epsilon = 0.5f; // Verify that about half the time the default policy is chosen
            int params = 101;
            TestPolicy<TestContext> my_policy(params, num_actions);
            TestContext my_context;

            EpsilonGreedyExplorer<TestContext> explorer(my_policy, epsilon, num_actions); // Initialize in fixed # action mode

            this->Epsilon_Greedy_Random_Context(num_actions, my_context, explorer, my_policy);
		}

        TEST_METHOD(Epsilon_Greedy_Random_Var_Context)
        {
            int num_actions = 10;
            float epsilon = 0.5f; // Verify that about half the time the default policy is chosen
            int params = 101;
            TestPolicy<TestVarContext> my_policy(params, num_actions);
            TestVarContext my_context(num_actions);
            
            EpsilonGreedyExplorer<TestVarContext> explorer(my_policy, epsilon); // Initialize in variable # action mode
            
            // Test results using context that supports variable # action interface but returns fixed # action.
            this->Epsilon_Greedy_Random_Context(num_actions, my_context, explorer, my_policy);
        }

        TEST_METHOD(Epsilon_Greedy_Toggle_Exploration)
        {
            int num_actions = 10;
            float epsilon = 0.5f;
            int params = 101;

            TestPolicy<TestContext> my_policy(params, num_actions);
            TestContext my_context;
            TestRecorder<TestContext> my_recorder;

            MwtExplorer<TestContext> mwt("salt", my_recorder);
            EpsilonGreedyExplorer<TestContext> explorer(my_policy, epsilon, num_actions);

            u32 policy_action = my_policy.Choose_Action(my_context);

            int times_choose = 10000;
            int times_policy_action_chosen = 0;

            explorer.Enable_Explore(false);

            // Verify that all the time the default policy is chosen
            for (int i = 0; i < times_choose; i++)
            {
                u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(i), my_context);
                if (chosen_action == policy_action)
                {
                    times_policy_action_chosen++;
                }
            }
            Assert::AreEqual(times_choose, times_policy_action_chosen);

            explorer.Enable_Explore(true);
            times_policy_action_chosen = 0;

            // Verify that about half the time the default policy is chosen
            for (int i = 0; i < times_choose; i++)
            {
                u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(i), my_context);
                if (chosen_action == policy_action)
                {
                    times_policy_action_chosen++;
                }
            }

            Assert::IsTrue(abs((double)times_policy_action_chosen / times_choose - 0.5) < 0.1);
        }

		TEST_METHOD(Tau_First)
		{
			int num_actions = 10;
			u32 tau = 0;
			int params = 101;

			TestPolicy<TestContext> my_policy(params, num_actions);
            TestRecorder<TestContext> my_recorder;
			TestContext my_context;

			MwtExplorer<TestContext> mwt("salt", my_recorder);
			TauFirstExplorer<TestContext> explorer(my_policy, tau, num_actions);

			u32 expected_action = my_policy.Choose_Action(my_context);

			u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(1), my_context);
			Assert::AreEqual(expected_action, chosen_action);

			// tau = 0 means no randomization and no logging
			vector<TestInteraction<TestContext>> interactions = my_recorder.Get_All_Interactions();
			this->Test_Interactions(interactions, 0, nullptr);
		}

		TEST_METHOD(Tau_First_Random)
		{
			int num_actions = 10;
            u32 tau = 2;
            TestContext my_context;
            TestPolicy<TestContext> my_policy(99, num_actions);
            TauFirstExplorer<TestContext> explorer(my_policy, tau, num_actions);

            this->Tau_First_Random_Context(num_actions, my_context, explorer);
		}

        TEST_METHOD(Tau_First_Random_Var_Context)
        {
            int num_actions = 10;
            u32 tau = 2;
            TestVarContext my_context(num_actions);
            TestPolicy<TestVarContext> my_policy(99, num_actions);
            TauFirstExplorer<TestVarContext> explorer(my_policy, tau);

            this->Tau_First_Random_Context(num_actions, my_context, explorer);
        }

        TEST_METHOD(Tau_First_Toggle_Exploration)
        {
            int num_actions = 10;
            u32 tau = 2;
            TestPolicy<TestContext> my_policy(99, num_actions);
            TestRecorder<TestContext> my_recorder;
            TestContext my_context;

            MwtExplorer<TestContext> mwt("salt", my_recorder);
            TauFirstExplorer<TestContext> explorer(my_policy, tau, num_actions);

            u32 policy_action = my_policy.Choose_Action(my_context);

            int times_choose = 10000;
            int times_policy_action_chosen = 0;

            explorer.Enable_Explore(false);

            // Verify that all the time the default policy is chosen
            for (int i = 0; i < times_choose; i++)
            {
                u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(i), my_context);
                if (chosen_action == policy_action)
                {
                    times_policy_action_chosen++;
                }
            }
            Assert::AreEqual(times_choose, times_policy_action_chosen);

            explorer.Enable_Explore(true);

            u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(1), my_context);
            chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(2), my_context);

            // Tau expired, did not explore
            chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(3), my_context);
            Assert::AreEqual((u32)10, chosen_action);

            // Only 2 interactions logged, 3rd one should not be stored
            vector<TestInteraction<TestContext>> interactions = my_recorder.Get_All_Interactions();
            float expected_probs[2] = { .1f, .1f };
            this->Test_Interactions(interactions, 2, expected_probs);
        }

		TEST_METHOD(Bootstrap)
		{
			int num_actions = 10;
			int params = 101;
            TestRecorder<TestContext> my_recorder;

			vector<unique_ptr<IPolicy<TestContext>>> policies;
            policies.push_back(unique_ptr<IPolicy<TestContext>>(new TestPolicy<TestContext>(params, num_actions)));
            policies.push_back(unique_ptr<IPolicy<TestContext>>(new TestPolicy<TestContext>(params + 1, num_actions)));

			TestContext my_context;

			MwtExplorer<TestContext> mwt("c++-test", my_recorder);
			BootstrapExplorer<TestContext> explorer(policies, num_actions);

			u32 expected_action1 = policies[0]->Choose_Action(my_context);
			u32 expected_action2 = policies[1]->Choose_Action(my_context);

			u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(1), my_context);
			Assert::AreEqual(expected_action2, chosen_action);

			chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(2), my_context);
			Assert::AreEqual(expected_action1, chosen_action);
			
			vector<TestInteraction<TestContext>> interactions = my_recorder.Get_All_Interactions();
			float expected_probs[2] = { .5f, .5f };
			this->Test_Interactions(interactions, 2, expected_probs);
		}

		TEST_METHOD(Bootstrap_Random)
        {
            int num_actions = 10;
            int params = 101;
            TestContext my_context;

            vector<unique_ptr<IPolicy<TestContext>>> policies;
            policies.push_back(unique_ptr<IPolicy<TestContext>>(new TestPolicy<TestContext>(params, num_actions)));
            policies.push_back(unique_ptr<IPolicy<TestContext>>(new TestPolicy<TestContext>(params + 1, num_actions)));

            BootstrapExplorer<TestContext> explorer(policies, num_actions);

            this->Bootstrap_Random_Context(num_actions, my_context, explorer);
		}

        TEST_METHOD(Bootstrap_Random_Var_Context)
        {
            int num_actions = 10;
            int params = 101;
            TestVarContext my_context(num_actions);

            vector<unique_ptr<IPolicy<TestVarContext>>> policies;
            policies.push_back(unique_ptr<IPolicy<TestVarContext>>(new TestPolicy<TestVarContext>(params, num_actions)));
            policies.push_back(unique_ptr<IPolicy<TestVarContext>>(new TestPolicy<TestVarContext>(params + 1, num_actions)));

            BootstrapExplorer<TestVarContext> explorer(policies);

            this->Bootstrap_Random_Context(num_actions, my_context, explorer);
        }

        TEST_METHOD(Bootstrap_Toggle_Exploration)
        {
            int num_actions = 10;
            int params = 101;
            TestRecorder<TestContext> my_recorder;

            vector<unique_ptr<IPolicy<TestContext>>> policies;
            policies.push_back(unique_ptr<IPolicy<TestContext>>(new TestPolicy<TestContext>(params, num_actions)));
            policies.push_back(unique_ptr<IPolicy<TestContext>>(new TestPolicy<TestContext>(params + 1, num_actions)));

            TestContext my_context;

            MwtExplorer<TestContext> mwt("c++-test", my_recorder);
            BootstrapExplorer<TestContext> explorer(policies, num_actions);

            u32 policy_action = policies[0]->Choose_Action(my_context);

            int times_choose = 10000;
            int times_policy_action_chosen = 0;

            explorer.Enable_Explore(false);

            // Verify that all the time the first policy is chosen
            for (int i = 0; i < times_choose; i++)
            {
                u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(i), my_context);
                if (chosen_action == policy_action)
                {
                    times_policy_action_chosen++;
                }
            }
            Assert::AreEqual(times_choose, times_policy_action_chosen);

            explorer.Enable_Explore(true);

            u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(1), my_context);
            chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(2), my_context);

            // Two bags choosing different actions so prob of each is 1/2
            vector<TestInteraction<TestContext>> interactions = my_recorder.Get_All_Interactions();

            float* expected_probs = new float[times_choose + 2];
            for (int i = 0; i < times_choose; i++)
            {
                expected_probs[i] = 1.f;
            }
            expected_probs[times_choose] = .5f;
            expected_probs[times_choose + 1] = .5f;
            this->Test_Interactions(interactions, times_choose + 2, expected_probs);

            delete[] expected_probs;
        }

		TEST_METHOD(Softmax)
		{
            int scorer_arg = 7;
            int num_actions = 10;
            float lambda = 0.f;

            TestContext my_context;

            TestScorer<TestContext> my_scorer(scorer_arg, num_actions);
            SoftmaxExplorer<TestContext> explorer(my_scorer, lambda, num_actions);

            this->Softmax_Context(num_actions, my_context, explorer);
		}

        TEST_METHOD(Softmax_Var_Context)
        {
            int scorer_arg = 7;
            int num_actions = 10;
            float lambda = 0.f;

            TestVarContext my_context(num_actions);

            TestScorer<TestVarContext> my_scorer(scorer_arg, num_actions);
            SoftmaxExplorer<TestVarContext> explorer(my_scorer, lambda);

            this->Softmax_Context(num_actions, my_context, explorer);
        }

		TEST_METHOD(Softmax_Scores)
		{
			int num_actions = 10;
			float lambda = 0.5f;
			int scorer_arg = 7;
            TestScorer<TestContext> my_scorer(scorer_arg, num_actions, /* uniform = */ false);
            TestRecorder<TestContext> my_recorder;
			TestContext my_context;

			MwtExplorer<TestContext> mwt("salt", my_recorder);
			SoftmaxExplorer<TestContext> explorer(my_scorer, lambda, num_actions);

			u32 action = mwt.Choose_Action(explorer, this->Get_Unique_Key(1), my_context);
			action = mwt.Choose_Action(explorer, this->Get_Unique_Key(2), my_context);
			action = mwt.Choose_Action(explorer, this->Get_Unique_Key(3), my_context);

			vector<TestInteraction<TestContext>> interactions = my_recorder.Get_All_Interactions();
			size_t num_interactions = interactions.size();

			Assert::AreEqual(3, (int)num_interactions);
			for (size_t i = 0; i < num_interactions; i++)
			{
				Assert::AreNotEqual(1.f / num_actions, interactions[i].Probability);
			}
		}

        TEST_METHOD(Softmax_Toggle_Exploration)
        {
            int num_actions = 10;
            float lambda = 0.5f;
            int scorer_arg = 7;
            TestScorer<TestContext> my_scorer(scorer_arg, num_actions, /* uniform = */ false);
            TestRecorder<TestContext> my_recorder;
            TestContext my_context;

            MwtExplorer<TestContext> mwt("salt", my_recorder);
            SoftmaxExplorer<TestContext> explorer(my_scorer, lambda, num_actions);

            vector<float> scores = my_scorer.Score_Actions(my_context);
            float max_score = 0.f;
            u32 policy_action = 0;
            for (size_t i = 0; i < scores.size(); i++)
            {
                if (max_score < scores[i])
                {
                    max_score = scores[i];
                    policy_action = (u32)i + 1;
                }
            }

            int times_choose = 10000;
            int times_policy_action_chosen = 0;

            explorer.Enable_Explore(false);

            // Verify that all the time the highest score action is chosen
            for (int i = 0; i < times_choose; i++)
            {
                u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(i), my_context);
                if (chosen_action == policy_action)
                {
                    times_policy_action_chosen++;
                }
            }
            Assert::AreEqual(times_choose, times_policy_action_chosen);

            explorer.Enable_Explore(true);

            u32 action = mwt.Choose_Action(explorer, this->Get_Unique_Key(1), my_context);
            action = mwt.Choose_Action(explorer, this->Get_Unique_Key(2), my_context);
            action = mwt.Choose_Action(explorer, this->Get_Unique_Key(3), my_context);

            vector<TestInteraction<TestContext>> interactions = my_recorder.Get_All_Interactions();
            size_t num_interactions = interactions.size();

            Assert::AreEqual(times_choose + 3, (int)num_interactions);
            for (size_t i = 0; i < (size_t)times_choose; i++)
            {
                Assert::AreEqual(1.f, interactions[i].Probability);
            }
            for (size_t i = times_choose; i < num_interactions; i++)
            {
                Assert::AreNotEqual(1.f / num_actions, interactions[i].Probability);
            }
        }

		TEST_METHOD(Generic)
		{
            int num_actions = 10;
            int scorer_arg = 7;
            
            TestContext my_context;
            
            TestScorer<TestContext> my_scorer(scorer_arg, num_actions);
            GenericExplorer<TestContext> explorer(my_scorer, num_actions);

            this->Generic_Context(num_actions, my_context, explorer);
		}

        TEST_METHOD(Generic_Var_Context)
        {
            int num_actions = 10;
            int scorer_arg = 7;

            TestVarContext my_context(num_actions);

            TestScorer<TestVarContext> my_scorer(scorer_arg, num_actions);
            GenericExplorer<TestVarContext> explorer(my_scorer);

            this->Generic_Context(num_actions, my_context, explorer);
        }

		TEST_METHOD(End_To_End_Epsilon_Greedy)
		{
			int num_actions = 10;
			float epsilon = 0.5f;
			int params = 101;

			TestSimplePolicy my_policy(params, num_actions);
			StringRecorder<SimpleContext> my_recorder;

			MwtExplorer<SimpleContext> mwt("salt", my_recorder);
			EpsilonGreedyExplorer<SimpleContext> explorer(my_policy, epsilon, num_actions);

			this->End_To_End(mwt, explorer, my_recorder);
		}

		TEST_METHOD(End_To_End_Tau_First)
		{
			int num_actions = 10;
			u32 tau = 5;
			int params = 101;

			TestSimplePolicy my_policy(params, num_actions);
			StringRecorder<SimpleContext> my_recorder;

			MwtExplorer<SimpleContext> mwt("salt", my_recorder);
			TauFirstExplorer<SimpleContext> explorer(my_policy, tau, num_actions);

			this->End_To_End(mwt, explorer, my_recorder);
		}

		TEST_METHOD(End_To_End_Bootstrap)
		{
			int num_actions = 10;
			u32 bags = 2;
			int params = 101;
			StringRecorder<SimpleContext> my_recorder;

			vector<unique_ptr<IPolicy<SimpleContext>>> policies;
			policies.push_back(unique_ptr<IPolicy<SimpleContext>>(new TestSimplePolicy(params, num_actions)));
			policies.push_back(unique_ptr<IPolicy<SimpleContext>>(new TestSimplePolicy(params, num_actions)));

			MwtExplorer<SimpleContext> mwt("salt", my_recorder);
			BootstrapExplorer<SimpleContext> explorer(policies, num_actions);

			this->End_To_End(mwt, explorer, my_recorder);
		}

		TEST_METHOD(End_To_End_Softmax)
		{
			int num_actions = 10;
			float lambda = 0.5f;
			int scorer_arg = 7;
			TestSimpleScorer my_scorer(scorer_arg, num_actions);
			StringRecorder<SimpleContext> my_recorder;

			MwtExplorer<SimpleContext> mwt("salt", my_recorder);
			SoftmaxExplorer<SimpleContext> explorer(my_scorer, lambda, num_actions);

			this->End_To_End(mwt, explorer, my_recorder);
		}

		TEST_METHOD(End_To_End_Generic)
		{
			int num_actions = 10;
			int scorer_arg = 7;
			TestSimpleScorer my_scorer(scorer_arg, num_actions);
			StringRecorder<SimpleContext> my_recorder;

			MwtExplorer<SimpleContext> mwt("salt", my_recorder);
			GenericExplorer<SimpleContext> explorer(my_scorer, num_actions);

			this->End_To_End(mwt, explorer, my_recorder);
		}

		TEST_METHOD(PRG_Coverage)
		{
			const u32 NUM_ACTIONS_COVER = 100;
			float C = 5.0f;

			// We could use many fewer bits (e.g. u8) per bin since we're throwing uniformly at
			// random, but this is safer in case we change things
			u32 bins[NUM_ACTIONS_COVER] = { 0 };
			u32 num_balls = (u32)(NUM_ACTIONS_COVER * log(NUM_ACTIONS_COVER) + C * NUM_ACTIONS_COVER);
			PRG::prg rg;
			u32 i;
			for (i = 0; i < num_balls; i++)
			{
				bins[rg.Uniform_Int(0, NUM_ACTIONS_COVER - 1)]++;
			}
			// Ensure all actions are covered
			for (i = 0; i < NUM_ACTIONS_COVER; i++)
			{
				Assert::IsTrue(bins[i] > 0);
			}
		}

		TEST_METHOD(Serialized_String)
		{
			int num_actions = 10;
			float epsilon = 0.5f;
			int params = 101;

			TestSimplePolicy my_policy(params, num_actions);

			StringRecorder<SimpleContext> my_recorder;
			MwtExplorer<SimpleContext> mwt("c++-test", my_recorder);
			EpsilonGreedyExplorer<SimpleContext> explorer(my_policy, epsilon, num_actions);

			vector<Feature> features1;
			features1.push_back({ 0.5f, 1 });
			SimpleContext context1(features1);

			u32 expected_action = my_policy.Choose_Action(context1);

			string unique_key1 = "key1";
			u32 chosen_action1 = mwt.Choose_Action(explorer, unique_key1, context1);

			vector<Feature> features2;
			features2.push_back({ -99999.5f, 123456789 });
			features2.push_back({ 1.5f, 39 });

			SimpleContext context2(features2);

			string unique_key2 = "key2";
			u32 chosen_action2 = mwt.Choose_Action(explorer, unique_key2, context2);

			string actual_log = my_recorder.Get_Recording();

			// Use hard-coded string to be independent of sprintf
			char* expected_log = "2 key1 0.55000 | 1:.5\n2 key2 0.55000 | 123456789:-99999.5 39:1.5\n";

			Assert::AreEqual(expected_log, actual_log.c_str());
		}

		TEST_METHOD(Serialized_String_Random)
		{
			PRG::prg rand;

			int num_actions = 10;
			int params = 101;

			TestSimplePolicy my_policy(params, num_actions);

			char expected_log[100] = { 0 };

			for (int i = 0; i < 10000; i++)
			{
				StringRecorder<SimpleContext> my_recorder;
				MwtExplorer<SimpleContext> mwt("c++-test", my_recorder);
				EpsilonGreedyExplorer<SimpleContext> explorer(my_policy, 0.f, num_actions);

				Feature feature;
				feature.Value = (rand.Uniform_Unit_Interval() - 0.5f) * rand.Uniform_Int(0, 100000);
				feature.Id = i;
				vector<Feature> features;
				features.push_back(feature);
				SimpleContext my_context(features);

				u32 action = mwt.Choose_Action(explorer, "", my_context);
				string actual_log = my_recorder.Get_Recording();

				ostringstream expected_stream;
				expected_stream << std::fixed << std::setprecision(10) << feature.Value;

				string expected_str = expected_stream.str();
				if (expected_str[0] == '0')
				{
					expected_str = expected_str.erase(0, 1);
				}

				sprintf_s(expected_log, "%d %s %.5f | %d:%s",
					action, "", 1.f, i, expected_str.c_str());

				size_t length = actual_log.length() - 1;
				int compare_result = string(expected_log).compare(0, length, actual_log, 0, length);
				Assert::AreEqual(0, compare_result);
			}
		}

		TEST_METHOD(Usage_Bad_Arguments)
		{
			int num_ex = 0;
			int params = 101;
            TestPolicy<TestContext> my_policy(params, 0);
            TestScorer<TestContext> my_scorer(params, 0);
			vector<unique_ptr<IPolicy<TestContext>>> policies;

			COUNT_INVALID(EpsilonGreedyExplorer<TestContext> explorer(my_policy, .5f, 0);) // Invalid # actions, must be > 0
			COUNT_INVALID(EpsilonGreedyExplorer<TestContext> explorer(my_policy, 1.5f, 10);) // Invalid epsilon, must be in [0,1]
			COUNT_INVALID(EpsilonGreedyExplorer<TestContext> explorer(my_policy, -.5f, 10);) // Invalid epsilon, must be in [0,1]

			COUNT_INVALID(BootstrapExplorer<TestContext> explorer(policies, 0);) // Invalid # actions, must be > 0
			COUNT_INVALID(BootstrapExplorer<TestContext> explorer(policies, 1);) // Invalid # bags, must be > 0

			COUNT_INVALID(TauFirstExplorer<TestContext> explorer(my_policy, 1, 0);) // Invalid # actions, must be > 0
			COUNT_INVALID(SoftmaxExplorer<TestContext> explorer(my_scorer, .5f, 0);) // Invalid # actions, must be > 0
			COUNT_INVALID(GenericExplorer<TestContext> explorer(my_scorer, 0);) // Invalid # actions, must be > 0


			Assert::AreEqual(8, num_ex);
		}

		TEST_METHOD(Usage_Bad_Policy)
		{
			int num_ex = 0;

			// Default policy returns action outside valid range
			COUNT_BAD_CALL
			(
                TestRecorder<TestContext> recorder;
				TestBadPolicy policy;
				TestContext context;

				MwtExplorer<TestContext> mwt("salt", recorder);
				EpsilonGreedyExplorer<TestContext> explorer(policy, 0.f, (u32)1);

				u32 expected_action = mwt.Choose_Action(explorer, "1001", context);
			)
			COUNT_BAD_CALL
			(
                TestRecorder<TestContext> recorder;
				TestBadPolicy policy;
				TestContext context;

				MwtExplorer<TestContext> mwt("salt", recorder);
				TauFirstExplorer<TestContext> explorer(policy, (u32)0, (u32)1);
				mwt.Choose_Action(explorer, "test", context);
			)
			COUNT_BAD_CALL
			(
                TestRecorder<TestContext> recorder;
				TestContext context;

				vector<unique_ptr<IPolicy<TestContext>>> policies;
				policies.push_back(unique_ptr<IPolicy<TestContext>>(new TestBadPolicy()));
				MwtExplorer<TestContext> mwt("salt", recorder);
				BootstrapExplorer<TestContext> explorer(policies, (u32)1);
				mwt.Choose_Action(explorer, "test", context);
			)
			Assert::AreEqual(3, num_ex);
		}

		TEST_METHOD(Usage_Bad_Scorer)
		{
			int num_ex = 0;

			// Default policy returns action outside valid range
			COUNT_BAD_CALL
			(
				u32 num_actions = 1;
				FixedScorer scorer(num_actions, -1);
                MwtExplorer<TestContext> mwt("salt", TestRecorder<TestContext>());
				GenericExplorer<TestContext> explorer(scorer, num_actions);
				mwt.Choose_Action(explorer, "test", TestContext());
			)
			COUNT_BAD_CALL
			(
				u32 num_actions = 1;
				FixedScorer scorer(num_actions, 0);
                MwtExplorer<TestContext> mwt("salt", TestRecorder<TestContext>());
				GenericExplorer<TestContext> explorer(scorer, num_actions);
				mwt.Choose_Action(explorer, "test", TestContext());
			)
			Assert::AreEqual(2, num_ex);
		}

		TEST_METHOD(Custom_Context)
		{
			int num_actions = 10;
			float epsilon = 0.f; // No randomization
			string unique_key = "1001";

			TestSimplePolicy my_policy(0, num_actions);

			TestSimpleRecorder my_recorder;
			MwtExplorer<SimpleContext> mwt("salt", my_recorder);

			vector<Feature> features;
			features.push_back({ 0.5f, 1 });
			features.push_back({ 1.5f, 6 });
			features.push_back({ -5.3f, 13 });
			SimpleContext custom_context(features);

			EpsilonGreedyExplorer<SimpleContext> explorer(my_policy, epsilon, num_actions);

			u32 chosen_action = mwt.Choose_Action(explorer, unique_key, custom_context);
			Assert::AreEqual((u32)1, chosen_action);

			float expected_probs[1] = { 1.f };

			vector<TestInteraction<SimpleContext>> interactions = my_recorder.Get_All_Interactions();
			Assert::AreEqual(1, (int)interactions.size());

			SimpleContext* returned_context = &interactions[0].Context;

			size_t onf = features.size();
			Feature* of = &features[0];

			vector<Feature>& returned_features = returned_context->Get_Features();
			size_t rnf = returned_features.size();
			Feature* rf = &returned_features[0];

			Assert::AreEqual(rnf, onf);
			for (size_t i = 0; i < rnf; i++)
			{
				Assert::AreEqual(of[i].Id, rf[i].Id);
				Assert::AreEqual(of[i].Value, rf[i].Value);
			}
		}

		TEST_METHOD_INITIALIZE(Test_Initialize)
		{
		}

		TEST_METHOD_CLEANUP(Test_Cleanup)
		{
		}

	private: 

        template <class TContext>
        void Epsilon_Greedy_Random_Context(int num_actions, TContext& my_context, EpsilonGreedyExplorer<TContext>& explorer, TestPolicy<TContext>& my_policy)
        {
            TestRecorder<TContext> my_recorder;
            MwtExplorer<TContext> mwt("salt", my_recorder);

            u32 policy_action = my_policy.Choose_Action(my_context);

            int times_choose = 10000;
            int times_policy_action_chosen = 0;
            for (int i = 0; i < times_choose; i++)
            {
                u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(i), my_context);
                if (chosen_action == policy_action)
                {
                    times_policy_action_chosen++;
                }
            }

            Assert::IsTrue(abs((double)times_policy_action_chosen / times_choose - 0.5) < 0.1);
        }

        template <class TContext>
        void Tau_First_Random_Context(int num_actions, TContext& my_context, TauFirstExplorer<TContext>& explorer)
        {
            TestRecorder<TContext> my_recorder;
            MwtExplorer<TContext> mwt("salt", my_recorder);

            u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(1), my_context);
            chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(2), my_context);

            // Tau expired, did not explore
            chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(3), my_context);
            Assert::AreEqual((u32)10, chosen_action);

            // Only 2 interactions logged, 3rd one should not be stored
            vector<TestInteraction<TContext>> interactions = my_recorder.Get_All_Interactions();
            float expected_probs[2] = { .1f, .1f };
            this->Test_Interactions(interactions, 2, expected_probs);
        }

        template <class TContext>
        void Bootstrap_Random_Context(int num_actions, TContext& my_context, BootstrapExplorer<TContext>& explorer)
        {
            TestRecorder<TContext> my_recorder;

            MwtExplorer<TContext> mwt("c++-test", my_recorder);

            u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(1), my_context);
            chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(2), my_context);

            // Two bags choosing different actions so prob of each is 1/2
            vector<TestInteraction<TContext>> interactions = my_recorder.Get_All_Interactions();
            float expected_probs[2] = { .5f, .5f };
            this->Test_Interactions(interactions, 2, expected_probs);
        }

        template <class TContext>
        void Softmax_Context(int num_actions, TContext& my_context, SoftmaxExplorer<TContext>& explorer)
        {
            u32 NUM_ACTIONS_COVER = 100;
            float C = 5.0f;

            TestRecorder<TContext> my_recorder;

            MwtExplorer<TContext> mwt("salt", my_recorder);

            // Scale C up since we have fewer interactions
            u32 num_decisions = (u32)(num_actions * log(num_actions * 1.0) + log(NUM_ACTIONS_COVER * 1.0 / num_actions) * C * num_actions);
            // The () following the array should ensure zero-initialization
            u32* actions = new u32[num_actions]();
            u32 i;
            for (i = 0; i < num_decisions; i++)
            {
                u32 action = mwt.Choose_Action(explorer, this->Get_Unique_Key(i + 1), my_context);
                // Action IDs are 1-based
                actions[action - 1]++;
            }
            // Ensure all actions are covered
            for (i = 0; i < (u32)num_actions; i++)
            {
                Assert::IsTrue(actions[i] > 0);
            }
            float* expected_probs = new float[num_decisions];
            for (i = 0; i < num_decisions; i++)
            {
                // Our default scorer currently assigns equal weight to each action
                expected_probs[i] = 1.0f / num_actions;
            }
            vector<TestInteraction<TContext>> interactions = my_recorder.Get_All_Interactions();
            this->Test_Interactions(interactions, num_decisions, expected_probs);

            delete actions;
            delete expected_probs;
        }

        template <class TContext>
        void Generic_Context(int num_actions, TContext& my_context, GenericExplorer<TContext>& explorer)
        {
            TestRecorder<TContext> my_recorder;

            MwtExplorer<TContext> mwt("salt", my_recorder);

            u32 chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(1), my_context);
            chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(2), my_context);
            chosen_action = mwt.Choose_Action(explorer, this->Get_Unique_Key(3), my_context);

            vector<TestInteraction<TContext>> interactions = my_recorder.Get_All_Interactions();
            float expected_probs[3] = { .1f, .1f, .1f };
            this->Test_Interactions(interactions, 3, expected_probs);
        }

		// Test end-to-end using StringRecorder with no crash
		template <class Exp>
		void End_To_End(MwtExplorer<SimpleContext>& mwt, Exp& explorer, StringRecorder<SimpleContext>& recorder)
		{
			PRG::prg rand;

			float rewards[10];
			for (int i = 0; i < 10; i++)
			{
				vector<Feature> features;
				for (int j = 0; j < 1000; j++)
				{
					features.push_back({ rand.Uniform_Unit_Interval(), j + 1 });
				}
				SimpleContext c(features);

				mwt.Choose_Action(explorer, to_string(i), c);

				rewards[i] = rand.Uniform_Unit_Interval();
			}

			recorder.Get_Recording();
		}

		template <class Ctx>
		inline void Test_Interactions(vector<TestInteraction<Ctx>> interactions, int num_interactions_expected, float* probs_expected)
		{
			size_t num_interactions = interactions.size();

			Assert::AreEqual(num_interactions_expected, (int)num_interactions);
			for (size_t i = 0; i < num_interactions; i++)
			{
				Assert::AreEqual(probs_expected[i], interactions[i].Probability);
			}
		}

		string Get_Unique_Key(u32 seed)
		{
			PRG::prg rg(seed);

			std::ostringstream unique_key_container;
			unique_key_container << rg.Uniform_Unit_Interval();

			return unique_key_container.str();
		}
	};
}
