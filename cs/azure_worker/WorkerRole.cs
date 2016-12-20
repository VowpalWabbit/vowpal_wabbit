// --------------------------------------------------------------------------------------------------------------------
// <copyright file="WorkerRole.cs">
//   Copyright (c) by respective owners including Yahoo!, Microsoft, and
//   individual contributors. All rights reserved.  Released under a BSD
//   license as described in the file LICENSE.
// </copyright>
// --------------------------------------------------------------------------------------------------------------------

using Microsoft.ApplicationInsights;
using Microsoft.ApplicationInsights.DataContracts;
using Microsoft.ApplicationInsights.Extensibility;
using Microsoft.Azure;
using Microsoft.Owin.Cors;
using Microsoft.Owin.Hosting;
using Microsoft.Practices.Unity;
using Microsoft.WindowsAzure.ServiceRuntime;
using Owin;
using System;
using System.Diagnostics;
using System.Net;
using System.Threading;
using System.Web.Http;
using System.Web.Http.Cors;
using Unity.WebApi;
using VW.Azure.Trainer;

namespace VW.Azure.Worker
{
    public class WorkerRole : RoleEntryPoint
    {
        private readonly ManualResetEventSlim stopEvent;
        private IDisposable webApp;
        private TelemetryClient telemetry;
        private OnlineTrainerSettingsWatcher settingsWatcher;
        private LearnEventProcessorHost trainProcesserHost;

        public WorkerRole()
        {
            this.stopEvent = new ManualResetEventSlim();
        }

        public override bool OnStart()
        {
            try
            {
                // Set the maximum number of concurrent connections
                ServicePointManager.DefaultConnectionLimit = 128;

                // For information on handling configuration changes
                // see the MSDN topic at http://go.microsoft.com/fwlink/?LinkId=166357.

                bool result = base.OnStart();

                TelemetryConfiguration.Active.InstrumentationKey = CloudConfigurationManager.GetSetting("APPINSIGHTS_INSTRUMENTATIONKEY");
                //TelemetryConfiguration.Active.TelemetryChannel.DeveloperMode = true;
                this.telemetry = new TelemetryClient();

                try
                {

                    this.telemetry.TrackTrace("WorkerRole starting", SeverityLevel.Information);

                    this.trainProcesserHost = new LearnEventProcessorHost();
                    this.settingsWatcher = new OnlineTrainerSettingsWatcher(this.trainProcesserHost);

                    this.StartRESTAdminEndpoint();
                }
                catch (Exception e)
                {
                    this.telemetry.TrackException(e);
                    // still start to give AppInsights a chance to log
                }
                return result;
            }
            catch (Exception e)
            {
                Debugger.Log(1, "ERROR", $"VowpalWabbit.AzureWorker failed to start: {e.Message} {e.StackTrace}");
                throw;
            }
        }

        public override void Run()
        {
            try
            {
                // wait for OnStop
                this.stopEvent.Wait();
            }
            catch (Exception e)
            {
                this.telemetry.TrackException(e);
            }
        }

        private void StartRESTAdminEndpoint()
        {
            // setup REST endpoint
            var endpoint = RoleEnvironment.CurrentRoleInstance.InstanceEndpoints["OnlineTrainer"];
            string baseUri = String.Format("{0}://{1}",
                endpoint.Protocol, endpoint.IPEndpoint);

            this.webApp = WebApp.Start(baseUri, app =>
            {
                var container = new UnityContainer();
                
                // Register controller
                container.RegisterType<ResetController>();
                container.RegisterType<CheckpointController>();
                container.RegisterType<StatusController>();

                // Register interface
                container.RegisterInstance(typeof(LearnEventProcessorHost), this.trainProcesserHost);

                var config = new HttpConfiguration();
                config.DependencyResolver = new UnityDependencyResolver(container);
                config.EnableCors(new EnableCorsAttribute("*", "*", "*"));
                config.Routes.MapHttpRoute(
                    "Default",
                    "{controller}/{id}",
                    new { id = RouteParameter.Optional });

                // config.Services.Add(typeof(IExceptionLogger), new AiWebApiExceptionLogger());

                app.UseCors(CorsOptions.AllowAll);
                app.UseWebApi(config);
            });
        }

        public override void OnStop()
        {
            this.telemetry.TrackTrace("WorkerRole stopping", SeverityLevel.Information);

            try
            {
                this.stopEvent.Set();

                if (this.settingsWatcher != null)
                {
                    this.settingsWatcher.Dispose();
                    this.settingsWatcher = null;
                }

                if (this.trainProcesserHost != null)
                {
                    this.trainProcesserHost.Dispose();
                    this.trainProcesserHost = null;
                }

                if (this.webApp != null)
                {
                    this.webApp.Dispose();
                    this.webApp = null;
                }

                base.OnStop();
            }
            catch (Exception e)
            {
                this.telemetry.TrackException(e);
            }

            this.telemetry.TrackTrace("WorkerRole stopped", SeverityLevel.Information);
        }
    }
}
