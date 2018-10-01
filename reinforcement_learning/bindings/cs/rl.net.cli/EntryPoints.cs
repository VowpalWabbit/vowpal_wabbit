using System;
using System.IO;
using Rl.Net;

namespace Rl.Net.Cli {
    static class EntryPoints
    {
        public static void Main(string [] args)
        {
            BasicUsageExample(args);
        }

        private static void WriteErrorAndExit(string errorMessage, int exitCode = -1)
        {
            Console.Error.WriteLine(errorMessage);
            Environment.Exit(exitCode);
        }

        private static void WriteStatusAndExit(ApiStatus apiStatus)
        {
            WriteErrorAndExit(apiStatus.ErrorMessage);
        }

        // TODO: Pull this out to a separate sample once we implement the simulator in this.
        public static void BasicUsageExample(string [] args)
        {
            const float outcome = 1.0f;
            const string eventId = "event_id";
            const string contextJson = "{'GUser':{'id':'a','major':'eng','hobby':'hiking'},'_multi':[ { 'TAction':{'a1':'f1'} },{'TAction':{'a2':'f2'}}]}";

            if (args.Length != 1) 
            {
                WriteErrorAndExit("Missing path to client configuration json");
            }

            if (!File.Exists(args[0]))
            {
                WriteErrorAndExit($"Could not find file with path '{args[0]}'.");
            }

            string json = File.ReadAllText(args[0]);

            ApiStatus apiStatus = new ApiStatus();

            Configuration config;
            if (!Configuration.TryLoadConfigurationFromJson(json, out config, apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            LiveModel liveModel = new LiveModel(config);
            if (!liveModel.TryInit(apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            RankingResponse rankingResponse = new RankingResponse();
            if (!liveModel.TryChooseRank(eventId, contextJson, rankingResponse, apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            long actionId;
            if (!rankingResponse.TryGetChosenAction(out actionId, apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }

            Console.WriteLine($"Chosen action id: {actionId}");

            if (!liveModel.TryReportOutcome(eventId, outcome, apiStatus))
            {
                WriteStatusAndExit(apiStatus);
            }
        }
    }
}