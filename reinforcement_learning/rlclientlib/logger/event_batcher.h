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

    template<typename TEvent>
    void event_batcher<TEvent>::batch_serialize(data_buffer& oss, size_t remaining, event_queue<TEvent>& queue, size_t _send_high_water_mark) {
      if (std::is_same<TEvent, ranking_event>::value) {
        // TODO: figure out how to determine the batch number
        flatbuffers::Offset<RankingEvent> eventsOffset[1000];
        flatbuffers::FlatBufferBuilder builder(_send_high_water_mark);
        TEvent evt;
        for (int i = 0; remaining > 0 && oss.size() < _send_high_water_mark; i++) {
          queue.pop(&evt);
          ranking_event* r_event = dynamic_cast<ranking_event*>(&evt);
          eventsOffset[i] = (r_event->serialize_eventhub_message(builder));
          --remaining;
        }
        VW::Events::RankingEventBatchBuilder ranking_event_batch_builder(builder);
        ranking_event_batch_builder.add_Events(builder.CreateVector(eventsOffset, 1000));
        auto orc = ranking_event_batch_builder.Finish();
        builder.Finish(orc);
        oss << (char*)builder.GetBufferPointer();
      }
      else if (std::is_same<TEvent, outcome_event>::value) {
        // TODO: figure out how to determine the batch number
        flatbuffers::Offset<OutcomeEvent> eventsOffset[1000];
        flatbuffers::FlatBufferBuilder builder(_send_high_water_mark);
        TEvent evt;
        for (int i = 0; remaining > 0 && oss.size() < _send_high_water_mark; i++) {
          queue.pop(&evt);
          outcome_event* o_event = dynamic_cast<outcome_event*>(&evt);
          eventsOffset[i] = (o_event->serialize_eventhub_message(builder));
          --remaining;
        }
        VW::Events::OutcomeEventBatchBuilder outcome_event_batch_builder(builder);
        outcome_event_batch_builder.add_Events(builder.CreateVector(eventsOffset, 1000));
        auto orc = outcome_event_batch_builder.Finish();
        builder.Finish(orc);
        oss << (char*)builder.GetBufferPointer();
      }
    }
  }
}
