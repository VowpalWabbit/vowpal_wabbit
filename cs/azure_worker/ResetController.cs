// --------------------------------------------------------------------------------------------------------------------
// <copyright file="ResetController.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Newtonsoft.Json;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Net;
using System.Net.Http;
using System.Text;
using System.Threading.Tasks;
using System.Web.Http;
using VW.Azure.Trainer;

namespace VW.Azure.Worker
{
    public sealed class ResetController : OnlineTrainerController
    {
        public ResetController(LearnEventProcessorHost trainProcessorFactory)
            : base(trainProcessorFactory)
        {
        }

        /// <summary>
        /// Vanilla reset.
        /// </summary>
        [HttpGet]
        public async Task<HttpResponseMessage> Get()
        {
            if (!this.TryAuthorize())
                return this.Request.CreateResponse(HttpStatusCode.Unauthorized);

            try
            {
                await this.trainProcessorHost.ResetModelAsync();

                return this.Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex);
                return this.Request.CreateResponse(HttpStatusCode.InternalServerError, ex.Message);
            }
        }

        /// <summary>
        /// Reset optionally include EventHub position.
        /// </summary>
        [HttpPost]
        public async Task<HttpResponseMessage> Post()
        {
            if (!this.TryAuthorize())
                return this.Request.CreateResponse(HttpStatusCode.Unauthorized);

            try
            {
                OnlineTrainerState state = null;
                var body = await Request.Content.ReadAsStringAsync();
                if (!string.IsNullOrWhiteSpace(body))
                    state = JsonConvert.DeserializeObject<OnlineTrainerState>(body);

                await this.trainProcessorHost.ResetModelAsync(state);

                return this.Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex);
                return this.Request.CreateResponse(HttpStatusCode.InternalServerError, ex.Message);
            }
        }

        /// <summary>
        /// Reset including a warm started model.
        /// </summary>
        [HttpPut]
        public async Task<HttpResponseMessage> Put()
        {
            if (!this.TryAuthorize())
                return this.Request.CreateResponse(HttpStatusCode.Unauthorized);

            try
            {
                var model = await Request.Content.ReadAsByteArrayAsync();

                await this.trainProcessorHost.ResetModelAsync(model: model);

                return this.Request.CreateResponse(HttpStatusCode.OK);
            }
            catch (Exception ex)
            {
                this.telemetry.TrackException(ex);
                return this.Request.CreateResponse(HttpStatusCode.InternalServerError, ex.Message);
            }
        }
    }
}
