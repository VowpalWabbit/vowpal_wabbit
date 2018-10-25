#pragma once
#include "event_queue.h"
#include "ranking_event.h"
#include "utility/data_buffer.h"

namespace reinforcement_learning {
  namespace utility {
    class event_batcher {
    public:
      event_batcher();
      void batch_serialize(data_buffer& oss, size_t& remaining, event_queue<ranking_event>& queue, size_t _send_high_water_mark);
      void batch_serialize(data_buffer& oss, size_t& remaining, event_queue<outcome_event>& queue, size_t _send_high_water_mark);

      template<typename TEvent>
      void batch_serialize(data_buffer& oss, size_t& remaining, event_queue<TEvent>& queue, size_t _send_high_water_mark);

    private:
      size_t _batch_size = 400;
      template<typename TEvent, typename TResultObject, typename TResultObjectBuilder>
      void batch_serialize_internal(data_buffer& oss, size_t& remaining, event_queue<TEvent>& queue, size_t _send_high_water_mark, size_t batch_size);
    };

    template<typename TEvent>
    void event_batcher::batch_serialize(data_buffer& oss, size_t& remaining, event_queue<TEvent>& queue, size_t _send_high_water_mark)
    {
      // This is used by unit_test
      TEvent evt;
      while (remaining > 0 && oss.size() < _send_high_water_mark) {
        queue.pop(&evt);
        event* annoymous_event = dynamic_cast<event*>(&evt);
        std::string event_id = annoymous_event->get_event_id();
        const unsigned char* data = (unsigned char*)(event_id.c_str());
        oss.append((const unsigned char*)data, event_id.length());
        remaining--;
      }
    }
  }
}
