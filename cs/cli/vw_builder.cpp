// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw_builder.h"
#include "vw/core/parser.h"

namespace VW
{
VowpalWabbitExampleBuilder::VowpalWabbitExampleBuilder(IVowpalWabbitExamplePool^ vw) :
  m_vw(vw), m_example(nullptr)
{ if (vw == nullptr)
    throw gcnew ArgumentNullException("vw");

  m_example = vw->GetOrCreateNativeExample();
}

VowpalWabbitExampleBuilder::~VowpalWabbitExampleBuilder()
{ this->!VowpalWabbitExampleBuilder();
}

VowpalWabbitExampleBuilder::!VowpalWabbitExampleBuilder()
{ if (m_example != nullptr)
  { // in case CreateExample is not getting called
    delete m_example;

    m_example = nullptr;
  }
}

VowpalWabbitExample^ VowpalWabbitExampleBuilder::CreateExample()
{ if (m_example == nullptr)
    return nullptr;

  try
  { // finalize example
    VW::setup_example(*m_vw->Native->m_vw, m_example->m_example);
  }
  CATCHRETHROW

  // hand memory management off to VowpalWabbitExample
  auto ret = m_example;
  m_example = nullptr;

  return ret;
}

void VowpalWabbitExampleBuilder::ApplyLabel(ILabel^ label)
{ if (label == nullptr)
    return;

  label->UpdateExample(m_vw->Native->m_vw, m_example->m_example);
}

VowpalWabbitNamespaceBuilder^ VowpalWabbitExampleBuilder::AddNamespace(Char featureGroup)
{ return AddNamespace((Byte)featureGroup);
}

VowpalWabbitNamespaceBuilder^ VowpalWabbitExampleBuilder::AddNamespace(Byte featureGroup)
{ uint32_t index = featureGroup;
  example* ex = m_example->m_example;

  return gcnew VowpalWabbitNamespaceBuilder(ex->feature_space.data() + index, featureGroup, m_example->m_example);
}

VowpalWabbitNamespaceBuilder::VowpalWabbitNamespaceBuilder(features* features,
    unsigned char index, example* example)
  : m_features(features), m_index(index), m_example(example)
{ // Register the namespace up front. Iteration over an example is driven entirely by example->indices,
  // so features written into a group whose index is not yet registered are invisible to
  // VW::setup_example (leaving their weight indices unscaled by the stride multiplier) and to
  // VW::empty_example (leaking them into the next user of a pooled example). The finalizer drops the
  // index again if nothing was added.
  // Pass the native parameter rather than m_index: push_back takes a const reference, and a field of a
  // managed class cannot bind to one.
  for (unsigned char ns : m_example->indices)
    if (ns == index)
      return;

  m_example->indices.push_back(index);
}

VowpalWabbitNamespaceBuilder::~VowpalWabbitNamespaceBuilder()
{ this->!VowpalWabbitNamespaceBuilder();
}

VowpalWabbitNamespaceBuilder::!VowpalWabbitNamespaceBuilder()
{ // Multiple builders can be open on the same feature group at the same time, so reconcile against what
  // the group actually holds rather than against what this builder contributed.
  // temp is a native copy: push_back takes a const reference, and a field of a managed class cannot
  // bind to one.
  unsigned char temp = m_index;
  bool has_features = m_features->size() > 0;

  for (auto i = m_example->indices.begin(); i != m_example->indices.end(); i++)
  { if (*i != temp)
      continue;

    if (!has_features)
      m_example->indices.erase(i);

    return;
  }

  if (has_features)
    m_example->indices.push_back(temp);
}

void VowpalWabbitNamespaceBuilder::AddFeaturesUnchecked(uint64_t weight_index_base, float* begin, float* end)
{ for (; begin != end; begin++)
  { float x = *begin;
    if (x != 0)
    { m_features->values.push_back_unchecked(x);
      m_features->indices.push_back_unchecked(weight_index_base);
    }
    weight_index_base++;
  }
}

void VowpalWabbitNamespaceBuilder::AddFeature(uint64_t weight_index, float x)
{ // filter out 0-values
  if (x == 0)
    return;

  m_features->push_back(x, weight_index);
}

void VowpalWabbitNamespaceBuilder::PreAllocate(int size)
{ m_features->values.reserve(m_features->values.size() + size);
  m_features->indices.reserve(m_features->indices.size() + size);
}

size_t VowpalWabbitNamespaceBuilder::FeatureCount::get()
{ return m_features->size();
}
}
