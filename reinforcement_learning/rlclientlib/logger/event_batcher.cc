#include "event_batcher.h"

#include "generated/RankingEvent_generated.h"
#include "generated/OutcomeEvent_generated.h"

namespace reinforcement_learning {
  namespace utility {

    event_batcher::event_batcher() = default;

    void event_batcher::batch_serialize(data_buffer& oss, size_t& remaining, event_queue<ranking_event>& queue, size_t _send_high_water_mark)
    {
      batch_serialize_internal<ranking_event, RankingEvent, VW::Events::RankingEventBatchBuilder>(oss, remaining, queue, _send_high_water_mark, _batch_size);
    }
    void event_batcher::batch_serialize(data_buffer& oss, size_t& remaining, event_queue<outcome_event>& queue, size_t _send_high_water_mark)
    {
      batch_serialize_internal<outcome_event, OutcomeEvent, VW::Events::OutcomeEventBatchBuilder>(oss, remaining, queue, _send_high_water_mark, _batch_size);
    }

    template<typename TEvent, typename TResultObject, typename TResultObjectBuilder>
    void event_batcher::batch_serialize_internal(data_buffer& oss, size_t& remaining, event_queue<TEvent>& queue, size_t _send_high_water_mark, size_t batch_size)
    {
      if (_batch_size <= 0) {
        throw("event_batcher::batch_serialize batch_size is 0 - message is too large.");
      }

      std::vector<flatbuffers::Offset<TResultObject>> events_offset;
      flatbuffers::FlatBufferBuilder builder(_send_high_water_mark);
      TEvent evt;
      for (int i = 0; remaining > 0 && oss.size() < _send_high_water_mark; i++) {
        if (i == batch_size) break;
        queue.pop(&evt);
        events_offset.push_back(evt.serialize_eventhub_message(builder));
        --remaining;
      }
      auto events_vector_offset = builder.CreateVector(events_offset);
      TResultObjectBuilder ranking_event_batch_builder(builder);
      ranking_event_batch_builder.add_Events(events_vector_offset);
      auto orc = ranking_event_batch_builder.Finish();
      builder.Finish(orc);

      size_t size = builder.GetSize();
      if (size > _send_high_water_mark) {
        batch_serialize_internal<TEvent, TResultObject, TResultObjectBuilder>(oss, remaining, queue, _send_high_water_mark, batch_size / 2);
        return;
      }

      oss.append(builder.GetBufferPointer(), builder.GetSize());
    }
  }
}