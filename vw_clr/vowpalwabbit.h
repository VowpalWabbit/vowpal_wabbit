/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#include "vw_clr.h"
#include "vw_base.h"
#include "vw_model.h"
#include "vw_prediction.h"
#include "vw_interface.h"

namespace VW
{
  ref class VowpalWabbitExampleBuilder;

  /// <summary>
  /// Simple string example based wrapper for vowpal wabbit.
  /// </summary>
  /// <remarks>If possible use VowpalWabbit{T} types as this wrapper suffers from marshalling performance wise.</remarks>
  public ref class VowpalWabbit : VowpalWabbitBase
  {
  private:
    /// <summary>
    /// Select the right hash method based on args.
    /// </summary>
    Func<String^, unsigned long, size_t>^ GetHasher();

    /// <summary>
    /// The selected hasher method.
    /// </summary>
    /// <remarks>
    /// Avoiding if-else for hash function selection. Delegates outperform function pointers according to http://stackoverflow.com/questions/13443250/performance-of-c-cli-function-pointers-versus-net-delegates
    /// </remarks>
    initonly Func<String^, unsigned long, size_t>^ m_hasher;

    VowpalWabbitExample^ ParseLine(String^ line);

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
    /// Persist model to file specified by -i.
    /// </summary>
    void SaveModel();

    /// <summary>
    /// Persist model to <paramref name="filename"/>.
    /// </summary>
    /// <param name="filename">The destination filename for the model.</param>
    void SaveModel(String^ filename);

    /// <summary>
    /// Gets Collected performance statistics.
    /// </summary>
    property VowpalWabbitPerformanceStatistics^ PerformanceStatistics
    {
      VowpalWabbitPerformanceStatistics^ get();
    }

    /// <summary>
    /// Hashes the given namespace <paramref name="s"/>.
    /// </summary>
    /// <param name="s">String to be hashed.</param>
    /// <returns>The resulting hash code.</returns>
    /// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
    uint32_t HashSpaceNative(String^ s);

    /// <summary>
    /// Hashes the given namespace <paramref name="s"/>.
    /// </summary>
    /// <param name="s">String to be hashed.</param>
    /// <returns>The resulting hash code.</returns>
    /// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
    uint32_t HashSpace(String^ s);

    /// <summary>
    /// Hash the given feature <paramref name="s"/>.
    /// </summary>
    /// <param name="s">String to be hashed.</param>
    /// <param name="u">Hash offset.</param>
    /// <returns>The resulting hash code.</returns>
    /// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
    uint32_t HashFeatureNative(String^ s, unsigned long u);

    /// <summary>
    /// Hash the given feature <paramref name="s"/>.
    /// </summary>
    /// <param name="s">String to be hashed.</param>
    /// <param name="u">Hash offset.</param>
    /// <returns>The resulting hash code.</returns>
    /// <remarks>The hash code depends on the vowpal wabbit instance as different has functions can be configured.</remarks>
    uint32_t HashFeature(String^s, unsigned long u);

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
    /// Predicts for the given example.
    /// </summary>
    /// <param name="example">Example to predict for.</param>
    void Predict(VowpalWabbitExample^ example);

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
  };
}
