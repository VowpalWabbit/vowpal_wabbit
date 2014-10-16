using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.IO;
using MultiWorldTesting;


public class LabDemo
{

    class IOUtils
    {
        public string contextfile, rewardfile;
        public List<CONTEXT> contexts;
        public List<float[]> rewards;
        private const int numActions = 8;
        private static int cur_id = 0;
        public IOUtils(string cfile, string rfile)
        {
            contextfile = cfile;
            rewardfile = rfile;
            contexts = new List<CONTEXT>();
            rewards = new List<float[]>();
        }

        public void ParseContexts()
        {
            Console.WriteLine("Parsing contexts");
            CONTEXT c;
            using (StreamReader sr = new StreamReader(contextfile))
            {
                String line;
                int ex_num = 0;
                while ((line = sr.ReadLine()) != null)
                {
                    //Console.WriteLine(line);
                    char[] delims = { ' ', '\t' };
                    List<FEATURE> featureList = new List<FEATURE>();
                    string[] features = line.Split(delims);
                    foreach (string s in features)
                    {
                        char[] feat_delim = { ':' };
                        string[] words = s.Split(feat_delim);
                        //Console.Write("{0} ", words.Length);
                        if (words.Length >= 1 && words[0] != "")
                        {
                            FEATURE f = new FEATURE();
                            //Console.WriteLine("{0}", words[0]);
                            f.WeightIndex = UInt32.Parse(words[0]);
                            if (words.Length == 2)
                                f.X = float.Parse(words[1]);
                            else
                                f.X = (float)1.0;
                            featureList.Add(f);
                        }
                    }
                    c = new CONTEXT(featureList.ToArray(), null);
                    contexts.Add(c);

                    //Console.WriteLine("Printing Example Number:{0}", ++ex_num);
                    //foreach (MyFeature f in Context)
                    //{
                    //    Console.Write("{0}::{1} ", f.id, f.x);
                    //}
                    //Console.WriteLine("");
                }
            }

            Console.WriteLine("Read {0} contexts", contexts.Count);
        }

        public void ParseRewards()
        {
            Console.WriteLine("Parsing rewards");
            using (StreamReader sr = new StreamReader(rewardfile))
            {
                String line;

                while ((line = sr.ReadLine()) != null)
                {
                    //Console.WriteLine(line);
                    char[] delims = { ' ', '\t' };
                    float[] rewards = new float[numActions];
                    string[] reward_strings = line.Split(delims);
                    int i = 0;
                    foreach (string s in reward_strings)
                    {
                        rewards[i++] = float.Parse(s);
                    }
                    rewardList.Add(rewards);
                }
            }
        }

        public CONTEXT getContext()
        {
            if (contexts.Count == 0)
                ParseContexts();
            else
                Console.WriteLine("current id = {0}, size of list = {1}", cur_id, contexts.Count);

            return contexts[cur_id++];
        }

        public float getReward(uint action, uint uid)
        {
            if (rewardList.Count == 0)
                ParseRewards();

            return rewardList[(int)uid][action];
        }
        
    }

    private static UInt32 ScoreBasedPolicy(float threshold, CONTEXT context)
    {
        
        int score_begin = context.Features.Length - 4;
        float base_val = context.Features[score_begin].X;
        uint num_action = 1;
        for(int i = 1;i < 4;i++) 
        {
            if (context.Features[score_begin+i].X >= base_val * threshold)
            {
                num_action += (uint)Math.Pow(2, i - 1);
            }
        }
        return num_action;
    }

    public static void Run()
    {
        MwtExplorer mwt = new MwtExplorer();

        uint numActions = 8;
        float epsilon = 0.2f;
        float policyParams = 0.5f;

        mwt.InitializeEpsilonGreedy<float>(epsilon, new StatefulPolicyDelegate<float>(ScoreBasedPolicy), policyParams, numActions);
        IOUtils iou = new IOUtils(@"..\Release\speller-contexts", @"..\Release\speller-rewards");

        CONTEXT c;
        uint uniqueID = 1;
        while ((c = iou.getContext()) != null)
        {
            uint action = mwt.ChooseAction(c, uniqueID.ToString());
            Console.WriteLine("Taking action {0} ", action);
            uniqueID++;
        }

        INTERACTION[] interactions = mwt.GetAllInteractions();

        MwtRewardReporter rewardReporter = new MwtRewardReporter(interactions);
        for (uint iInter = 0; iInter < interactions.Length; iInter++)
        {
            float r = iou.getReward(interactions[iInter].ChosenAction, iInter);
            rewardReporter.ReportReward(interactions[iInter].Id, r);
        }

        MwtOptimizer mwtopt = new MwtOptimizer(interactions, numActions);
        Console.WriteLine("Value of default policy = {0} = ",mwtopt.EvaluatePolicy<float>(new StatefulPolicyDelegate<float>(ScoreBasedPolicy), 0.5f));
        Console.WriteLine("Value of default policy and threshold 0.6 = {0} = ", mwtopt.EvaluatePolicy<float>(new StatefulPolicyDelegate<float>(ScoreBasedPolicy), 0.6f));
        Console.WriteLine("Value of default policy and threshold 0.4 = {0} = ", mwtopt.EvaluatePolicy<float>(new StatefulPolicyDelegate<float>(ScoreBasedPolicy), 0.4f));
    }

    private static CONTEXT GetContext()
    {
        return null;
    }

    private static List<float[]> rewardList;
    private float GetReward(uint action, uint uniqueID)
    {
        if (rewardList == null)
        {
            rewardList = new List<float[]>();
            // Read reward from file and populate the list
        }
        // 
        return rewardList[(int)uniqueID][action];
    }

    private static void DoAction(uint action, uint uniqueID)
    {
        // Performs the action
    }

    
}
