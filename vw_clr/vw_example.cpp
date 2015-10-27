/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#include "vowpalwabbit.h"
#include "vw_example.h"
#include "vw_prediction.h"

namespace VW
{
	VowpalWabbitExample::VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, example* example) :
		m_owner(owner), m_example(example), m_innerExample(nullptr)
	{
	}

	VowpalWabbitExample::VowpalWabbitExample(IVowpalWabbitExamplePool^ owner, VowpalWabbitExample^ example) :
		m_owner(owner), m_example(example->m_example), m_innerExample(example), m_string(example->m_string)
	{
	}

	VowpalWabbitExample::!VowpalWabbitExample()
	{
		if (m_owner != nullptr)
			m_owner->ReturnExampleToPool(this);
	}

	VowpalWabbitExample::~VowpalWabbitExample()
	{
		this->!VowpalWabbitExample();
	}

	VowpalWabbitExample^ VowpalWabbitExample::InnerExample::get()
	{
		return m_innerExample;
	}

	IVowpalWabbitExamplePool^ VowpalWabbitExample::Owner::get()
	{
		return m_owner;
	}

	generic<typename T>
	T VowpalWabbitExample::GetPrediction(VowpalWabbit^ vw, IVowpalWabbitPredictionFactory<T>^ factory)
	{
#ifdef _DEBUG
		if (vw == nullptr)
			throw gcnew ArgumentNullException("vw");
#endif

		return factory->Create(vw->m_vw, m_example);
	}
	
	String^ VowpalWabbitExample::VowpalWabbitString::get()
	{
		return m_string;
	}

	void VowpalWabbitExample::VowpalWabbitString::set(String^ value)
	{
		m_string = value;
	}

  bool VowpalWabbitExample::IsNewLine::get()
  {
    return example_is_newline(*m_example);
  }

  void FormatIndices(example* a, System::Text::StringBuilder^ sb)
  {
    for (auto i = a->indices.begin; i != a->indices.end; i++)
    {
      if (*i == 0)
        sb->Append("NULL:0,");
      else
        sb->AppendFormat("'{0}':{1},", gcnew System::Char(*i), (int)*i);
    }
  }

  System::String^ FormatIndices(example* a, example *b)
  {
    auto sb = gcnew System::Text::StringBuilder();

    sb->AppendFormat("Namespace indicies differ: {0} vs {1}. this.indices: [",
      a->indices.size(),
      b->indices.size());

    FormatIndices(a, sb);

    sb->Append("] other.indices: [");

    FormatIndices(b, sb);

    sb->Append("]");

    return sb->ToString();
  }

  System::String^ FormatFeature(vw* vw, feature* f1)
  {
    auto masked_weight_index1 = f1->weight_index & vw->reg.weight_mask;

    return System::String::Format(
      "weight_index = {0}/{1}, x = {2}",
      masked_weight_index1,
      f1->weight_index,
      gcnew System::Single(f1->x));
  }

  System::String^ FormatFeature(vw* vw, feature* f1, feature* f2)
  {
    return System::String::Format(
      "Feature differ: this({0}) vs other({1})",
      FormatFeature(vw, f1),
      FormatFeature(vw, f2));
  }

  bool FloatEqual(float a, float b)
  {
    if ((abs(a) < 1e-20 && abs(b) < 1e-20) ||
      (isinf(a) && isinf(b)))
    {
      return true;
    }

    return abs(a - b) / max(a, b) < 1e-6;
  }

  System::String^ FormatFeatures(vw* vw, v_array<feature>& arr)
  {
    auto sb = gcnew System::Text::StringBuilder();
    for (auto f = arr.begin; f != arr.end; f++)
      sb->Append(FormatFeature(vw, f))->Append(" ");

    return sb->ToString();
  }

  System::String^ CompareFeatures(vw* vw, v_array<feature>& fa, v_array<feature>& fb)
  {
    vector<feature*> fa_missing;
    for (auto k = fa.begin, l = fb.begin; k != fa.end; k++)
    {
      auto masked_weight_index = k->weight_index & vw->reg.weight_mask;

      auto other_masked_weight_index = l->weight_index & vw->reg.weight_mask;
      if (masked_weight_index == other_masked_weight_index && FloatEqual(k->x, l->x))
        l++;
      else
      {
        // fallback to search
        auto l_old = l;
        bool found = false;
        for (l = fb.begin; l != fb.end; l++)
        {
          auto other_masked_weight_index = l->weight_index & vw->reg.weight_mask;

          if (masked_weight_index == other_masked_weight_index)
          {
            if (!FloatEqual(k->x, l->x))
            {
              return FormatFeature(vw, k, l);
            }
            else
            {
              found = true;
              break;
            }
          }
        }

        if (!found)
        {
          fa_missing.push_back(&*k);
        }

        l = l_old + 1;
      }
    }

    if (!fa_missing.empty())
    {
      auto diff = gcnew System::Text::StringBuilder("missing: ");
      for each (feature* k in fa_missing)
      {
        diff->AppendFormat("this.weight_index = {0}, x = {1}, ",
          k->weight_index & vw->reg.weight_mask,
          k->x);
      }

      return diff->ToString();
    }

    return nullptr;
  }

  System::String^ VowpalWabbitExample::Diff(VowpalWabbit^ vw, VowpalWabbitExample^ other, IVowpalWabbitLabelComparator^ labelComparator)
	{
		auto a = this->m_example;
		auto b = other->m_example;

    if (a->indices.size() != b->indices.size())
    {
      return FormatIndices(a, b);
    }

    for (auto i = a->indices.begin, j = b->indices.begin; i != a->indices.end; i++)
    {
      if (*i == *j)
        j++;
      else
      {
        // fall back on search
        auto j_old = j;

        j = b->indices.begin;
        bool found = false;
        for (; j != b->indices.end; j++)
        {
          if (*i == *j)
          {
            found = true;
            break;
          }
        }

        if (!found)
          return FormatIndices(a, b);

        j = j_old + 1;
      }

			// compare features
			auto fa = a->atomics[*i];
			auto fb = b->atomics[*i];

			if (fa.size() != fb.size())
        return System::String::Format("Feature length differ {0} vs {1}. this({2}) vs other({3})",
          fa.size(), fb.size(), FormatFeatures(vw->m_vw, fa), FormatFeatures(vw->m_vw, fb));

      auto diff = CompareFeatures(vw->m_vw, fa, fb);
      if (diff != nullptr)
        return diff;

      diff = CompareFeatures(vw->m_vw, fb, fa);
      if (diff != nullptr)
        return diff;
		}

    if (labelComparator != nullptr)
    {
      // Compare the label
      auto diff = labelComparator->Diff(this, other);
      if (diff != nullptr)
        return diff;
    }

		return nullptr;
	}

  String^ VowpalWabbitSimpleLabelComparator::Diff(VowpalWabbitExample^ ex1, VowpalWabbitExample^ ex2)
  {
    auto s1 = ex1->m_example->l.simple;
    auto s2 = ex2->m_example->l.simple;

    if (!(FloatEqual(s1.initial, s2.initial) &&
      FloatEqual(s1.label, s2.label) &&
      FloatEqual(s1.weight, s2.weight)))
    {
      return System::String::Format("Label differ. label {0} vs {1}. initial {2} vs {3}. weight {4} vs {5}",
        s1.label, s2.label,
        s1.initial, s2.initial,
        s1.weight, s2.weight);
    }

    return nullptr;
  }

  String^ VowpalWabbitContextualBanditLabelComparator::Diff(VowpalWabbitExample^ ex1, VowpalWabbitExample^ ex2)
  {
    auto s1 = ex1->m_example->l.cb;
    auto s2 = ex2->m_example->l.cb;

    if (s1.costs.size() != s2.costs.size())
    {
      return System::String::Format("Cost size differ: {0} vs {1}", s1.costs.size(), s2.costs.size());
    }

    for (size_t i = 0; i < s1.costs.size(); i++)
    {
      auto c1 = s1.costs[i];
      auto c2 = s2.costs[i];
      if (c1.action != c2.action)
      {
        return System::String::Format("Action differ: {0} vs {1}", c1.action, c2.action);
      }

      if (c1.cost != c2.cost)
      {
        return System::String::Format("Cost differ: {0} vs {1}", c1.cost, c2.cost);
      }

      if (abs(c1.probability - c2.probability) / max(c1.probability, c2.probability) > 0.01)
      {
        return System::String::Format("Probability differ: {0} vs {1}", c1.probability, c2.probability);
      }
    }

    return nullptr;
  }
}
