using Rl.Net;
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading;
using System.Threading.Tasks;

namespace Rl.Net.Cli
{
    public interface IDriverStepProvider<out TOutcome> : IEnumerable<IStepContext<TOutcome>>
    {
    }

    public interface IStepContext<out TOutcome>
    {
        string EventId
        {
            get;
        }

        string DecisionContext
        {
            get;
        }

        TOutcome GetOutcome(long actionIndex, IEnumerable<ActionProbability> actionDistribution);
    }

    internal class RunContext
    {
        public RankingResponse ResponseContainer
        {
            get;
        } = new RankingResponse();

        public ApiStatus ApiStatusContainer
        {
            get;
        } = new ApiStatus();
    }

    internal interface IOutcomeReporter<TOutcome>
    {
        bool TryReportOutcome(RunContext runContext, string eventId, TOutcome outcome);
    }

    public class RLDriver : IOutcomeReporter<float>, IOutcomeReporter<string>
    {
        private LiveModel liveModel;

        public RLDriver(LiveModel liveModel)
        {
            this.liveModel = liveModel;
        }

        public TimeSpan StepInterval
        {
            get;
            set;
        } = TimeSpan.FromSeconds(2);

        public void Run<TOutcome>(IDriverStepProvider<TOutcome> stepProvider)
        {
            // TODO: Enable consumers to provider their own outcomeReporter
            IOutcomeReporter<TOutcome> outcomeReporter = this as IOutcomeReporter<TOutcome>;
            if (outcomeReporter == null)
            {
                throw new ArgumentException($"Invalid type argument {typeof(TOutcome).Name}", nameof(TOutcome));
            }

            int stepsCount = 0;
            RunContext runContext = new RunContext();
            foreach (IStepContext<TOutcome> step in stepProvider)
            {
                this.Step(runContext, outcomeReporter, step);

                // TODO: Change this to be a command-line arg
                Thread.Sleep(25);

                if (++stepsCount % 1000 == 0)
                {
                    Console.Out.WriteLine($"Processed {stepsCount} steps.");
                }
            }
        }

        public event EventHandler<ApiStatus> OnError;

        bool IOutcomeReporter<float>.TryReportOutcome(RunContext runContext, string eventId, float outcome)
        {
            return this.liveModel.TryReportOutcome(eventId, outcome, runContext.ApiStatusContainer);
        }

        bool IOutcomeReporter<string>.TryReportOutcome(RunContext runContext, string eventId, string outcome)
        {
            return this.liveModel.TryReportOutcome(eventId, outcome, runContext.ApiStatusContainer);
        }

        private void Step<TOutcome>(RunContext runContext, IOutcomeReporter<TOutcome> outcomeReporter, IStepContext<TOutcome> step)
        {
            string eventId = step.EventId;

            if (!liveModel.TryChooseRank(eventId, step.DecisionContext, runContext.ResponseContainer, runContext.ApiStatusContainer))
            {
                this.SafeRaiseError(runContext.ApiStatusContainer);
            }

            long actionIndex = -1;
            if (!runContext.ResponseContainer.TryGetChosenAction(out actionIndex, runContext.ApiStatusContainer))
            {
                this.SafeRaiseError(runContext.ApiStatusContainer);
            }

            TOutcome outcome = step.GetOutcome(actionIndex, runContext.ResponseContainer.AsEnumerable());
            if (outcome == null)
            {
                return;
            }

            if (!outcomeReporter.TryReportOutcome(runContext, eventId, outcome))
            {
                this.SafeRaiseError(runContext.ApiStatusContainer);
            }
        }

        private void SafeRaiseError(ApiStatus errorStatus)
        {
            EventHandler<ApiStatus> localHandler = this.OnError;
            if (localHandler != null)
            {
                localHandler(this, errorStatus);
            }
        }
    }
}
