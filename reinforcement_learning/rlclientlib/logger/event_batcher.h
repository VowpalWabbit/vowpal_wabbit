#include "event_queue.h"
#include "ranking_event.h"
#include "generated/RankingEvent_generated.h"
#include "generated/OutcomeEvent_generated.h"

namespace reinforcement_learning {
  namespace utility {
    template<typename TEvent>
    class event_batcher {
    public:
      event_batcher() {};
      void batch_serialize(data_buffer& oss, size_t remaining, event_queue<TEvent>& queue, size_t _send_high_water_mark);
    };
  }
}
