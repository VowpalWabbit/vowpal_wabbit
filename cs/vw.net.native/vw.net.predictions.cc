#include "vw.net.predictions.h"

#include <algorithm>

namespace vw_net_native
{
template <typename T> /* TODO: ideally we should enable_if this only on .NET Blittable types */
inline vw_net_native::dotnet_size_t v_copy_to_managed(
    const v_array<T>& source, T* destination, vw_net_native::dotnet_size_t count)
{
  if (count < source.size())
  {
    return size_to_neg_dotnet_size(source.size());  // Not enough space in destination buffer
  }

  std::copy(source.begin(), source.end(), destination);

  // This downcast is safe, despite being signed-to-unsigned, because we implicitly checked for
  // this overflow above, when comparing against the size of the output array.
  return (vw_net_native::dotnet_size_t)source.size();
}

template <typename T> /* TODO: ideally we should enable_if this only on .NET Blittable types */
inline vw_net_native::dotnet_size_t v_copy_n_to_managed(
    const v_array<T>& source, T* destination, vw_net_native::dotnet_size_t limit)
{
  size_t copied_count = std::min(source.size(), static_cast<size_t>(limit));
  std::copy_n(source.begin(), copied_count, destination);

  // This downcast is safe, despite being signed-to-unsigned, because we implicitly checked for
  // this overflow above, when comparing against the size of the output array.
  return (vw_net_native::dotnet_size_t)copied_count;
}
}  // namespace vw_net_native

API float GetPredictionScalar(VW::example* ex) { return ex->pred.scalar; }

API vw_net_native::scalar_confidence_t GetPredictionScalarConfidence(VW::workspace* vw, VW::example* ex)
{
  vw_net_native::scalar_confidence_t ret;
  ret.value = ex->pred.scalar;
  ret.confidence = ex->confidence;

  return ret;
}

API size_t GetPredictionScalarsCount(VW::workspace* vw, VW::example* ex) { return ex->pred.scalars.size(); }

API vw_net_native::dotnet_size_t GetPredictionScalars(
    VW::workspace* vw, VW::example* ex, float* values, vw_net_native::dotnet_size_t count)
{
  return vw_net_native::v_copy_to_managed(ex->pred.scalars, values, count);
}

API float GetPredictionProb(VW::workspace* vw, VW::example* ex) { return ex->pred.prob; }

API float GetPredictionCostSensitive(VW::workspace* vw, VW::example* ex)
{
  return VW::get_cost_sensitive_prediction(ex);
}

API uint32_t GetPredictionMulticlassClass(VW::workspace* vw, VW::example* ex) { return ex->pred.multiclass; }

API size_t GetPredictionMultilabelCount(VW::workspace* vw, VW::example* ex)
{
  return ex->pred.multilabels.label_v.size();
}

API vw_net_native::dotnet_size_t GetPredictionMultilabel(
    VW::workspace* vw, VW::example* ex, uint32_t* values, vw_net_native::dotnet_size_t count)
{
  return vw_net_native::v_copy_to_managed(ex->pred.multilabels.label_v, values, count);
}

API size_t GetPredictionActionScoresCount(VW::workspace* vw, VW::example* ex) { return ex->pred.a_s.size(); }

API vw_net_native::dotnet_size_t GetPredictionActionScores(
    VW::workspace* vw, VW::example* ex, VW::action_score* values, vw_net_native::dotnet_size_t count)
{
  return vw_net_native::v_copy_to_managed(ex->pred.a_s, values, count);
}

API size_t GetPredictionTopicProbsCount(VW::workspace* vw, VW::example* ex)
{
  return static_cast<size_t>(vw->reduction_state.lda);
}

API vw_net_native::dotnet_size_t GetPredictionTopicProbs(
    VW::workspace* vw, VW::example* ex, float* values, vw_net_native::dotnet_size_t count)
{
  if (count < vw->reduction_state.lda)
  {
    return vw_net_native::size_to_neg_dotnet_size(vw->reduction_state.lda);  // not enough space in the output array
  }

  const v_array<float>& scalars = ex->pred.scalars;

  return vw_net_native::v_copy_n_to_managed(scalars, values, count);
}

API uint32_t GetPredictionActiveMulticlassClass(VW::workspace* vw, VW::example* ex)
{
  return ex->pred.active_multiclass.predicted_class;
}

API size_t GetPredictionActiveMulticlassMoreInfoRequiredClassesCount(VW::workspace* vw, VW::example* ex)
{
  return ex->pred.active_multiclass.more_info_required_for_classes.size();
}

API vw_net_native::dotnet_size_t GetPredictionActiveMulticlassMoreInfoRequiredClasses(
    VW::workspace* vw, VW::example* ex, int32_t* values, vw_net_native::dotnet_size_t count)
{
  // This matches what the C#/CLI implementation does, but it does assume that we never have more
  // classes than can be stored in a 32-bit signed integer. Pretend our destination buffer is the
  // right type.
  uint32_t* unsigned_values = reinterpret_cast<uint32_t*>(values);

  return vw_net_native::v_copy_to_managed(
      ex->pred.active_multiclass.more_info_required_for_classes, unsigned_values, count);
}

API vw_net_native::action_pdf_value_t GetPredictionActionPdfValue(VW::workspace* vw, VW::example* ex)
{
  vw_net_native::action_pdf_value_t ret;
  ret.action = ex->pred.pdf_value.action;
  ret.pdf_value = ex->pred.pdf_value.pdf_value;
  return ret;
}
