#include "vw_clr.h"

namespace Microsoft
{
	namespace Research
	{
		namespace  MachineLearning
		{
			VowpalWabbitExample::VowpalWabbitExample(vw* vw, example* example) : 
				m_vw(vw), m_example(example), m_isDisposed(false)
			{
			}

			VowpalWabbitExample::!VowpalWabbitExample()
			{
				if (m_isDisposed)
				{
					return;
				}

				if (m_example)
				{
					// make sure we don't free an example that belongs to the ring
					if (!VW::is_ring_example(*m_vw, m_example))
					{
						if (m_vw->multilabel_prediction)
						{
							VW::dealloc_example(m_vw->p->lp.delete_label, *m_example, MULTILABEL::multilabel.delete_label);
						}
						else
						{
							VW::dealloc_example(m_vw->p->lp.delete_label, *m_example);
						}

						::free_it(m_example);
					}

					m_example = nullptr;
				}

				m_isDisposed = true;
			}

			VowpalWabbitExample::~VowpalWabbitExample()
			{
				this->!VowpalWabbitExample();
			}

			void VowpalWabbitExample::Finish()
			{
				m_vw->l->finish_example(*m_vw, *m_example);
			}

			float VowpalWabbitExample::Learn()
			{
				m_vw->learn(m_example);

				return VW::get_prediction(m_example);
			}

			float VowpalWabbitExample::Predict()
			{
				m_vw->l->predict(*m_example);

				//BUG: The below method may return garbage as it assumes a certain structure for ex->ld
				//which may not be the actual one used (e.g., for cost-sensitive multi-class learning)
				return VW::get_prediction(m_example);
			}

			float VowpalWabbitExample::CostSensitivePrediction::get()
			{
				return VW::get_cost_sensitive_prediction(m_example);
			}

			cli::array<int>^ VowpalWabbitExample::MultilabelPredictions::get()
			{
				size_t length;
				uint32_t* labels = VW::get_multilabel_predictions(m_example, length);

				if (length > Int32::MaxValue)
				{
					throw gcnew ArgumentOutOfRangeException("Multi-label predictions too large");
				}

				auto result = gcnew cli::array<int>((int)length);
				Marshal::Copy(IntPtr(labels), result, 0, (int)length);

				return result;
			}

			cli::array<float>^ VowpalWabbitExample::TopicPredictions::get()
			{
				auto result = gcnew cli::array<float>(m_vw->lda);
				Marshal::Copy(IntPtr(m_example->topic_predictions.begin), result, 0, m_vw->lda);
				return result;
			}

			System::String^ VowpalWabbitExample::Diff(IVowpalWabbitExample^ other, bool sameOrder)
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
						for (; j != b->indices.end; j++)
						{
							if (*i == *j)
							{
								break;
							}
						}

						if (j == b->indices.end)
						{
							return gcnew System::String("Can't find index.");
						}
					}

					// compare features
					auto fa = a->atomics[*i];
					auto fb = b->atomics[*i];

					if (fa.size() != fb.size())
					{
						return gcnew System::String("Feature length differ");
					}

					for (auto k = fa.begin, l = fb.begin; k != fa.end; k++)
					{
						auto masked_weight_index = k->weight_index & m_vw->reg.weight_mask;

						if (sameOrder)
						{
							auto other_masked_weight_index = l->weight_index & otherSameType->m_vw->reg.weight_mask;
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
							for (l = fb.begin; l != fb.end; l++)
							{
								auto other_masked_weight_index = l->weight_index & otherSameType->m_vw->reg.weight_mask;

								if (masked_weight_index == other_masked_weight_index)
								{
									if (k->x != l->x)
									{
										return System::String::Format(
											"Feature differ: this(weight_index = {0}, x = {1}) vs other(weight_index = {2}, x = {3})",
											masked_weight_index, k->x,
											other_masked_weight_index, l->x);
									}
								}
							}

							if (l == fb.end)
							{
								return System::String::Format("Can't find feature: weight_index = {0}, x = {1}", k->weight_index, k->x);
							}
						}
					}
				}

				// TODO: introduce parameter to check different kind of labels
				auto s1 = a->l.simple;
				auto s2 = b->l.simple;

				if (!(s1.initial == s2.initial &&
					s1.label == s2.label &&
					s1.weight == s2.weight))
				{
					return gcnew System::String("Label differ");
				}

				return nullptr;
			}
		}
	}
}