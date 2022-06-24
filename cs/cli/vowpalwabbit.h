// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#pragma once

#include "vw_clr.h"
#include "vw_base.h"
#include "vw_model.h"
#include "vw_prediction.h"
#include "vw_interface.h"

namespace VW
{
ref class VowpalWabbitExampleBuilder;
ref struct VowpalWabbitFeature;

/// <summary>
/// Simple string example based wrapper for vowpal wabbit.
/// </summary>
/// <remarks>If possible use VowpalWabbit{T} types as this wrapper suffers from marshalling performance wise.</remarks>
public ref class VowpalWabbit : VowpalWabbitBase, IVowpalWabbitExamplePool
{
private:
  /// <summary>
  /// Select the right hash method based on args.
  /// </summary>
  Func<String^, size_t, size_t>^ GetHasher();

  /// <summary>
  /// The selected hasher method.
  /// </summary>
  /// <remarks>
  /// Avoiding if-else for hash function selection. Delegates outperform function pointers according to http://stackoverflow.com/questions/13443250/performance-of-c-cli-function-pointers-versus-net-delegates
  /// </remarks>
  initonly Func<String^, size_t, size_t>^ m_hasher;

  template<typename T>
  cli::array<cli::array<float>^>^ FillTopicAllocation(T& weights);

  /// <summary>
  /// Write and empty line example to vw cache file.
  /// </summary>
  /// <remarks>
  /// This is used to emit empty lines to cache while handling multiline examples.
  /// Used internally by Learn(IEnumerable&lt;String&gt; lines)
  /// </remarks>
  void CacheEmptyLine();

public:
  /// <summary>
  /// Initializes a new <see cref="VowpalWabbit"/> instance.
  /// </summary>
  /// <param name="settings">The settings.</param>
  VowpalWabbit(VowpalWabbitSettings^ settings);

  /// <summary>
  /// Initializes a new <see cref="VowpalWabbit"/> instance.
  /// </summary>
  /// <param name="args">Command line arguments.</param>
  VowpalWabbit(String^ args);

  /// <summary>
  /// Run multi-passe training.
  /// </summary>
  void RunMultiPass();

  /// <summary>
  /// Gets Collected performance statistics.
  /// </summary>
  property VowpalWabbitPerformanceStatistics^ PerformanceStatistics
  { VowpalWabbitPerformanceStatistics^ get();
  }

  /// <summary>
  /// Parses <paramref name="line"/> using the C++ parser.
  /// </summary>
  /// <returns>
  /// Returns a <see cref="VowpalWabbitExample"/> ready to be used for <see cref="Learn(VowpalWabbitExample^)"/> or <see cref="Predict(VowpalWabbitExample^)"/>.
  /// </returns>
  VowpalWabbitExample^ ParseLine(String^ line);

  /// <summary>
  /// Parses <paramref name="line"/> using the C++ parser.
  /// TODO: this should return VowpalWabbitExampleCollection, but that would require moving VowpalWaabitExampleCollection to C++/CLI
  /// </summary>
  /// <returns>
  /// Returns a <see cref="VowpalWabbitExample"/> ready to be used for <see cref="Learn(VowpalWabbitExample^)"/> or <see cref="Predict(VowpalWabbitExample^)"/>.
  /// </returns>
  List<VowpalWabbitExample^>^ ParseJson(String^ line);

  /// <summary>
  /// Parses <paramref name="json"/> using the C++ parser and supports the extra wrapping introduced by Decision Service.
  /// TODO: this should return VowpalWabbitExampleCollection, but that would require moving VowpalWaabitExampleCollection to C++/CLI
  /// TODO: the header should be passed along with the List of VowpalWabbit examples, but that requires additional care wrt disposing items.
  /// </summary>
  /// <param name="json">This needs to be null-terminated string.</param>
  /// <param name="copyJson">If true the json array is copied prior to destructive parsing</param>
  /// <returns>
  /// Returns a <see cref="VowpalWabbitExample"/> ready to be used for <see cref="Learn(VowpalWabbitExample^)"/> or <see cref="Predict(VowpalWabbitExample^)"/>.
  /// </returns>
  List<VowpalWabbitExample^>^ VowpalWabbit::ParseDecisionServiceJson(cli::array<Byte>^ json, int offset, int length, bool copyJson, [Out] VowpalWabbitDecisionServiceInteractionHeader^% header);

  /// <summary>
  /// Hashes the given namespace <paramref name="s"/>.
  /// </summary>
  /// <param name="s">String to be hashed.</param>
  /// <returns>The resulting hash code.</returns>
  /// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
  uint64_t HashSpaceNative(String^ s);

  /// <summary>
  /// Hashes the given namespace <paramref name="s"/>.
  /// </summary>
  /// <param name="s">String to be hashed.</param>
  /// <returns>The resulting hash code.</returns>
  /// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
  uint64_t HashSpace(String^ s);

  /// <summary>
  /// Hash the given feature <paramref name="s"/>.
  /// </summary>
  /// <param name="s">String to be hashed.</param>
  /// <param name="u">Hash offset.</param>
  /// <returns>The resulting hash code.</returns>
  /// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
  uint64_t HashFeatureNative(String^ s, size_t u);

  /// <summary>
  /// Hash the given feature <paramref name="s"/>.
  /// </summary>
  /// <param name="s">String to be hashed.</param>
  /// <param name="u">Hash offset.</param>
  /// <returns>The resulting hash code.</returns>
  /// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
  uint64_t HashFeature(String^ s, size_t u);

  /// <summary>
  /// Return full topic allocation [topic, feature].
  /// </summary>
  cli::array<cli::array<float>^>^ GetTopicAllocation();

  /// <summary>
  /// Return the <paramref name="top"/> topic weights.
  /// </summary>
  cli::array<System::Collections::Generic::List<VowpalWabbitFeature^>^>^ GetTopicAllocation(int top);

  /// <summary>
  /// The associated <see cref="VowpalWabbitBase"/> instance learns from this example and returns the prediction result for this example.
  /// </summary>
  /// <returns>The prediction result.</returns>
  /// <typeparam name="TPrediction">The prediction result type.</typeparam>
  generic<typename T> T Learn(VowpalWabbitExample^ example, IVowpalWabbitPredictionFactory<T>^ predictionFactory);

  /// <summary>
  /// Predicts for the given example.
  /// </summary>
  /// <typeparam name="T">The prediction type.</typeparam>
  /// <param name="example">Example to predict for.</param>
  /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
  /// <returns>The prediction for the given <paramref name="example"/>.</returns>
  generic<typename T> T Predict(VowpalWabbitExample^ example, IVowpalWabbitPredictionFactory<T>^ predictionFactory);

  /// <summary>
  /// Learns from the given example.
  /// </summary>
  /// <param name="example">Example to learn from.</param>
  void Learn(VowpalWabbitExample^ example);

  /// <summary>
  /// Learns from the given multiline example.
  /// </summary>
  /// <param name="examples">Example to learn from.</param>
  void Learn(List<VowpalWabbitExample^>^ examples);

  /// <summary>
  /// Predicts for the given example.
  /// </summary>
  /// <param name="example">Example to predict for.</param>
  void Predict(VowpalWabbitExample^ example);

  /// <summary>
  /// Predicts for the given multiline example.
  /// </summary>
  /// <param name="examples">Example to predict for.</param>
  void Predict(List<VowpalWabbitExample^>^ examples);

  /// <summary>
  /// Learns from string data.
  /// </summary>
  /// <param name="line">Data in vw string format.</param>
  void Learn(String^ line);

  /// <summary>
  /// Predicts for string data.
  /// </summary>
  /// <param name="line">Data in vw string format.</param>
  void Predict(String^ line);

  /// <summary>
  /// Learns from string data.
  /// </summary>
  /// <typeparam name="T">The prediction type.</typeparam>
  /// <param name="line">Data in vw string format.</param>
  /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
  /// <returns>The prediction for the given <paramref name="line"/>.</returns>
  generic<typename T> T Learn(String^ line, IVowpalWabbitPredictionFactory<T>^ predictionFactory);

  /// <summary>
  /// Predicts for string data.
  /// </summary>
  /// <typeparam name="T">The prediction type.</typeparam>
  /// <param name="line">Data in vw string format.</param>
  /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
  /// <returns>The prediction for the given <paramref name="line"/>.</returns>
  generic<typename T> T Predict(String^ line, IVowpalWabbitPredictionFactory<T>^ predictionFactory);

  /// <summary>
  /// Learns from multi-line examples.
  /// </summary>
  /// <param name="lines">Data in vw string format.</param>
  void Learn(IEnumerable<String^>^ lines);

  /// <summary>
  /// Predicts for multi-line examples.
  /// </summary>
  /// <param name="lines">Data in vw string format.</param>
  void Predict(IEnumerable<String^>^ lines);

  /// <summary>
  /// Learns from multi-line examples.
  /// </summary>
  /// <typeparam name="T">The prediction type.</typeparam>
  /// <param name="lines">Data in vw string format.</param>
  /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
  /// <returns>The prediction for the given <paramref name="lines"/>.</returns>
  generic<typename T> T Learn(IEnumerable<String^>^ lines, IVowpalWabbitPredictionFactory<T>^ predictionFactory);

  /// <summary>
  /// Predicts for the given lines.
  /// </summary>
  /// <typeparam name="T">The prediction type.</typeparam>
  /// <param name="lines">Data in vw string format.</param>
  /// <param name="predictionFactory">The prediction factory to be used. See <see cref="VowpalWabbitPredictionType"/>.</param>
  /// <returns>The prediction for the given <paramref name="lines"/>.</returns>
  generic<typename T> T Predict(IEnumerable<String^>^ lines, IVowpalWabbitPredictionFactory<T>^ predictionFactory);

  /// <summary>
  /// Signals the end of a pass.
  /// </summary>
  void EndOfPass();

  /// <summary>
  /// Invokes the driver.
  /// </summary>
  void Driver();

  virtual property VowpalWabbit^ Native
  { virtual VowpalWabbit^ get() sealed;
  }

  /// <summary>
  /// Gets or creates a native example from a CLR maintained, but natively allocated pool.
  /// </summary>
  /// <returns>A ready to use cleared native example data structure.</returns>
  virtual VowpalWabbitExample^ GetOrCreateNativeExample() sealed;

  /// <summary>
  /// Puts a native example data structure back into the pool.
  /// </summary>
  /// <param name="example">The example to be returned.</param>
  virtual void ReturnExampleToPool(VowpalWabbitExample^ example) sealed;
};
}
