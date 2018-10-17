using OnlineTrainer.Net;
using System.Text;

namespace onlinetrainer.net.cli
{
    class Program
    {
        private static VWOnlineTrainer CreateVWOnlineTrainerOrExit(string args, byte[] model = null)
        {
            return new VWOnlineTrainer(args, model);
        }
        public static void Main(string[] args)
        {
            VWOnlineTrainer onlineTrainer = CreateVWOnlineTrainerOrExit(args[0], Encoding.ASCII.GetBytes("Test Model File"));
        }
    }
}
