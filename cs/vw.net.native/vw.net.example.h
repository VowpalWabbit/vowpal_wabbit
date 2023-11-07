#pragma once

#include "vw.net.native.h"
#include "vw.net.workspace.h"
#include "vw/core/feature_group.h"
#include "vw/core/vw.h"

#include <type_traits>

namespace vw_net_native
{
using namespace_enumerator = vw_net_native::v_iterator_context<VW::namespace_index>;

struct feature_enumerator
{
  const VW::features* feat;
  VW::features::const_iterator it;
  VW::namespace_index ns;
};
}  // namespace vw_net_native

extern "C"
{
  API VW::example* CreateExample(vw_net_native::workspace_context* workspace);
  API void DeleteExample(example* example);

  // This function is deprecated and will be removed in VW 10.
  API int IsRingExample(vw_net_native::workspace_context* workspace, VW::example* example);
  API int IsExampleNewline(VW::example* example);
  API char* ComputeDiffDescriptionExample(
      vw_net_native::workspace_context* workspace, VW::example* ex1, VW::example* ex2);
  API uint64_t GetExampleNumberOfFeatures(VW::example* example);
  API void EmptyExampleData(vw_net_native::workspace_context* workspace, VW::example* example);
  API void MakeIntoNewlineExample(vw_net_native::workspace_context* workspace, VW::example* example);
  API void MakeLabelDefault(vw_net_native::workspace_context* workspace, VW::example* example);
  API void UpdateExampleWeight(vw_net_native::workspace_context* workspace, VW::example* example);

  API vw_net_native::namespace_enumerator* CreateNamespaceEnumerator(
      vw_net_native::workspace_context* workspace, VW::example* example);
  API void DeleteNamespaceEnumerator(vw_net_native::namespace_enumerator* it);
  API int NamespaceEnumeratorMoveNext(vw_net_native::namespace_enumerator* it);
  API void NamespaceEnumeratorReset(vw_net_native::namespace_enumerator* it);
  API VW::namespace_index NamespaceEnumeratorGetNamespace(vw_net_native::namespace_enumerator* it);

  API vw_net_native::feature_enumerator* CreateFeatureEnumerator(
      vw_net_native::workspace_context* workspace, VW::example* example, VW::namespace_index ns);
  API void DeleteFeatureEnumerator(vw_net_native::feature_enumerator* it);
  API int FeatureEnumeratorMoveNext(vw_net_native::feature_enumerator* it);
  API void FeatureEnumeratorReset(vw_net_native::feature_enumerator* it);
  API void FeatureEnumeratorGetFeature(vw_net_native::feature_enumerator* it, VW::feature* feature);
  API float FeatureEnumeratorGetFeatureValue(vw_net_native::feature_enumerator* it);
  API VW::feature_index FeatureEnumeratorGetFeatureIndex(vw_net_native::feature_enumerator* it);

  API VW::feature_index GetShiftedWeightIndex(
      vw_net_native::workspace_context* workspace, VW::example* ex, VW::feature_index weight_index_unshifted);
  API float GetWeight(
      vw_net_native::workspace_context* workspace, VW::example* ex, VW::feature_index weight_index_unshifted);
  API float GetAuditWeight(
      vw_net_native::workspace_context* workspace, VW::example* ex, VW::feature_index weight_index_unshifted);
}
