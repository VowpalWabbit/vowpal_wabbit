// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.
#pragma once

#include "spanning_tree.h"

namespace VW
{
/// <summary>
/// Managed wrapper for AllReduce spanning tree implementation.
/// </summary>
public ref class SpanningTreeClr
{
private:
  SpanningTree* m_spanningTree;

public:
  /// <summary>
  /// Initializes a new <see cref="SpanningTreeClr"/> instance.
  /// </summary>
  SpanningTreeClr();

  ~SpanningTreeClr();

  /// <summary>
  /// Starts the server on a background thread.
  /// </summary>
  void Start();

  /// <summary>
  /// Runs the server on the calling thread.
  /// </summary>
  void Run();

  /// <summary>
  /// Stops the background thread.
  /// </summary>
  void Stop();
};
}
