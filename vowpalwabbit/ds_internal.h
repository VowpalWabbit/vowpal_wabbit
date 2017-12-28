#pragma once

//#include "vw.h"
#include <vector>
#include <boost/thread/mutex.hpp>
#include <boost/thread/lock_guard.hpp>

namespace ds {
  struct ActionProbability {
    int action;
    float probability;
  };

  class IRanker {
  public:
    virtual std::vector<ActionProbability> rank(const char* context) = 0;
    virtual ~IRanker() { }
  };

  template<typename TObject>
  class PooledObject {
  private:
    TObject* _val;

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
