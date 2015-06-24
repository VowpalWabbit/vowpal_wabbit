#include "black_box_tests.h"

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

void explore_test(Value& v)
{

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
            explore_test(d[i]);
            break;
        }
    }

    return 0;
}