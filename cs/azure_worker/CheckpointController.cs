// --------------------------------------------------------------------------------------------------------------------
// <copyright file="CheckpointController.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using System.Net;
using System.Net.Http;
using System.Threading.Tasks;
using VW.Azure.Trainer;

namespace VW.Azure.Worker
{
    /// <summary>
    /// HTTP Front end to trigger checkpointing
    /// </summary>
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
