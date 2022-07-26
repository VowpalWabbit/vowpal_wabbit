#pragma once

#include "vw.net.native.h"
#include "vw.net.workspace.h"
#include "vw/core/feature_group.h"
#include "vw/core/vw.h"

#include <type_traits>

namespace vw_net_native
{
  typedef vw_net_native::v_iterator_context<namespace_index> namespace_enumerator;
  
  struct feature_enumerator
  {
    const features* feat;
    features::const_iterator it;
    namespace_index ns;
  };
}

extern "C" {
  API example* CreateExample(vw_net_native::workspace_context* workspace);
  API void DeleteExample(example* example);

  API bool IsRingExample(vw_net_native::workspace_context* workspace, example* example);
  API bool IsExampleNewline(example* example);
  API char* ComputeDiffDescriptionExample(vw_net_native::workspace_context* workspace, example* ex1, example* ex2);
  API uint64_t GetExampleNumberOfFeatures(example* example);
  API void EmptyExampleData(vw_net_native::workspace_context* workspace, example* example);
  API void MakeIntoNewlineExample(vw_net_native::workspace_context* workspace, example* example);
  API void MakeLabelDefault(vw_net_native::workspace_context* workspace, example* example);
  API void UpdateExampleWeight(vw_net_native::workspace_context* workspace, example* example);

  API vw_net_native::namespace_enumerator* CreateNamespaceEnumerator(vw_net_native::workspace_context* workspace, example* example);
  API void DeleteNamespaceEnumerator(vw_net_native::namespace_enumerator* it);
  API bool NamespaceEnumeratorMoveNext(vw_net_native::namespace_enumerator* it);
  API void NamespaceEnumeratorReset(vw_net_native::namespace_enumerator* it);
  API namespace_index NamespaceEnumeratorGetNamespace(vw_net_native::namespace_enumerator* it);

  API vw_net_native::feature_enumerator* CreateFeatureEnumerator(vw_net_native::workspace_context* workspace, example* example, namespace_index ns);
  API void DeleteFeatureEnumerator(vw_net_native::feature_enumerator* it);
  API bool FeatureEnumeratorMoveNext(vw_net_native::feature_enumerator* it);
  API void FeatureEnumeratorReset(vw_net_native::feature_enumerator* it);
  API void FeatureEnumeratorGetFeature(vw_net_native::feature_enumerator* it, feature* feature);
  API float FeatureEnumeratorGetFeatureValue(vw_net_native::feature_enumerator* it);
  API feature_index FeatureEnumeratorGetFeatureIndex(vw_net_native::feature_enumerator* it);

  API feature_index GetShiftedWeightIndex(vw_net_native::workspace_context* workspace, example* ex, feature_index weight_index_unshifted);
  API float GetWeight(vw_net_native::workspace_context* workspace, example* ex, feature_index weight_index_unshifted);
  API float GetAuditWeight(vw_net_native::workspace_context* workspace, example* ex, feature_index weight_index_unshifted);
}
