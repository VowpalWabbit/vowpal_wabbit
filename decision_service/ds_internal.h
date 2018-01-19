#pragma once

#include "ds_api.h"
#include "event_hub_client.h"

#include <vector>
#include <memory>
#include <thread>

#include <cpprest/http_client.h>

#include <boost/lockfree/queue.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace Microsoft {
  namespace DecisionService {
    class DecisionServiceClientInternal {
    private:
      // std::unique_ptr<VowpalWabbitThreadSafe> _pool;
      DecisionServiceConfiguration _config;
      uint64_t _seed_from_app_id;

      boost::lockfree::queue<std::vector<unsigned char>*> _queue;

      std::vector<EventHubClient> _event_hub_interactions;
      EventHubClient _event_hub_observation;

      std::atomic_bool _thread_running;

      std::thread _upload_interaction_thread;
      std::thread _download_model_thread;

      EpsilonGreedyExplorer _default_explorer;

    public:
      DecisionServiceClientInternal(DecisionServiceConfiguration config);

      ~DecisionServiceClientInternal();

      void upload_reward(const char* event_id, const char* reward);

      void enqueue_interaction(RankResponse& rankResponse);

    private:
      void download_model();

      void upload_interactions();

      friend class DecisionServiceClient;
    };

    // TODO: move below!
    class IRanker {
    public:
      virtual ~IRanker() { }

      virtual std::vector<float> rank(const char* context) = 0;
    };

    template<typename TObject>
    class PooledObject {
    private:
      TObject * _val;

    public:
      PooledObject(TObject* obj, int pversion)
        : _val(obj), version(pversion)
      { }

      ~PooledObject()
      {
        if (_val) {
          delete _val;
          _val = nullptr;
        }
      }

      inline TObject* val() { return _val; }

      const int version;
    };

    template<typename TObject, typename TFactory>
    class ObjectPool;

    // RAII guard to handle exception case properly
    template<typename TObject, typename TFactory>
    class PooledObjectGuard {
      ObjectPool<TObject, TFactory>* _pool;
      PooledObject<TObject>* _obj;

    public:
      PooledObjectGuard(ObjectPool<TObject, TFactory>& pool, PooledObject<TObject>* obj)
        : _pool(&pool), _obj(obj)
      { }

      ~PooledObjectGuard() {
        _pool->return_to_pool(_obj);
      }

      // TODO: overload -> operator to make invocation nicer.
      TObject* obj() { return _obj->val(); }
    };

    template<typename TObject, typename TFactory>
    class ObjectPool {
      int version;
      std::vector<PooledObject<TObject>*> pool;
      TFactory* factory;
      boost::mutex _mutex;
      int used_objects;

    public:
      ObjectPool()
        : version(0), factory(nullptr), used_objects(0)
      { }

      ~ObjectPool() {
        // TODO: I'm not sure this is needed as we better not have outstanding
        // make sure we have synchronized access
        boost::unique_lock<boost::mutex> lock(_mutex);

        // delete factory
        if (factory) {
          delete factory;
          factory = nullptr;
        }
        
        // delete each pool object
        for (auto&& obj : pool)
          delete obj;
        pool.clear();

        // TODO: log if used_objects > 0
      }

      PooledObject<TObject>* get_or_create() {
        boost::unique_lock<boost::mutex> lock(_mutex);

        // TODO: introduce exception handleable by swig
        /*if (!factory)
          throw new */

        if (pool.size() == 0) {
          used_objects++;
          return new PooledObject<TObject>((*factory)(), version);
        }

        auto back = pool.back();
        pool.pop_back();

        return back;
      }

      void return_to_pool(PooledObject<TObject>* obj) {
        boost::unique_lock<boost::mutex> lock(_mutex);

        if (version == obj->version) {
          pool.push_back(obj);
          return;
        }

        delete obj;
      }

      // takes owner-ship of factory (and will free using delete)
      void update_factory(TFactory* new_factory) {
        boost::unique_lock<boost::mutex> lock(_mutex);

        // use new factory and delete old one
        if (factory)
          delete factory;
        factory = new_factory;

        // increment the version
        version++;

        // dispose old objects
        for (auto&& obj : pool)
          delete obj;
        pool.clear();
      }
    };
  }
}
