#include "event_batcher.h"

namespace reinforcement_learning {
  namespace utility {
    template<typename TEvent>
    void event_batcher<TEvent>::batch_serialize(data_buffer& oss, size_t remaining, event_queue<TEvent>& queue, size_t _send_high_water_mark) {
      if (std::is_same<TEvent, ranking_event>) {
        // TODO: figure out how to determine the batch number
        auto eventsOffset = Offset<RankingEvent>[100];
        flatbuffers::FlatBufferBuilder builder(_send_high_water_mark);
        TEvent evt;
        for (int i = 0; remaining > 0 && oss.size() < _send_high_water_mark; i++) {
          _queue.pop(&evt);
          ranking_event r_event = dynamic_cast<ranking_event>(evt);
          eventsOffset[i] = r_event.serialize_eventhub_message(builder);
          --remaining;
        }
        VW::Events::RankingEventBuilder ranking_event_batch_builder(builder);
        ranking_event_batch_builder.add_Events(eventsOffset);
        auto orc = ranking_event_batch_builder.Finish();
        builder.Finish(orc);
        oss << builder.GetBufferPointer();
      }
      else if (std::is_same<TEvent, outcome_event>) {
        // TODO: figure out how to determine the batch number
        auto eventsOffset = Offset<OutcomeEvent>[100];
        flatbuffers::FlatBufferBuilder builder(_send_high_water_mark);
        TEvent evt;
        for (int i = 0; remaining > 0 && oss.size() < _send_high_water_mark; i++) {
          _queue.pop(&evt);
          outcome_event o_event = dynamic_cast<outcome_event>(evt);
          eventsOffset[i] = o_event.serialize_eventhub_message(builder);
          --remaining;
        }
        VW::Events::OutcomeEventBatchBuilder outcome_event_batch_builder(builder);
        outcome_event_batch_builder.add_Events(eventsOffset);
        auto orc = outcome_event_batch_builder.Finish();
        builder.Finish(orc);
        oss << builder.GetBufferPointer();
      }
    }
  }
}