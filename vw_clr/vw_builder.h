/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw_clr.h"
#include "vw_base.h"
#include "vw_example.h"
#include "vowpalwabbit.h"

namespace VW
{
  /// <summary>
  /// Helper class to ease construction of native vowpal wabbit namespace data structure.
  /// </summary>
  public ref class VowpalWabbitNamespaceBuilder
  {
  private:
    /// <summary>
    /// Sum of squares.
    /// </summary>
    float* m_sum_feat_sq;

    /// <summary>
    /// Features.
    /// </summary>
    v_array<feature>* m_atomic;

  internal:
    /// <summary>
    /// Initializes a new <see cref="VowpalWabbitNamespaceBuilder"/> instance.
    /// </summary>
    /// <param name="sum_feat_sq">Pointer into sum squares array owned by <see cref="VowpalWabbitExample"/>.</param>
    /// <param name="atomic">Pointer into atomics owned by <see cref="VowpalWabbitExample"/>.</param>
    VowpalWabbitNamespaceBuilder(float* sum_feat_sq, v_array<feature>* atomic);

  public:
    /// <summary>
    /// Add feature entry.
    /// </summary>
    /// <param name="weight_index">The weight index.</param>
    /// <param name="x">The value.</param>
    void AddFeature(uint32_t weight_index, float x);

    /// <summary>
    /// Pre-allocate features of <paramref name="size"/>.
    /// </summary>
    /// <param name="size">The number of features to pre-allocate.</param>
    void PreAllocate(int size);
  };

  /// <summary>
  /// Helper class to ease construction of native vowpal wabbit example data structure.
  /// </summary>
  public ref class VowpalWabbitExampleBuilder
  {
  private:
    /// <summary>
    /// The native vowpal wabbit data structure.
    /// </summary>
    vw* const m_vw;

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
    VowpalWabbitExampleBuilder(VowpalWabbit^ vw);

    /// <summary>
    /// Cleanup.
    /// </summary>
    ~VowpalWabbitExampleBuilder();

    /// <summary>
    /// Creates the managed example representation.
    /// </summary>
    VowpalWabbitExample^ CreateExample();

    /// <summary>
    /// Sets the label for the resulting example.
    /// </summary>
    void ParseLabel(String^ value);

    /// <summary>
    /// Creates and adds a new namespace to this example.
    /// </summary>
    VowpalWabbitNamespaceBuilder^ AddNamespace(Byte featureGroup);

    /// <summary>
    /// Creates and adds a new namespace to this example.
    /// </summary>
    /// <remarks>Casts to System::Byte.</remarks>
    VowpalWabbitNamespaceBuilder^ AddNamespace(Char featureGroup);
  };
}
