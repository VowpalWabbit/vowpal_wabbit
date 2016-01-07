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

    generic<typename T> T VowpalWabbitExample::GetPrediction(VowpalWabbit^ vw, IVowpalWabbitPredictionFactory<T>^ factory)
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
        return example_is_newline(*m_example) != 0;
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

    System::String^ FormatFeature(vw* vw, feature_value& f1, feature_index& i1)
    {
        uint64_t masked_weight_index1 = i1 & vw->reg.weight_mask;

        return System::String::Format(
            "weight_index = {0}/{1}, x = {2}",
            masked_weight_index1,
            i1,
            gcnew System::Single(f1));
    }

    System::String^ FormatFeature(vw* vw, feature_value& f1, feature_index& i1, feature_value& f2, feature_index& i2)
    {
        return System::String::Format(
            "Feature differ: this({0}) vs other({1})",
            FormatFeature(vw, f1, i1),
            FormatFeature(vw, f2, i2));
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

    System::String^ FormatFeatures(vw* vw, features& arr)
    {
        auto sb = gcnew System::Text::StringBuilder();
        for (size_t i = 0; i < arr.values.size(); i++)
        {
            sb->Append(FormatFeature(vw, arr.values[i], arr.indicies[i]))->Append(" ");
        }

        return sb->ToString();
    }

    System::String^ CompareFeatures(vw* vw, features& fa, features& fb)
    {
        vector<size_t> fa_missing;
        for (size_t ia = 0, ib = 0; ia < fa.values.size(); ia++)
        {
            uint64_t masked_weight_index = fa.indicies[ia] & vw->reg.weight_mask;

            auto other_masked_weight_index = fb.indicies[ib] & vw->reg.weight_mask;
            if (masked_weight_index == other_masked_weight_index && FloatEqual(fa.values[ia], fb.values[ib]))
                ib++;
            else
            {
                // fallback to search
                size_t ib_old = ib;
                bool found = false;
                for (ib = 0; ib < fb.values.size(); ib++)
                {
                    uint64_t other_masked_weight_index = fb.indicies[ib] & vw->reg.weight_mask;

                    if (masked_weight_index == other_masked_weight_index)
                    {
                        if (!FloatEqual(fa.values[ia], fb.values[ib]))
                        {
                            return FormatFeature(vw, fa.values[ia], fa.indicies[ia], fb.values[ib], fb.indicies[ib]);
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
                    fa_missing.push_back(ia);
                }

                ib = ib_old + 1;
            }
        }

        if (!fa_missing.empty())
        {
            auto diff = gcnew System::Text::StringBuilder("missing: ");
            for each (size_t ia in fa_missing)
            {
                diff->AppendFormat("this.weight_index = {0}, x = {1}, ",
                    fa.indicies[ia] & vw->reg.weight_mask,
                    fa.values[ia]);
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
            features& fa = a->feature_space[*i];
            features& fb = b->feature_space[*i];

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
