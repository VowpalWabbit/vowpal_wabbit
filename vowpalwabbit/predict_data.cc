#include "predict_data.h"

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
