using OnlineTrainer.Net;

namespace onlinetrainer.net.cli
{
    class Program
    {
        private static VWOnlineTrainer CreateVWOnlineTrainerOrExit(string args)
        {
            return new VWOnlineTrainer(args);
        }
        public static void Main(string[] args)
        {
            VWOnlineTrainer onlineTrainer = CreateVWOnlineTrainerOrExit(args[0]);
        }
    }
}
