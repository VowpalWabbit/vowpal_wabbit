// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vowpalwabbit.h"
#include "vw_example.h"
#include "vw_prediction.h"
#include "simple_label_parser.h"
#include "gd.h"
#include <algorithm>
#include "shared_data.h"

namespace VW
{
using namespace Labels;

VowpalWabbitExample::VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, example* example) :
  m_owner(owner), m_example(example), m_innerExample(nullptr)
{
}

VowpalWabbitExample::VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, VowpalWabbitExample^ example) :
  m_owner(owner), m_example(example->m_example), m_innerExample(example), m_string(example->m_string)
{
}

VowpalWabbitExample::!VowpalWabbitExample()
{ if (m_owner != nullptr)
    m_owner->ReturnExampleToPool(this);
}

VowpalWabbitExample::~VowpalWabbitExample()
{ this->!VowpalWabbitExample();
}

VowpalWabbitExample^ VowpalWabbitExample::InnerExample::get()
{ return m_innerExample;
}

IVowpalWabbitExamplePool^ VowpalWabbitExample::Owner::get()
{ return m_owner;
}

size_t VowpalWabbitExample::NumberOfFeatures::get()
{ return m_example->num_features;
}

generic<typename T> T VowpalWabbitExample::GetPrediction(VowpalWabbit^ vw, IVowpalWabbitPredictionFactory<T>^ factory)
{
#ifdef _DEBUG
  if (vw == nullptr)
    throw gcnew ArgumentNullException("vw");
#endif

  return factory->Create(vw->m_vw, m_example);
}

String^ VowpalWabbitExample::VowpalWabbitString::get()
{ return m_string;
}

void VowpalWabbitExample::VowpalWabbitString::set(String^ value)
{ m_string = value;
}

bool VowpalWabbitExample::IsNewLine::get()
{ return example_is_newline(*m_example) != 0;
}

ILabel^ VowpalWabbitExample::Label::get()
{ ILabel^ label;
  auto lp = m_owner->Native->m_vw->example_parser->lbl_parser;
  if (!memcmp(&lp, &simple_label_parser, sizeof(lp)))
    label = gcnew SimpleLabel();
  else if (!memcmp(&lp, &CB::cb_label, sizeof(lp)))
    label = gcnew ContextualBanditLabel();
  else if (!memcmp(&lp, &CB_EVAL::cb_eval, sizeof(lp)))
    label = gcnew SimpleLabel();
  else if (!memcmp(&lp, &COST_SENSITIVE::cs_label, sizeof(lp)))
    label = gcnew SimpleLabel();
  else
    return nullptr;

  // TODO:
  //else if (!memcmp(&lp, &MULTICLASS::multilabel, sizeof(lp)))
  //  label = gcnew MulticlassLabel;
  //else if (!memcmp(&lp, &MC::multilabel, sizeof(lp)))

  label->ReadFromExample(this->m_example);

  return label;
}

void VowpalWabbitExample::Label::set(ILabel^ label)
{
	if (label == nullptr)
		return;

	label->UpdateExample(m_owner->Native->m_vw, m_example);

	// we need to update the example weight as setup_example() can be called prior to this call.
	m_example->weight = m_owner->Native->m_vw->example_parser->lbl_parser.get_weight(&m_example->l, m_example->_reduction_features);
}

void VowpalWabbitExample::MakeEmpty(VowpalWabbit^ vw)
{ char empty = '\0';
  VW::read_line(*vw->m_vw, m_example, &empty);

  VW::setup_example(*vw->m_vw, m_example);
}

void FormatIndices(example* a, System::Text::StringBuilder^ sb)
{ for (auto ns : a->indices)
  { if (ns == 0)
      sb->Append("NULL:0,");
    else
      sb->AppendFormat("'{0}':{1},", gcnew System::Char(ns), (int)ns);
  }
}

System::String^ FormatIndices(example* a, example *b)
{ auto sb = gcnew System::Text::StringBuilder();

  sb->AppendFormat("Namespace indicies differ: {0} vs {1}. this.indices: [",
                   a->indices.size(),
                   b->indices.size());

  FormatIndices(a, sb);

  sb->Append("] other.indices: [");

  FormatIndices(b, sb);

  sb->Append("]");

  return sb->ToString();
}

System::String^ FormatFeature(vw* vw, feature_value& f1, feature_index& i1)
{ uint64_t masked_weight_index1 = i1 & vw->weights.mask();

  return System::String::Format(
           "weight_index = {0}/{1}, x = {2}",
           masked_weight_index1,
           i1,
           gcnew System::Single(f1));
}

