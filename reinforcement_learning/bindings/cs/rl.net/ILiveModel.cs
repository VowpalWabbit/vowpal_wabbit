namespace Rl.Net
{
    public interface ILiveModel
    {
        bool TryChooseRank(string actionId, string contextJson, out RankingResponse response, ApiStatus apiStatus = null);
        bool TryChooseRank(string actionId, string contextJson, RankingResponse response, ApiStatus apiStatus = null);
        bool TryInit(ApiStatus apiStatus = null);
        bool TryReportOutcome(string actionId, object outcome, ApiStatus apiStatus = null);
        void Dispose();
    }
}