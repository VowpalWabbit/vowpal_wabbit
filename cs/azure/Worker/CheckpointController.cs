using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using VowpalWabbit.Azure.Trainer;

namespace VowpalWabbit.Azure.Worker
{
    public sealed class CheckpointController : OnlineTrainerController
    {
        public CheckpointController(LearnEventProcessorHost trainProcessorFactory)
            : base(trainProcessorFactory)
        {
        }

        public async Task<HttpResponseMessage> Get()
        {
            if (!this.TryAuthorize())
                return this.Request.CreateResponse(HttpStatusCode.Unauthorized);

            await this.trainProcessorHost.CheckpointAsync();

            return this.Request.CreateResponse(HttpStatusCode.OK);
        }
    }
}
