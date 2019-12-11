// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw_builder.h"
#include "parser.h"

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
{
}

VowpalWabbitNamespaceBuilder::~VowpalWabbitNamespaceBuilder()
{ this->!VowpalWabbitNamespaceBuilder();
}

VowpalWabbitNamespaceBuilder::!VowpalWabbitNamespaceBuilder()
{ if (m_features->size() > 0)
  { unsigned char temp = m_index;

    // avoid duplicate insertion
    // can't check at the beginning, because multiple builders can be open
    // at the same time
    for (unsigned char ns : m_example->indices)
      if (ns == temp)
        return;

    m_example->indices.push_back(temp);
  }
}

void VowpalWabbitNamespaceBuilder::AddFeaturesUnchecked(uint64_t weight_index_base, float* begin, float* end)
{ for (; begin != end; begin++)
  { float x = *begin;
    if (x != 0)
    { m_features->values.push_back_unchecked(x);
      m_features->indicies.push_back_unchecked(weight_index_base);
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
{ m_features->values.resize(m_features->values.end() - m_features->values.begin() + size);
  m_features->indicies.resize(m_features->indicies.end() - m_features->indicies.begin() + size);
}

size_t VowpalWabbitNamespaceBuilder::FeatureCount::get()
{ return m_features->size();
}
}
