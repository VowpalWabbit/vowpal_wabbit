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
		m_owner(owner), m_example(example->m_example), m_innerExample(example)
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

	System::String^ VowpalWabbitExample::Diff(VowpalWabbit^ vw, VowpalWabbitExample^ other, bool sameOrder)
	{
		auto otherSameType = dynamic_cast<VowpalWabbitExample^>(other);

		if (otherSameType == nullptr)
		{
			return gcnew System::String("Can't compare examples of different types.");
		}

		auto a = this->m_example;
		auto b = otherSameType->m_example;
		if (a->indices.size() != b->indices.size())
		{
			return System::String::Format("Indicies length differ: {0} vs {1}",
				a->indices.size(),
				b->indices.size());
		}

		for (auto i = a->indices.begin, j = b->indices.begin; i != a->indices.end; i++)
		{
			if (sameOrder)
			{
				if (*i != *j)
				{
					return gcnew System::String("Can't find index.");
				}
				j++;
			}
			else
			{
				// search index
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
				{
					System::String^ s = gcnew System::String("this: ");

					for (auto x = a->indices.begin; x != a->indices.end; x++)
					{
						System::String^ sc;

						if (*x == 0)
							sc = gcnew System::String("NULL");
						else
							sc = gcnew System::String((char*)&*x, 0, 1);

						s += System::String::Format("{0}:'{1}',", *x, sc);
					}

					s += "\n";

					for (auto x = b->indices.begin; x != b->indices.end; x++)
					{
						System::String^ sc;

						if (*x == 0)
							sc = gcnew System::String("NULL");
						else
							sc = gcnew System::String((char*)&*x, 0, 1);

						s += System::String::Format("{0}:'{1}',", *x, sc);
					}

					return System::String::Format("Can't find index: {0}", s);
				}
			}

			// compare features
			auto fa = a->atomics[*i];
			auto fb = b->atomics[*i];

			if (fa.size() != fb.size())
			{
				return gcnew System::String("Feature length differ");
			}

			vector<feature*> fa_missing;
			for (auto k = fa.begin, l = fb.begin; k != fa.end; k++)
			{
				auto masked_weight_index = k->weight_index & vw->m_vw->reg.weight_mask;

				if (sameOrder)
				{
					auto other_masked_weight_index = l->weight_index & vw->m_vw->reg.weight_mask;
					if (!(masked_weight_index == other_masked_weight_index && abs(k->x - l->x) < 1e-5))
					{
						return System::String::Format(
							"Feature differ: this(weight_index = {0}, x = {1}) vs other(weight_index = {2}, x = {3})",
							masked_weight_index, k->x,
							other_masked_weight_index, l->x);
					}

					l++;
				}
				else
				{
					bool found = false;
					for (l = fb.begin; l != fb.end; l++)
					{
						auto other_masked_weight_index = l->weight_index & vw->m_vw->reg.weight_mask;

						if (masked_weight_index == other_masked_weight_index)
						{
							//if (abs(k->x - l->x) > 1e-5)
							if (abs(k->x - l->x) / max(k->x, l->x) > 0.01)
							{
								return System::String::Format(
									"Feature differ: this(weight_index = {0}/{4}, x = {1}) vs other(weight_index = {2}/{5}, x = {3})",
									masked_weight_index, gcnew System::Single(k->x),
									other_masked_weight_index, gcnew System::Single(l->x),
									k->weight_index,
									l->weight_index);
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
						// return System::String::Format("Can't find feature: this.weight_index = {0}, x = {1}", k->weight_index, k->x);
					}
				}
			}
			vector<feature*> fb_missing;
			for (auto k = fb.begin, l = fa.begin; k != fb.end; k++)
			{
				auto masked_weight_index = k->weight_index & vw->m_vw->reg.weight_mask;

				if (sameOrder)
				{
					auto other_masked_weight_index = l->weight_index & vw->m_vw->reg.weight_mask;
					if (!(masked_weight_index == other_masked_weight_index && abs(k->x - l->x) < 1e-5))
					{
						return System::String::Format(
							"Feature differ: other(weight_index = {0}, x = {1}) vs this(weight_index = {2}, x = {3})",
							masked_weight_index, k->x,
							other_masked_weight_index, l->x);
					}

					l++;
				}
				else
				{
					bool found = false;
					for (l = fa.begin; l != fa.end; l++)
					{
						auto other_masked_weight_index = l->weight_index & vw->m_vw->reg.weight_mask;

						if (masked_weight_index == other_masked_weight_index)
						{
							if (abs(k->x - l->x) / max(k->x, l->x) > 0.01)
								//if (abs(k->x - l->x) > 1e-5)
							{
								return System::String::Format(
									"Feature differ: this(weight_index = {0}/{4}, x = {1}) vs other(weight_index = {2}/{5}, x = {3})",
									masked_weight_index, gcnew System::Single(k->x),
									other_masked_weight_index, gcnew System::Single(l->x),
									k->weight_index,
									l->weight_index);
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
						fb_missing.push_back(&*k);
						// return System::String::Format("Can't find feature: this.weight_index = {0}, x = {1}", k->weight_index, k->x);
					}
				}
			}

			if (!fa_missing.empty() || !fb_missing.empty())
			{
				System::String^ diff = gcnew System::String("missing: ");
				for each (feature* k in fa_missing)
				{
					diff += System::String::Format("this.weight_index = {0}, x = {1}, ",
						k->weight_index & vw->m_vw->reg.weight_mask,
						k->x);
				}

				diff += "\n";

				for each (feature* k in fb_missing)
				{
					diff += System::String::Format("other.weight_index = {0}, x = {1}, ",
						k->weight_index & vw->m_vw->reg.weight_mask,
						k->x);
				}

				return diff;
			}
		}

		// TODO: introduce parameter to check different kind of labels
		//auto s1 = a->l.simple;
		//auto s2 = b->l.simple;

		//if (!(s1.initial == s2.initial &&
		//	s1.label == s2.label &&
		//	s1.weight == s2.weight))
		//{
		//	return gcnew System::String("Label differ");
		//}

		auto s1 = a->l.cb;
		auto s2 = b->l.cb;

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