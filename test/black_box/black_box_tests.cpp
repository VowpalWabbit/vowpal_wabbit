#include "black_box_tests.h"
#include "..\unit\MWTExploreTests.h"
#include <vector>

void test_prg(Value& v)
{
    u64 seed = v["Seed"].GetUint64();
    int iterations = v["Iterations"].GetInt();

    int uniformIntervalStart = -1;
    int uniformIntervalEnd = -1;
    if (!v["UniformInterval"].IsNull())
    {
        uniformIntervalStart = v["UniformInterval"]["Item1"].GetInt();
        uniformIntervalEnd = v["UniformInterval"]["Item2"].GetInt();
    }

    const char* outputFile = v["OutputFile"].GetString();

    PRG::prg rg(seed);

    ofstream out_file(outputFile);

    for (int i = 0; i < iterations; i++)
    {
        if (uniformIntervalStart != -1 && uniformIntervalEnd != -1)
        {
            u32 random = rg.Uniform_Int(uniformIntervalStart, uniformIntervalEnd);
            out_file << random << endl;
        }
        else
        {
            float random = rg.Uniform_Unit_Interval();
            out_file << random << endl;
        }
    }
}

void test_hash(Value& v)
{
    const char* outputFile = v["OutputFile"].GetString();

    ofstream out_file(outputFile);

    Value& values = v["Values"];
    for (SizeType i = 0; i < values.Size(); i++)
    {
        string value(values[i].GetString());

        out_file << HashUtils::Compute_Id_Hash(value) << endl;
    }
}

template <typename Ctx>
EpsilonGreedyExplorer<Ctx>* CreateEpsilonExplorer(IPolicy<Ctx>& policy, float epsilon, u32 num_actions, std::true_type)
{
    return new EpsilonGreedyExplorer<Ctx>(policy, epsilon);
}

template <typename Ctx>
EpsilonGreedyExplorer<Ctx>* CreateEpsilonExplorer(IPolicy<Ctx>& policy, float epsilon, u32 num_actions, std::false_type)
{
    return new EpsilonGreedyExplorer<Ctx>(policy, epsilon, num_actions);
}

template <typename Ctx>
TauFirstExplorer<Ctx>* CreateTauFirstExplorer(IPolicy<Ctx>& policy, u32 tau, u32 num_actions, std::true_type)
{
    return new TauFirstExplorer<Ctx>(policy, tau);
}

template <typename Ctx>
TauFirstExplorer<Ctx>* CreateTauFirstExplorer(IPolicy<Ctx>& policy, u32 tau, u32 num_actions, std::false_type)
{
    return new TauFirstExplorer<Ctx>(policy, tau, num_actions);
}

template <typename Ctx>
void explore_epsilon_greedy(
    const char* app_id, 
    int policy_type, 
    Value& config_policy, 
    float epsilon, 
    u32 num_actions, 
    Value& experimental_unit_ids, 
    vector<Ctx> contexts, 
    const char* output_file)
{
    ofstream out_file(output_file);

    StringRecorder<Ctx> recorder;
    MwtExplorer<Ctx> mwt(std::string(app_id), recorder);

    switch (policy_type)
    {
        case 0: // fixed policy
        {
            u32 policy_action = config_policy["Action"].GetUint();
            
            TestPolicy<Ctx> policy;
            policy.Set_Action_To_Choose(policy_action);

            unique_ptr<EpsilonGreedyExplorer<Ctx>> explorer;

            explorer.reset(CreateEpsilonExplorer(policy, epsilon, num_actions, std::is_base_of<IVariableActionContext, Ctx>()));

            for (SizeType i = 0; i < experimental_unit_ids.Size(); i++)
            {
                mwt.Choose_Action(*explorer.get(), string(experimental_unit_ids[i].GetString()), contexts.at(i));
            }

            out_file << recorder.Get_Recording() << endl;

            break;
        }
    }
}