System::String^ FormatFeature(vw* vw, feature_value& f1, feature_index& i1, feature_value& f2, feature_index& i2)
{ return System::String::Format(
           "Feature differ: this({0}) vs other({1})",
           FormatFeature(vw, f1, i1),
           FormatFeature(vw, f2, i2));
}

bool FloatEqual(float a, float b)
{ if ((abs(a) < 1e-20 && abs(b) < 1e-20) ||
      (isinf(a) && isinf(b)))
  { return true;
  }

  return abs(a - b) / std::max(a, b) < 1e-6;
}

System::String ^
    FormatFeatures(vw* vw, VW::namespaced_feature_store::index_flat_iterator begin,
        VW::namespaced_feature_store::index_flat_iterator end) {
      auto sb = gcnew System::Text::StringBuilder();
      for (; begin != end; ++begin) { sb->Append(FormatFeature(vw, (*begin).value(), (*begin).index()))->Append(" "); }
  return sb->ToString();
}

System::String
    ^
    CompareFeatures(vw* vw, VW::namespaced_feature_store::index_flat_iterator fa_begin,
        VW::namespaced_feature_store::index_flat_iterator fa_end,
        VW::namespaced_feature_store::index_flat_iterator fb_begin,
        VW::namespaced_feature_store::index_flat_iterator fb_end, unsigned char ns) {

  std::vector<std::pair<uint64_t, float>> fa_missing;
      for (; fa_begin != fa_end; ++fa_begin)
  {
        auto original_fb_begin = fb_begin;
        auto masked_weight_index = (*fa_begin).index() & vw->weights.mask();
    auto other_masked_weight_index = (*fb_begin).index() & vw->weights.mask();

    /*System::Diagnostics::Debug::WriteLine(System::String::Format("{0} -> {1} vs {2} -> {3}",
      fa.indicies[ia], masked_weight_index,
      fb.indicies[ib], other_masked_weight_index
      ));*/

    if (masked_weight_index == other_masked_weight_index && FloatEqual((*fa_begin).value(), (*fb_begin).value()))
      ++fb_begin;
    else
    { // fallback to search
      bool found = false;
      for (auto inner_fb_begin = original_fb_begin; inner_fb_begin != fb_end; ++inner_fb_begin)
      {
        auto other_masked_weight_index = (*inner_fb_begin).index() & vw->weights.mask();
        if (masked_weight_index == other_masked_weight_index)
        {
          if (!FloatEqual((*fa_begin).value(), (*inner_fb_begin).value()))
          {
            return FormatFeature(
                vw, (*fa_begin).value(), (*fa_begin).index(), (*inner_fb_begin).value(), (*inner_fb_begin).index());
          }
          else
          { found = true;
            break;
          }
        }
      }

      if (!found) { fa_missing.emplace_back((*fa_begin).index(), (*fa_begin).value());
      }
    }
  }

  if (!fa_missing.empty())
  { auto diff = gcnew System::Text::StringBuilder();
    diff->AppendFormat("missing features in ns '{0}'/'{1}': ", ns, gcnew Char(ns));
    for (auto ia : fa_missing)
    { diff->AppendFormat("this.weight_index = {0}, x = {1}, ",
                         ia.first & vw->weights.mask(),
                         ia.second);
    }

    return diff->ToString();
  }

  return nullptr;
}

