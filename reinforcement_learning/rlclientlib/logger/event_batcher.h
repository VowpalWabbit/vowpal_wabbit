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
      void batch_serialize(data_buffer& oss, size_t& remaining, event_queue<TEvent>& queue, size_t _send_high_water_mark);
    };

    template<typename TEvent>
    void event_batcher<TEvent>::batch_serialize(data_buffer& oss, size_t& remaining, event_queue<TEvent>& queue, size_t _send_high_water_mark) {
      if (std::is_same<TEvent, ranking_event>::value) {
        // TODO: figure out how to determine the batch number
        std::vector<flatbuffers::Offset<RankingEvent>> events_offset;
        flatbuffers::FlatBufferBuilder builder(_send_high_water_mark);
        TEvent evt;
        for (int i = 0; remaining > 0 && oss.size() < _send_high_water_mark; i++) {
          if (i == 1000) break;
          queue.pop(&evt);
          ranking_event* r_event = dynamic_cast<ranking_event*>(&evt);
          events_offset.push_back(r_event->serialize_eventhub_message(builder));
          --remaining;
        }
        auto events_vector_offset = builder.CreateVector(events_offset);
        VW::Events::RankingEventBatchBuilder ranking_event_batch_builder(builder);
        ranking_event_batch_builder.add_Events(events_vector_offset);
        auto orc = ranking_event_batch_builder.Finish();
        builder.Finish(orc);
        oss.append((char*)builder.GetBufferPointer(), builder.GetSize());
      }
      else if (std::is_same<TEvent, outcome_event>::value) {
        // TODO: figure out how to determine the batch number
        std::vector<flatbuffers::Offset<OutcomeEvent>> events_offset;
        flatbuffers::FlatBufferBuilder builder(_send_high_water_mark);
        TEvent evt;
        for (int i = 0; remaining > 0 && oss.size() < _send_high_water_mark; i++) {
          if (i == 1000) break;
          queue.pop(&evt);
          outcome_event* o_event = dynamic_cast<outcome_event*>(&evt);
          events_offset.push_back(o_event->serialize_eventhub_message(builder));
          --remaining;
        }
        auto events_vector_offset = builder.CreateVector(events_offset);
        VW::Events::OutcomeEventBatchBuilder outcome_event_batch_builder(builder);
        outcome_event_batch_builder.add_Events(events_vector_offset);
        auto orc = outcome_event_batch_builder.Finish();
        builder.Finish(orc);
        oss.append((char*)builder.GetBufferPointer(), builder.GetSize());
      }
    }
  }
}
