using System;
using Rl.Net;

namespace Rl.Net.Cli {
    static class EntryPoints
    {
        public static void Main(string [] args)
        {
            if (args.Length != 1) {
                Console.Error.WriteLine("Missing path to client.json");
            }

            Rl.Net.EntryPoints.DllMain(args);
        }
    }
}