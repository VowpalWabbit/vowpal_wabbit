// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw_clr.h"
#include "vw_base.h"
#include "vw_example.h"
#include "vowpalwabbit.h"
#include "vw_label.h"

namespace VW
{
using namespace VW::Labels;

/// <summary>
/// Helper class to ease construction of native vowpal wabbit namespace data structure.
/// </summary>
public ref class VowpalWabbitNamespaceBuilder sealed
{
private:
  /// <summary>
  /// Features.
  /// </summary>
  features* m_features;

  /// <summary>
  /// The namespace index.
  /// </summary>
  unsigned char m_index;

  /// <summary>
  /// The native example.
  /// </summary>
  example* m_example;

  // float(*m_sum_of_squares)(float*, float*);

  !VowpalWabbitNamespaceBuilder();

internal:
  /// <summary>
  /// Initializes a new <see cref="VowpalWabbitNamespaceBuilder"/> instance.
  /// </summary>
  /// <param name="features">Pointer into features owned by <see cref="VowpalWabbitExample"/>.</param>
  /// <param name="index">The namespace index.</param>
  /// <param name="example">The native example to build up.</param>
  VowpalWabbitNamespaceBuilder(features* features, unsigned char index, example* example);

public:
  ~VowpalWabbitNamespaceBuilder();

  /// <summary>
  /// Add feature entry.
  /// </summary>
  /// <param name="weight_index">The weight index.</param>
  /// <param name="x">The value.</param>
  void AddFeature(uint64_t weight_index, float x);

  /// <summary>
  /// Adds a dense array to the example.
  /// </summary>
  /// <param name="weight_index_base">The base weight index. Each element is then placed relative to this index.</param>
  /// <param name="begin">The start pointer of the float array.</param>
  /// <param name="end">The end pointer of the float array.</param>
  void AddFeaturesUnchecked(uint64_t weight_index_base, float* begin, float* end);

  /// <summary>
  /// Pre-allocate features of <paramref name="size"/>.
  /// </summary>
  /// <param name="size">The number of features to pre-allocate.</param>
  void PreAllocate(int size);

  property size_t FeatureCount { size_t get(); }
};

/// <summary>
/// Helper class to ease construction of native vowpal wabbit example data structure.
/// </summary>
public ref class VowpalWabbitExampleBuilder sealed
{
private:
  IVowpalWabbitExamplePool^ m_vw;

  /// <summary>
  /// The produced CLR example data structure.
  /// </summary>
  VowpalWabbitExample^ m_example;

protected:
  /// <summary>
  /// Cleanup.
  /// </summary>
  !VowpalWabbitExampleBuilder();

public:
  /// <summary>
  /// Initializes a new <see cref="VowpalWabbitExampleBuilder"/> instance.
  /// </summary>
  /// <param name="vw">The parent vowpal wabbit instance.</param>
  VowpalWabbitExampleBuilder(IVowpalWabbitExamplePool^ vw);

  /// <summary>
  /// Cleanup.
  /// </summary>
  ~VowpalWabbitExampleBuilder();

  /// <summary>
  /// Creates the managed example representation.
  /// </summary>
  /// <returns>Creates the managed example.</returns>
  VowpalWabbitExample^ CreateExample();

  /// <summary>
  /// Sets the label for the resulting example.
  /// </summary>
  void ApplyLabel(ILabel^ label);

  /// <summary>
  /// Creates and adds a new namespace to this example.
  /// </summary>
  VowpalWabbitNamespaceBuilder^ AddNamespace(Byte featureGroup);

  /// <summary>
  /// Creates and adds a new namespace to this example.
  /// </summary>
  /// <param name="featureGroup">The feature group of the new namespace.</param>
  /// <remarks>Casts to System::Byte.</remarks>
  VowpalWabbitNamespaceBuilder^ AddNamespace(Char featureGroup);
};
}