template <typename Ctx>
void explore_tau_first(
    const char* app_id,
    int policy_type,
    Value& config_policy,
    u32 tau,
    u32 num_actions,
    Value& experimental_unit_ids,
    vector<Ctx> contexts,
    const char* output_file)
{
    ofstream out_file(output_file);

    StringRecorder<Ctx> recorder;
    MwtExplorer<Ctx> mwt(std::string(app_id), recorder);

    switch (policy_type)
    {
    case 0: // fixed policy
    {
        u32 policy_action = config_policy["Action"].GetUint();

        TestPolicy<Ctx> policy;
        policy.Set_Action_To_Choose(policy_action);

        unique_ptr<TauFirstExplorer<Ctx>> explorer;

        explorer.reset(CreateTauFirstExplorer(policy, tau, num_actions, std::is_base_of<IVariableActionContext, Ctx>()));

        for (SizeType i = 0; i < experimental_unit_ids.Size(); i++)
        {
            mwt.Choose_Action(*explorer.get(), string(experimental_unit_ids[i].GetString()), contexts.at(i));
        }

        out_file << recorder.Get_Recording() << endl;

        break;
    }
    }
}

void test_epsilon_greedy(Value& v)
{
    const char* output_file = v["OutputFile"].GetString();
    const char* app_id = v["AppId"].GetString();
    u32 num_actions = v["NumberOfActions"].GetUint();

    Value& experimental_unit_id_list = v["ExperimentalUnitIdList"];
    
    float epsilon = v["Epsilon"].GetDouble();
    Value& config_policy = v["PolicyConfiguration"];
    int policy_type = config_policy["PolicyType"].GetInt();

    switch (v["ContextType"].GetInt())
    {
        case 0: // fixed action context
        {
            vector<TestContext> contexts;
            for (SizeType i = 0; i < experimental_unit_id_list.Size(); i++)
            {
                TestContext tc;
                tc.Id = i;
                contexts.push_back(tc);
            }

            explore_epsilon_greedy(app_id, policy_type, config_policy, epsilon, num_actions, experimental_unit_id_list, contexts, output_file);

            break;
        }
        case 1: // variable action context
        {
            vector<TestVarContext> contexts;
            for (SizeType i = 0; i < experimental_unit_id_list.Size(); i++)
            {
                TestVarContext tc(num_actions);
                tc.Id = i;
                contexts.push_back(tc);
            }

            explore_epsilon_greedy(app_id, policy_type, config_policy, epsilon, num_actions, experimental_unit_id_list, contexts, output_file);

            break;
        }
    }
}

// TODO: refactor
void test_tau_first(Value& v)
{
    const char* output_file = v["OutputFile"].GetString();
    const char* app_id = v["AppId"].GetString();
    u32 num_actions = v["NumberOfActions"].GetUint();

    Value& experimental_unit_id_list = v["ExperimentalUnitIdList"];

    u32 tau = v["Tau"].GetUint();
    Value& config_policy = v["PolicyConfiguration"];
    int policy_type = config_policy["PolicyType"].GetInt();

    switch (v["ContextType"].GetInt())
    {
    case 0: // fixed action context
    {
        vector<TestContext> contexts;
        for (SizeType i = 0; i < experimental_unit_id_list.Size(); i++)
        {
            TestContext tc;
            tc.Id = i;
            contexts.push_back(tc);
        }

        explore_tau_first(app_id, policy_type, config_policy, tau, num_actions, experimental_unit_id_list, contexts, output_file);

        break;
    }
    case 1: // variable action context
    {
        vector<TestVarContext> contexts;
        for (SizeType i = 0; i < experimental_unit_id_list.Size(); i++)
        {
            TestVarContext tc(num_actions);
            tc.Id = i;
            contexts.push_back(tc);
        }

        explore_tau_first(app_id, policy_type, config_policy, tau, num_actions, experimental_unit_id_list, contexts, output_file);

        break;
    }
    }
}

int _tmain(int argc, _TCHAR* argv[])
{
    if (argc != 2)
    {
        return 0;
    }

    ifstream json_file(argv[1]);
    string json_file_content;
    while (json_file >> json_file_content) { }

    Document d;
    d.Parse(json_file_content.c_str());

    for (SizeType i = 0; i < d.Size(); i++)
    {
        switch (d[i]["Type"].GetInt())
        {
        case 0:
            test_prg(d[i]);
            break;
        case 1:
            test_hash(d[i]);
            break;
        case 2:
            test_epsilon_greedy(d[i]);
            break;
        case 3:
            test_tau_first(d[i]);
            break;
        }
    }

    return 0;
}