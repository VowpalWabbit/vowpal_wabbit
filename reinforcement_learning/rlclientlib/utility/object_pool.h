#pragma once
#include <mutex>
#include <vector>

namespace reinforcement_learning { namespace utility {
  template<typename TObject>
  class pooled_object {
  private:
    TObject * _val;

  public:
    pooled_object(TObject* obj, int pversion)
      : _val(obj), version(pversion)
    { }

    pooled_object(const pooled_object&) = delete;
    pooled_object& operator=(const pooled_object& other) = delete;
    pooled_object(pooled_object&& other) = delete;

    ~pooled_object()
    {
      delete _val;
      _val = nullptr;
    }

    inline TObject* val() { return _val; }

    const int version;
  };

  template<typename TObject, typename TFactory>
  class object_pool;

  // RAII guard to handle exception case properly
  template<typename TObject, typename TFactory>
  class pooled_object_guard {
    object_pool<TObject, TFactory>* _pool;
    pooled_object<TObject>* _obj;

  public:
    pooled_object_guard(object_pool<TObject, TFactory>& pool, pooled_object<TObject>* obj)
      : _pool(&pool), _obj(obj)
    { }

    pooled_object_guard(const pooled_object_guard&) = delete;
    pooled_object_guard& operator=(const pooled_object_guard& other) = delete;
    pooled_object_guard(pooled_object_guard&& other) = delete;

    ~pooled_object_guard() {
      _pool->return_to_pool(_obj);
    }

    TObject* operator->() { return _obj->val(); }
    TObject* get() { return _obj->val(); }
  };

  template<typename TObject, typename TFactory>
  class object_pool {
    int _version;
    std::vector<pooled_object<TObject>*> _pool;
    TFactory* _factory;
    std::mutex _mutex;
    int _used_objects;

  public:
    object_pool(TFactory* factory)
      : _version(0), _factory(factory), _used_objects(0)
    { }

    object_pool(const object_pool&) = delete;
    object_pool& operator=(const object_pool& other) = delete;
    object_pool(object_pool&& other) = delete;

    ~object_pool() {
      std::lock_guard<std::mutex> lock(_mutex);

      // delete factory
      delete _factory;
      _factory = nullptr;

      // delete each pool object
      for (auto&& obj : _pool)
        delete obj;
      _pool.clear();
    }

    pooled_object<TObject>* get_or_create() {
      std::lock_guard<std::mutex> lock(_mutex);

      if (_pool.size() == 0) {
        _used_objects++;
        return new pooled_object<TObject>((*_factory)(), _version);
      }

      auto back = _pool.back();
      _pool.pop_back();

      return back;
    }

    void return_to_pool(pooled_object<TObject>* obj) {
      std::lock_guard<std::mutex> lock(_mutex);

      if (_version == obj->version) {
        _pool.emplace_back(obj);
        return;
      }

      delete obj;
    }

    // takes owner-ship of factory (and will free using delete)
    void update_factory(TFactory* new_factory) {
      std::lock_guard<std::mutex> lock(_mutex);

      // use new factory and delete old one
      if (_factory)
        delete _factory;
      _factory = new_factory;

      // increment the version
      _version++;

      // dispose old objects
      for (auto&& obj : _pool)
        delete obj;
      _pool.clear();
    }
  };
}}
