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
                            f.Id = UInt32.Parse(words[0]);
                            if (words.Length == 2)
                                f.Value = float.Parse(words[1]);
                            else
                                f.Value = (float)1.0;
                            featureList.Add(f);
                        }
                    }
                    c = new CONTEXT(featureList.ToArray(), null);
                    contexts.Add(c);
                    
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
                    char[] delims = { '\t' };
                    float[] reward_arr = new float[numActions];
                    string[] reward_strings = line.Split(delims);
                    int i = 0;
                    foreach (string s in reward_strings)
                    {
                        reward_arr[i++] = float.Parse(s);
                        if (i == numActions) break;
                    }
                    rewards.Add(reward_arr);
                }
            }
        }

        public CONTEXT getContext()
        {
            if (contexts.Count == 0)
                ParseContexts();

            if (cur_id < contexts.Count)
                return contexts[cur_id++];
            else
                return null;
        }

        public float getReward(uint action, uint uid)
        {            
            if (rewards.Count == 0)
                ParseRewards();

            //Console.WriteLine("Read {0} rewards, uid = {1}, action = {2}", rewards.Count, uid, action);

            if (uid >= rewards.Count)
                Console.WriteLine("Found illegal uid {0}", uid);

            return rewards[(int)(uid)][action-1];
        }
        
    }

    private static UInt32 ScoreBasedPolicy(float threshold, CONTEXT context)
    {
        
        int score_begin = context.Features.Length - 4;
        float base_val = context.Features[score_begin].Value;
        uint num_action = 1;
        for(int i = 1;i < 4;i++) 
        {
            if (context.Features[score_begin+i].Value >= base_val * threshold)
            {
                num_action += (uint)Math.Pow(2, i - 1);
            }
        }
        return num_action;
    }

    public static void Run()
    {
        MwtExplorer mwt = new MwtExplorer("test");

        uint numActions = 8;
        float epsilon = 0.1f;
        float policyParams = 0.1f;

        mwt.InitializeEpsilonGreedy<float>(epsilon, new StatefulPolicyDelegate<float>(ScoreBasedPolicy), policyParams, numActions);
        IOUtils iou = new IOUtils(@"..\Release\speller-contexts", @"..\Release\speller-rewards");

        CONTEXT c;
        uint uniqueID = 1;
        while ((c = iou.getContext()) != null)
        {
            uint action = mwt.ChooseAction(uniqueID.ToString(), c);
            //Console.WriteLine("Taking action {0} on id {1}", action,uniqueID-1);
            uniqueID++;
        }

        INTERACTION[] interactions = mwt.GetAllInteractions();

        MwtRewardReporter rewardReporter = new MwtRewardReporter(interactions);
        for (uint iInter = 0; iInter < interactions.Length; iInter++)
        {            
            float r = iou.getReward(interactions[iInter].GetAction(),iInter);
            //Console.WriteLine("Got reward on interaction {0} with Action {1} as {2}", iInter, interactions[iInter].ChosenAction,r);
            rewardReporter.ReportReward(interactions[iInter].GetId(), r);
        }

        INTERACTION[] full_interactions = rewardReporter.GetAllInteractions();

        //for (uint iInter = 0; iInter < full_interactions.Length; iInter++)
        //{            
        //    Console.WriteLine("Stored reward on interaction {0} with Action {1} as {2}", iInter, full_interactions[iInter].ChosenAction, full_interactions[iInter].Reward);
        //    Console.WriteLine("Action of default policy on this context = {0}", ScoreBasedPolicy(policyParams, full_interactions[iInter].ApplicationContext));
        //}

        MwtOptimizer mwtopt = new MwtOptimizer(full_interactions, numActions);
        float val = mwtopt.EvaluatePolicy<float>(new StatefulPolicyDelegate<float>(ScoreBasedPolicy), 0.1f);
        if (val == 0)
            Console.WriteLine("ZERO!!");
        Console.WriteLine("Value of default policy = {0}", val);
        Console.WriteLine("Value of default policy and threshold 0.2 = {0}", mwtopt.EvaluatePolicy<float>(new StatefulPolicyDelegate<float>(ScoreBasedPolicy), 0.2f));
        Console.WriteLine("Value of default policy and threshold 0.05 = {0}", mwtopt.EvaluatePolicy<float>(new StatefulPolicyDelegate<float>(ScoreBasedPolicy), 0.05f));
        Console.WriteLine("Value of default policy and threshold 1 = {0}", mwtopt.EvaluatePolicy<float>(new StatefulPolicyDelegate<float>(ScoreBasedPolicy), 1.0f));

        Console.WriteLine("Now we will optimize");
        mwtopt.OptimizePolicyVWCSOAA("model");
        Console.WriteLine("Done with optimization, now we will evaluate the optimized model");
        Console.WriteLine("Value of optimized policy using VW = {0}", mwtopt.EvaluatePolicyVWCSOAA("model"));
        Console.ReadKey();
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