System::String^ VowpalWabbitExample::Diff(VowpalWabbit^ vw, VowpalWabbitExample^ other, IVowpalWabbitLabelComparator^ labelComparator)
{ auto a = this->m_example;
  auto b = other->m_example;

  if (a->indices.size() != b->indices.size())
  { return FormatIndices(a, b);
  }

  for (auto i = a->indices.begin(), j = b->indices.begin(); i != a->indices.end(); i++)
  { if (*i == *j)
      j++;
    else
    { // fall back on search
      auto j_old = j;

      j = b->indices.begin();
      bool found = false;
      for (; j != b->indices.end(); j++)
      { if (*i == *j)
        { found = true;
          break;
        }
      }

      if (!found)
        return FormatIndices(a, b);

      j = j_old + 1;
    }

    // compare features
    //features& fa = a->feature_space[*i];
    //features& fb = b->feature_space[*i];
    auto fa_i_begin = a->feature_space.index_flat_begin(*i);
    auto fa_i_end = a->feature_space.index_flat_end(*i);
    auto fa_size = fa_i_end - fa_i_begin;
    auto fb_i_begin = b->feature_space.index_flat_begin(*i);
    auto fb_i_end = b->feature_space.index_flat_end(*i);
    auto fb_size = fb_i_end - fb_i_begin;

    if (fa_size != fb_size)
      return System::String::Format("Feature length differ {0} vs {1}. this({2}) vs other({3})", fa_size, fb_size,
          FormatFeatures(vw->m_vw, fa_i_begin, fa_i_end), FormatFeatures(vw->m_vw, fb_i_begin, fb_i_end));

    auto diff = CompareFeatures(vw->m_vw, fa_i_begin, fa_i_end, fb_i_begin, fb_i_end, *i);
    if (diff != nullptr)
      return diff;

    diff = CompareFeatures(vw->m_vw, fb_i_begin, fb_i_end, fa_i_begin, fa_i_end, *i);
    if (diff != nullptr)
      return diff;
  }

  if (labelComparator != nullptr)
  { // Compare the label
    auto diff = labelComparator->Diff(this, other);
    if (diff != nullptr)
      return diff;
  }

  return nullptr;
}

String^ VowpalWabbitSimpleLabelComparator::Diff(VowpalWabbitExample^ ex1, VowpalWabbitExample^ ex2)
{ auto s1 = ex1->m_example->l.simple;
  auto s2 = ex2->m_example->l.simple;
  auto ex1_initial = ex1->m_example->_reduction_features.template get<simple_label_reduction_features>().initial;
  auto ex2_initial = ex2->m_example->_reduction_features.template get<simple_label_reduction_features>().initial;

  if (!(FloatEqual(ex1_initial, ex2_initial) &&
        FloatEqual(s1.label, s2.label) &&
        FloatEqual(ex1->m_example->weight, ex2->m_example->weight)))
  { return System::String::Format("Label differ. label {0} vs {1}. initial {2} vs {3}. weight {4} vs {5}",
                                  s1.label, s2.label,
                                  ex1_initial, ex2_initial,
                                  ex1->m_example->weight, ex2->m_example->weight);
  }

  return nullptr;
}

String^ VowpalWabbitContextualBanditLabelComparator::Diff(VowpalWabbitExample^ ex1, VowpalWabbitExample^ ex2)
{ auto s1 = ex1->m_example->l.cb;
  auto s2 = ex2->m_example->l.cb;

  if (s1.costs.size() != s2.costs.size())
  { return System::String::Format("Cost size differ: {0} vs {1}", s1.costs.size(), s2.costs.size());
  }

  for (size_t i = 0; i < s1.costs.size(); i++)
  { auto c1 = s1.costs[i];
    auto c2 = s2.costs[i];
    if (c1.action != c2.action)
    { return System::String::Format("Action differ: {0} vs {1}", c1.action, c2.action);
    }

    if (c1.cost != c2.cost)
    { return System::String::Format("Cost differ: {0} vs {1}", c1.cost, c2.cost);
    }

    if (abs(c1.probability - c2.probability) / std::max(c1.probability, c2.probability) > 0.01)
    { return System::String::Format("Probability differ: {0} vs {1}", c1.probability, c2.probability);
    }
  }

  return nullptr;
}

System::Collections::IEnumerator^ VowpalWabbitExample::EnumerableGetEnumerator::get()
{ return GetEnumerator();
}

IEnumerator<VowpalWabbitNamespace^>^ VowpalWabbitExample::GetEnumerator()
{ return gcnew NamespaceEnumerator(this);
}

VowpalWabbitExample::NamespaceEnumerator::NamespaceEnumerator(VowpalWabbitExample^ example)
  : m_example(example)
{
  m_current = nullptr;
  m_end = new Holder<std::vector<namespace_index>::const_iterator>{m_example->m_example->indices.end()};
}

VowpalWabbitExample::NamespaceEnumerator::~NamespaceEnumerator()
{
  delete m_current;
  delete m_end;
}

bool VowpalWabbitExample::NamespaceEnumerator::MoveNext()
{
  if (m_current == nullptr)
  {
    m_current = new Holder<std::vector<namespace_index>::const_iterator>{m_example->m_example->indices.begin()};
  }
  else
  {
    m_current->value.operator++();
  }
  return m_current->value != m_end->value;
}

void VowpalWabbitExample::NamespaceEnumerator::Reset()
{
  delete m_current;
  m_current = nullptr;
}

