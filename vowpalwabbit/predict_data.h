#pragma once

/*
 * class predict_data
 * Description:
 *   This data structure manages access and lifetime of the features data used in various VW reductions.
 *
 *   All contained data types MUST instantiate a `clear()` function that readies the object for reuse.
 *
 *   Calling create() on an existing element will clear and overwrite the existing data
 *
 * Member functions:
 *   T& get<T>();
 *   const T& get<T>() const;
 *   void clear();
 *
 * Data structure usage:
 *   Accessing an instance of existing data:
 *     features_data fd;
 *     auto& data = fd.get<data_type>();
 */

/////////////////////////////////////////////
// sample placeholder structs. For demonstrative purposes only
struct basic_predict_data
{
  basic_predict_data() : x(0) {}
  int x;
  void clear();
};
namespace CCB{
struct predict_data
{
  int x;
  float y;
  double z;
  void clear();
};
}
/////////////////////////////////////////////

class predict_data
{
 private:
  basic_predict_data _basic_predict_data;
  CCB::predict_data _ccb_predict_data;

 public:
  template <typename T>
  T& get();
  template <typename T>
  const T& get() const;

  // call clear() on all instantiated types
  void clear() {
    _basic_predict_data.clear();
    _ccb_predict_data.clear();
  }
};

template<> 
basic_predict_data& predict_data::get<basic_predict_data>()
{
  return _basic_predict_data;
}
template <>
const basic_predict_data& predict_data::get<basic_predict_data>() const
{
  return _basic_predict_data;
}

template <>
CCB::predict_data& predict_data::get<CCB::predict_data>()
{
  return _ccb_predict_data;
}
template <>
const CCB::predict_data& predict_data::get<CCB::predict_data>() const
{
  return _ccb_predict_data;
}
