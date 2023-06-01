#include "vw.net.example.h"

#include "vw/core/feature_group.h"
#include "vw/core/reductions/gd.h"
#include "vw/core/shared_data.h"
#include "vw/text_parser/parse_example_text.h"

#include <sstream>

API VW::example* CreateExample(vw_net_native::workspace_context* workspace)
{
  auto* ex = new VW::example;
  workspace->vw->parser_runtime.example_parser->lbl_parser.default_label(ex->l);
  return ex;
}

API void DeleteExample(VW::example* example) { delete example; }

API int IsRingExample(vw_net_native::workspace_context* workspace, VW::example* example)
{
  return VW::is_ring_example(*workspace->vw, example);
}

API int IsExampleNewline(VW::example* example) { return VW::example_is_newline(*example) != 0; }

inline void format_indicies(VW::example* a, std::stringstream& sstream)
{
  for (auto ns : a->indices)
  {
    if (ns == 0) { sstream << "NULL:0"; }
    else { sstream << '\'' << static_cast<char>(ns) << "\':" << ns << ','; }
  }
}

char* FormatIndicies(VW::example* a, VW::example* b)
{
  std::stringstream sstream;

  sstream << "Namespace indicies differ: " << a->indices.size() << " vs. " << b->indices.size() << ". this.indicies [";

  format_indicies(a, sstream);
  sstream << "] other.indicies [";
  format_indicies(b, sstream);
  sstream << ']';

  return vw_net_native::stringstream_to_cstr(sstream);
}

inline void format_feature(
    vw_net_native::workspace_context* workspace, VW::feature_value x, VW::feature_index i, std::stringstream& sstream)
{
  VW::feature_index masked_weight_index = i & workspace->vw->weights.mask();
  sstream << "weight_index = " << masked_weight_index << '/' << i << ", x = " << x;
}

inline void format_feature(vw_net_native::workspace_context* workspace, VW::feature_value x1, VW::feature_index i1,
    VW::feature_value x2, VW::feature_index i2, std::stringstream& sstream)
{
  sstream << "Feature differ: this(";
  format_feature(workspace, x1, i1, sstream);
  sstream << ") vs other(";
  format_feature(workspace, x2, i2, sstream);
  sstream << ')';
}

char* format_features(vw_net_native::workspace_context* workspace, VW::features& feature_data)
{
  std::stringstream sstream;
  for (size_t i = 0; i < feature_data.values.size(); i++)
  {
    format_feature(workspace, feature_data.values[i], feature_data.indices[i], sstream);
    sstream << ' ';
  }

  return vw_net_native::stringstream_to_cstr(sstream);
}

char* compare_features(
    vw_net_native::workspace_context* workspace, VW::features& fa, VW::features& fb, VW::namespace_index ns)
{
  std::vector<size_t> fa_missing;
  for (size_t ia = 0, ib = 0; ia < fa.values.size(); ia++)
  {
    VW::feature_index masked_weight_index = fa.indices[ia] & workspace->vw->weights.mask();
    VW::feature_index other_masked_weight_index = fb.indices[ib] & workspace->vw->weights.mask();

    if (masked_weight_index == other_masked_weight_index && vw_net_native::FloatEqual(fa.values[ia], fb.values[ib]))
    {
      ib++;
    }
    else
    {
      // fallback to search
      size_t ib_old = ib;
      bool found = false;
      for (ib = 0; ib < fb.values.size(); ib++)
      {
        auto other_masked_weight_index = fb.indices[ib] & workspace->vw->weights.mask();
        if (masked_weight_index == other_masked_weight_index)
        {
          if (!vw_net_native::FloatEqual(fa.values[ia], fb.values[ib]))
          {
            std::stringstream sstream;
            format_feature(workspace, fa.values[ia], fa.indices[ia], fb.values[ib], fb.indices[ib], sstream);
            return vw_net_native::stringstream_to_cstr(sstream);
          }
          else
          {
            found = true;
            break;
          }
        }
      }

      if (!found) { fa_missing.push_back(ia); }

      ib = ib_old + 1;
    }
  }

  if (!fa_missing.empty())
  {
    std::stringstream sstream;
    sstream << "missing features in ns " << ns << "\'/\'" << static_cast<char>(ns) << "\': ";

    for (auto ia : fa_missing)
    {
      sstream << "this.weight_index = " << (fa.indices[ia] & workspace->vw->weights.mask()) << ", x = " << fa.values[ia]
              << ", ";
    }

    return vw_net_native::stringstream_to_cstr(sstream);
  }

  return nullptr;
}