VowpalWabbitNamespace^ VowpalWabbitExample::NamespaceEnumerator::Current::get()
{
  return gcnew VowpalWabbitNamespace(m_example, *m_current->value);
}

System::Object^ VowpalWabbitExample::NamespaceEnumerator::IEnumeratorCurrent::get()
{ return Current;
}

VowpalWabbitFeature::VowpalWabbitFeature(VowpalWabbitExample^ example, feature_value x, uint64_t weight_index)
  : m_example(example), m_vw(m_example->Owner->Native), m_x(x), m_weight_index(weight_index)
{ }

VowpalWabbitFeature::VowpalWabbitFeature(VowpalWabbit^ vw, feature_value x, uint64_t weight_index)
  : m_vw(vw), m_x(x), m_weight_index(weight_index)
{ }

float VowpalWabbitFeature::X::get()
{ return m_x;
}

uint64_t VowpalWabbitFeature::FeatureIndex::get()
{ return m_weight_index;
}

uint64_t VowpalWabbitFeature::WeightIndex::get()
{ if (m_example == nullptr)
    throw gcnew InvalidOperationException("VowpalWabbitFeature must be initialized with example");

  vw* vw = m_example->Owner->Native->m_vw;
  return ((m_weight_index + m_example->m_example->ft_offset) >> vw->weights.stride_shift()) & vw->parse_mask;
}

float VowpalWabbitFeature::Weight::get()
{ if (m_example == nullptr)
    throw gcnew InvalidOperationException("VowpalWabbitFeature must be initialized with example");

  vw* vw = m_example->Owner->Native->m_vw;

  uint64_t weightIndex = m_weight_index + m_example->m_example->ft_offset;
  return vw->weights[weightIndex];
}


float VowpalWabbitFeature::AuditWeight::get()
{ vw* vw = m_vw->m_vw;

  return GD::trunc_weight(Weight, (float)vw->sd->gravity) * (float)vw->sd->contraction;
}

bool VowpalWabbitFeature::Equals(Object^ o)
{ VowpalWabbitFeature^ other = dynamic_cast<VowpalWabbitFeature^>(o);

  return other != nullptr &&
         other->m_x == m_x &&
         other->m_weight_index == m_weight_index;
}

int VowpalWabbitFeature::GetHashCode()
{ return (int)(m_x + m_weight_index);
}


VowpalWabbitNamespace::VowpalWabbitNamespace(VowpalWabbitExample^ example, namespace_index ns)
  : m_example(example), m_ns(ns)
{ }

VowpalWabbitNamespace::~VowpalWabbitNamespace()
{ }

namespace_index VowpalWabbitNamespace::Index::get()
{ return m_ns;
}

System::Collections::IEnumerator^ VowpalWabbitNamespace::EnumerableGetEnumerator::get()
{ return GetEnumerator();
}

IEnumerator<VowpalWabbitFeature^>^ VowpalWabbitNamespace::GetEnumerator()
{ return gcnew FeatureEnumerator(m_example, m_ns);
}

VowpalWabbitNamespace::FeatureEnumerator::FeatureEnumerator(VowpalWabbitExample^ example, namespace_index ns_index)
    : m_example(example), m_ns(ns_index)
{
  m_current = nullptr;
  m_end = new Holder<VW::namespaced_feature_store::index_flat_iterator>{
      example->m_example->feature_space.index_flat_end(ns_index)};
}

VowpalWabbitNamespace::FeatureEnumerator::~FeatureEnumerator()
{ delete m_end;
  delete m_current;
}

void VowpalWabbitNamespace::FeatureEnumerator::Reset()
{
  delete m_current;
  m_current = nullptr;
}

bool VowpalWabbitNamespace::FeatureEnumerator::MoveNext()
{
  if (m_current == nullptr)
  {
    m_current = new Holder<VW::namespaced_feature_store::index_flat_iterator>{
        m_example->m_example->feature_space.index_flat_begin(m_ns)};
  }
  else
  {
    ++m_current->value;
  }

  return m_current->value != m_end->value;
}

System::Object^ VowpalWabbitNamespace::FeatureEnumerator::IEnumeratorCurrent::get()
{ return Current;
}

VowpalWabbitFeature^ VowpalWabbitNamespace::FeatureEnumerator::Current::get()
{
  return gcnew VowpalWabbitFeature(
      m_example, m_current->value.operator*().value(), m_current->value.operator*().index());
}
}
