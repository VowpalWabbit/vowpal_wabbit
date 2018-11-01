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
        events_offset.push_back(serialize_eventhub_message(evt, builder));
        --remaining;
      }
      auto events_vector_offset = builder.CreateVector(events_offset);
      TResultObjectBuilder ranking_event_batch_builder(builder);
      ranking_event_batch_builder.add_Events(events_vector_offset);
      auto orc = ranking_event_batch_builder.Finish();
      builder.Finish(orc);

      size_t size = builder.GetSize();
      if (size > _send_high_water_mark) {
        oss.reset();
        batch_serialize_internal<TEvent, TResultObject, TResultObjectBuilder>(oss, remaining, queue, _send_high_water_mark, batch_size / 2);
        return;
      }

      oss.append(builder.GetBufferPointer(), builder.GetSize());
    }

    flatbuffers::Offset<RankingEvent> event_batcher::serialize_eventhub_message(ranking_event& evt, flatbuffers::FlatBufferBuilder& builder) {
      short version = 1;
      auto event_id_offset = builder.CreateString(evt.get_event_id());

      auto action_ids_vector_offset = builder.CreateVector(evt.get_action_ids());
      auto probabilities_vector_offset = builder.CreateVector(evt.get_probabilities());
      auto context_offset = builder.CreateVector(evt.get_context());

      auto model_id_offset = builder.CreateString(evt.get_model_id());

      return VW::Events::CreateRankingEvent(builder, version, event_id_offset, evt.get_defered_action(), action_ids_vector_offset, context_offset, probabilities_vector_offset, model_id_offset);
    }

    flatbuffers::Offset<OutcomeEvent> event_batcher::serialize_eventhub_message(outcome_event& evt, flatbuffers::FlatBufferBuilder& builder) {
      auto event_id_offset = builder.CreateString(evt.get_event_id());
      std::vector<uint8_t> outcome_vector;
      std::string outcome = evt.get_outcome();
      outcome_vector.insert(outcome_vector.begin(), outcome.begin(), outcome.end());
      auto outcome_vector_offset = builder.CreateVector(outcome_vector);
      auto type_offset = builder.CreateString("string");
      return VW::Events::CreateOutcomeEvent(builder, event_id_offset, evt.get_deferred_action(), type_offset, outcome_vector_offset);
    }
  }
}