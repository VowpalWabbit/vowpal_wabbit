/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw_example.h"

namespace VW
{
  ref class VowpalWabbitExample;

  /// <summary>
  /// Interface for label comparators.
  /// </summary>
  public interface class IVowpalWabbitLabelComparator
  {
  public:
    /// <summary>
    /// Compares labels of <paramref name="ex1"/> and <paramref name="ex2"/>.
    /// </summary>
    /// <returns>Returns null if labels are equivalent, otherwise returns the difference description.</returns>
    String^ Diff(VowpalWabbitExample^ ex1, VowpalWabbitExample^ ex2);
  };

  /// <summary>
  /// A label comparer for simple labels.
  /// </summary>
  public ref class VowpalWabbitSimpleLabelComparator sealed : IVowpalWabbitLabelComparator
  {
  public:
    /// <summary>
    /// Compares labels of <paramref name="ex1"/> and <paramref name="ex2"/>.
    /// </summary>
    /// <returns>Returns null if labels are equivalent, otherwise returns the difference description.</returns>
    virtual String^ Diff(VowpalWabbitExample^ ex1, VowpalWabbitExample^ ex2) sealed;
  };

  /// <summary>
  /// A label comparer for contextual bandit label.
  /// </summary>
  public ref class VowpalWabbitContextualBanditLabelComparator sealed : IVowpalWabbitLabelComparator
  {
    // TODO: create nested class in  VowpalWabbitLabelComparator
  public:
    /// <summary>
    /// Compares labels of <paramref name="ex1"/> and <paramref name="ex2"/>.
    /// </summary>
    /// <returns>Returns null if labels are equivalent, otherwise returns the difference description.</returns>
    virtual String^ Diff(VowpalWabbitExample^ ex1, VowpalWabbitExample^ ex2) sealed;
  };

  /// <summary>
  /// Factory 
  /// </summary>
  public ref class VowpalWabbitLabelComparator sealed abstract
  {
  public:
    /// <summary>
    /// Label comparator 
    /// </summary>
    static initonly IVowpalWabbitLabelComparator^ Simple = gcnew VowpalWabbitSimpleLabelComparator;

    static initonly IVowpalWabbitLabelComparator^ ContextualBandit = gcnew VowpalWabbitContextualBanditLabelComparator;
  };
}