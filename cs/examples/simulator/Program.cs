using System;

namespace simulator
{
    class Program
    {
        private static readonly string help_string = "usage: simulator initial_random tot_iter mod_iter reward_seed vw_seed exp_iter num_contexts num_actions ml_args_snips";

        static void Main(string[] args)
        {
            string ml_args = args[0] + " --quiet";

            if (!int.TryParse(args[1], out int initial_random))
            {
                Console.WriteLine(help_string);
                return;
            }

            if (!int.TryParse(args[2], out int tot_iter))
            {
                Console.WriteLine(help_string);
                return;
            }

            if (!int.TryParse(args[3], out int mod_iter))
            {
                Console.WriteLine(help_string);
                return;
            }

            if (!int.TryParse(args[4], out int reward_seed))
            {
                Console.WriteLine(help_string);
                return;
            }

            if (!ulong.TryParse(args[5], out ulong vw_seed))
            {
                Console.WriteLine(help_string);
                return;
            }

            if (!int.TryParse(args[6], out int exp_iter))
            {
                Console.WriteLine(help_string);
                return;
            }

            if (!int.TryParse(args[7], out int num_contexts))
            {
                Console.WriteLine(help_string);
                return;
            }

            if (!int.TryParse(args[8], out int num_actions))
            {
                Console.WriteLine(help_string);
                return;
            }

            string ml_args_snips = "--cb_explore_adf --epsilon .05 --cb_type mtr -l 1e-8 --power_t 0 --quiet";
            if (args.Length > 9)
                ml_args_snips = args[9] + " --quiet";

            VowpalWabbitSimulator.Run(ml_args, initial_random, tot_iter, mod_iter, reward_seed, vw_seed, exp_iter, num_contexts, num_actions, ml_args_snips);
        }
    }
}
