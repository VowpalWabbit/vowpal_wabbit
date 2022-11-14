#pragma once

template <typename T>
struct beacon
{
public:
  using ptype = const uint8_t*;

public:
  template <typename C>
  struct cobeacon
  {
    using beacon = beacon<T>;

    cobeacon(size_t delta) : delta{delta}
    {}
      
    const beacon& chase(C* origin_frame)
    {
      return *reinterpret_cast<const beacon*>(chase_frame(reinterpret_cast<ptype>(origin_frame), delta));
    }

    size_t delta;
  };

  beacon(T* data) : data{data}
  {
  }

  T& get() const
  {
    return *data;
  }

  template <typename C>
  cobeacon<C> unchase(const C* origin_frame) const
  {
    return { unchase_frame(reinterpret_cast<ptype>(this), reinterpret_cast<ptype>(origin_frame)) };
  }

private:
  inline static size_t unchase_frame(ptype beacon_frame, ptype origin_frame) { return origin_frame - beacon_frame; }
  inline static ptype chase_frame(ptype origin_frame, size_t delta)          { return origin_frame - delta; }

  template <typename C>
  friend class cobeacon;

  T* data;
};