#include "black_box_tests.h"
#include "..\unit\MWTExploreTests.h"
#include <vector>

void prg_test(Value& v)
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

void hash_test(Value& v)
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
void explore_epsilon_greedy(
    const char* app_id, 
    int policy_type, 
    Value& config_policy, 
    float epsilon, 
    u32 num_actions, 
    vector<string> experimental_unit_ids, 
    vector<Ctx> contexts, 
    const char* output_file)
{
    ofstream out_file(output_file);

    StringRecorder<Ctx> recorder;
    MwtExplorer<Ctx> mwt(std::string(app_id), recorder);

    bool is_variable_action_context = std::is_base_of<IVariableActionContext, Ctx>::value;

    switch (policy_type)
    {
        case 0: // fixed policy
        {
            u32 policy_action = config_policy["Action"].GetUint();
            
            TestPolicy<Ctx> policy;
            policy.Set_Action_To_Choose(policy_action);

            unique_ptr<EpsilonGreedyExplorer<Ctx>> explorer;

            explorer.reset(is_variable_action_context ?
                new EpsilonGreedyExplorer<Ctx>(policy, epsilon) :
                new EpsilonGreedyExplorer<Ctx>(policy, epsilon, num_actions));

            for (size_t i = 0; i < experimental_unit_ids.size(); i++)
            {
                mwt.Choose_Action(*explorer.get(), experimental_unit_ids.at(i), contexts.at(i));
            }

            out_file << recorder.Get_Recording() << endl;

            break;
        }
    }
}

void epsilon_greedy_test(Value& v)
{
    const char* outputFile = v["OutputFile"].GetString();
    const char* app_id = v["AppId"].GetString();
    u32 numActions = v["NumberOfActions"].GetUint();

    //var experimentalUnitIdList = config["ExperimentalUnitIdList"].ToObject<string[]>();
    
    float epsilon = v["Epsilon"].GetDouble();
    //JToken configPolicy = config["PolicyConfiguration"];
    int policy_type = v["PolicyType"].GetInt();

    switch (v["ContextType"].GetInt())
    {
        case 0: // fixed action context
        {
            break;
        }
        case 1: // variable action context
        {
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
            prg_test(d[i]);
            break;
        case 1:
            hash_test(d[i]);
            break;
        case 2:
            epsilon_greedy_test(d[i]);
            break;
        }
    }

    return 0;
}