API char* ComputeDiffDescriptionExample(vw_net_native::workspace_context* workspace, VW::example* a, VW::example* b)
{
  if (a->indices.size() != b->indices.size()) { return FormatIndicies(a, b); }

  for (auto i = a->indices.begin(), j = b->indices.begin(); (i != a->indices.end()) && (j != b->indices.end());
       i++, j++)
  {
    if (*i != *j)
    {
      // fall back on search
      auto maybe_found = std::find(b->indices.begin(), b->indices.end(), *i);
      if (maybe_found == b->indices.end()) { return FormatIndicies(a, b); }
    }

    // compare features
    VW::features& fa = a->feature_space[*i];
    VW::features& fb = b->feature_space[*i];

    if (fa.size() != fb.size())
    {
      std::stringstream sstream;
      sstream << "Feature length differ " << fa.size() << " vs " << fb.size() << "this("
              << format_features(workspace, fa) << ") vs other(" << format_features(workspace, fb) << ')';

      return vw_net_native::stringstream_to_cstr(sstream);
    }

    auto diff = compare_features(workspace, fa, fb, *i);
    if (diff != nullptr) { return diff; }

    diff = compare_features(workspace, fb, fa, *i);
    if (diff != nullptr) { return diff; }
  }

  return nullptr;
}

API uint64_t GetExampleNumberOfFeatures(VW::example* example) { return example->num_features; }

API void EmptyExampleData(vw_net_native::workspace_context* workspace, VW::example* example)
{
  VW::empty_example(*workspace->vw, *example);
}

API void MakeIntoNewlineExample(vw_net_native::workspace_context* workspace, VW::example* example)
{
  const char empty = '\0';

  VW::parsers::text::read_line(*workspace->vw, example, &empty);
  VW::setup_example(*workspace->vw, example);
}

API void MakeLabelDefault(vw_net_native::workspace_context* workspace, VW::example* example)
{
  workspace->vw->parser_runtime.example_parser->lbl_parser.default_label(example->l);
}

API void UpdateExampleWeight(vw_net_native::workspace_context* workspace, VW::example* example)
{
  example->weight =
      workspace->vw->parser_runtime.example_parser->lbl_parser.get_weight(example->l, example->ex_reduction_features);
}

API vw_net_native::namespace_enumerator* CreateNamespaceEnumerator(
    vw_net_native::workspace_context* workspace, VW::example* example)
{
  auto* it = new vw_net_native::namespace_enumerator;
  it->v = &example->indices;

  NamespaceEnumeratorReset(it);

  return it;
}

API void DeleteNamespaceEnumerator(vw_net_native::namespace_enumerator* it) { delete it; }

API int NamespaceEnumeratorMoveNext(vw_net_native::namespace_enumerator* it)
{
  it->it++;
  return it->it < it->v->cend();
}

API void NamespaceEnumeratorReset(vw_net_native::namespace_enumerator* it) { it->it = it->v->cbegin() - 1; }

API VW::namespace_index NamespaceEnumeratorGetNamespace(vw_net_native::namespace_enumerator* it) { return *it->it; }

API vw_net_native::feature_enumerator* CreateFeatureEnumerator(
    vw_net_native::workspace_context* workspace, VW::example* example, VW::namespace_index ns)
{
  auto* it = new vw_net_native::feature_enumerator;
  it->feat = &example->feature_space[ns];
  it->ns = ns;

  FeatureEnumeratorReset(it);

  return it;
}

API void DeleteFeatureEnumerator(vw_net_native::feature_enumerator* it) { delete it; }

API int FeatureEnumeratorMoveNext(vw_net_native::feature_enumerator* it)
{
  it->it.operator++();  // Not sure why it does not like to compile without explicit reference to the operator.
  return it->it < it->feat->cend();
}

API void FeatureEnumeratorReset(vw_net_native::feature_enumerator* it) { it->it = it->feat->cbegin() - 1; }

API void FeatureEnumeratorGetFeature(vw_net_native::feature_enumerator* it, VW::feature* feature)
{
  feature->x = it->it.value();
  feature->weight_index = it->it.index();
}

API float FeatureEnumeratorGetFeatureValue(vw_net_native::feature_enumerator* it) { return it->it.value(); }

API VW::feature_index FeatureEnumeratorGetFeatureIndex(vw_net_native::feature_enumerator* it) { return it->it.index(); }

API VW::feature_index GetShiftedWeightIndex(
    vw_net_native::workspace_context* workspace, VW::example* example, VW::feature_index feature_index)
{
  VW::workspace* vw = workspace->vw;
  return ((feature_index + example->ft_offset) >> vw->weights.stride_shift()) & vw->runtime_state.parse_mask;
}

API float GetWeight(vw_net_native::workspace_context* workspace, VW::example* example, VW::feature_index feature_index)
{
  // TODO: Is this calculation right? Why are we not shifting this?
  uint64_t weightIndex = feature_index + example->ft_offset;
  return workspace->vw->weights[weightIndex];
}

API float GetAuditWeight(
    vw_net_native::workspace_context* workspace, VW::example* example, VW::feature_index feature_index)
{
  VW::workspace* vw = workspace->vw;

  float weight = GetWeight(workspace, example, feature_index);
  return GD::trunc_weight(weight, (float)vw->sd->gravity) + (float)vw->sd->contraction;
}
