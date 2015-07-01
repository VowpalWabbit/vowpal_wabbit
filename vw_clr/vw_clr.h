/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once

#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif

#include "vw.h"
#include <stack>
#include "parser.h"
#include "vw_exception.h"

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace VW
{
	ref class VowpalWabbitExample;
    ref class VowpalWabbitBase;

	/// <summary>
	/// Base-class for prediction results. 
	/// </summary>
	public ref class VowpalWabbitPrediction abstract
	{
	public:
		/// <summary>
		/// Extracts data and forwards to <see cref="ReadFromExample(vw*, example*)" />
		/// </summary>
		void ReadFromExample(VowpalWabbitExample^ example);
		
		/// <summary>
		/// Subclasses must extract the prediction result from the example.
		/// </summary>
		virtual void ReadFromExample(vw* vw, example* ex) abstract;
	};

	/// <summary>
	/// A scalar prediction result.
	/// </summary>
	public ref class VowpalWabbitScalarPrediction : VowpalWabbitPrediction
	{
	public:
		/// <summary>
		/// Extracts prediction results from example.
		/// </summary>
		void ReadFromExample(vw* vw, example* ex) override;

		/// <summary>
		/// The scalar prediction.
		/// </summary>
		property float Value;
	};

	/// <summary>
	/// A cost sensitive prediction result.
	/// </summary>
	public ref class VowpalWabbitCostSensitivePrediction : VowpalWabbitPrediction
	{
	public:
		/// <summary>
		/// Extracts prediction results from example.
		/// </summary>
		void ReadFromExample(vw* vw, example* ex) override;

		/// <summary>
		/// The cost sensitive prediction.
		/// </summary>
		property float Value;
	};

	/// <summary>
	/// A multi label prediction result.
	/// </summary>
	public ref class VowpalWabbitMultilabelPrediction : VowpalWabbitPrediction
	{
	public:
		/// <summary>
		/// Extracts prediction results from example.
		/// </summary>
		void ReadFromExample(vw* vw, example* ex) override;

		/// <summary>
		/// The predicted labels.
		/// </summary>
		property cli::array<int>^ Values;
	};

	/// <summary>
	/// A topic prediction result.
	/// </summary>
	public ref class VowpalWabbitTopicPrediction : VowpalWabbitPrediction
	{
	public:
		/// <summary>
		/// Extracts prediction results from example.
		/// </summary>
		void ReadFromExample(vw* vw, example* ex) override;

		/// <summary>
		/// The predicted topics.
		/// </summary>
		property cli::array<float>^ Values;
	};

	/// <summary>
	/// Interface defining a vowpal wabbit example. 
	/// </summary>
	public interface class IVowpalWabbitExample : public IDisposable
	{
	public:
		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance learns from this example.
		/// </summary>
		virtual void Learn() = 0;

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance predicts an outcome using this example and discards the result.
		/// </summary>
		/// <remarks>Used with multi-line examples.</remarks>
		virtual void PredictAndDiscard() = 0;

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance learns from this example and returns the prediction result for this example.
		/// </summary>
		/// <returns>The prediction result.</returns>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		virtual TPrediction LearnAndPredict() = 0;

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance predicts an outcome using this example.
		/// </summary>
		/// <returns>The prediction result.</returns>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		virtual TPrediction Predict() = 0;

		/// <summary>
		/// The underlying <see cref="VowpalWabbitExample"/> this instance wraps.
		/// </summary>
		virtual property VowpalWabbitExample^ UnderlyingExample
		{
			VowpalWabbitExample^ get() = 0;
		}
	};

	/// <summary>
	/// A CLR representation of a vowpal wabbit example.
	/// </summary>
	/// <remarks>
	/// Underlying memory is allocated by native code, but examples are not part of the ring.
	/// To optimize performance each <see cref="VowpalWabbitBase"/> instance maintains an example pool.
	/// </remarks>
	public ref class VowpalWabbitExample : public IVowpalWabbitExample
	{
	protected:
		/// <summary>
		/// Returns native example data structure to parent <see cref="VowpalWabbitBase"/> instance.
		/// </summary>
		!VowpalWabbitExample();

	internal :
		/// <summary>
		/// Initializes a new instance of <see cref="VowpalWabbitExample"/>.
		/// </summary>
		/// <param name="vw">The parent instance. Examples cannot be shared between <see cref="VowpalWabbitBase"/> instances.</param>
		/// <param name="example">The already allocated example structure</param>
		VowpalWabbitExample(VowpalWabbitBase^ vw, example* example);

		/// <summary>
		/// The parent instance. Examples cannot be shared between <see cref="VowpalWabbitBase"/> instances.
		/// </summary>
        initonly VowpalWabbitBase^ m_vw;

		/// <summary>
		/// The native example data structure.
		/// </summary>
		example* m_example;

	public:
		/// <summary>
		/// Returns native example data structure to parent <see cref="VowpalWabbitBase"/> instance.
		/// </summary>
		~VowpalWabbitExample();

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance learns from this example.
		/// </summary>
		virtual void Learn();

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance predicts an outcome using this example and discards the result.
		/// </summary>
		/// <remarks>Used with multi-line examples.</remarks>
		virtual void PredictAndDiscard();

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance learns from this example and returns the prediction result for this example.
		/// </summary>
		/// <returns>The prediction result.</returns>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		virtual TPrediction LearnAndPredict();

		/// <summary>
		/// The associated <see cref="VowpalWabbitBase"/> instance predicts an outcome using this example.
		/// </summary>
		/// <returns>The prediction result.</returns>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		virtual TPrediction Predict();

		/// <summary>
		/// As this is the acutal example, it simply returns this.
		/// </summary>
		virtual property VowpalWabbitExample^ UnderlyingExample
		{
			VowpalWabbitExample^ get();
		}
	};
			
	/// <summary>
	/// Collected erformance statistics.
	/// </summary>
	public ref class VowpalWabbitPerformanceStatistics
	{
	public:
		property uint64_t TotalNumberOfFeatures;

		property double WeightedExampleSum;

		property uint64_t NumberOfExamplesPerPass;

		property double WeightedLabelSum;

		property double AverageLoss;

		property double BestConstant;

		property double BestConstantLoss;
	};

	/// <summary>
	/// A base wrapper around vowpal wabbit machine learning instance.
	/// </summary>
	/// <remarks>
	/// Since the model class must delay diposal of <see cref="m_vw"/> until all referencing
	/// VowpalWabbit instances are disposed, the base class does not dispose <see cref="m_vw"/>.
	/// </remarks>
	public ref class VowpalWabbitBase abstract
	{
	private:
		/// <summary>
		/// The CLR maintained, but natively allocated example pool.
		/// </summary>
		stack<example*>* m_examples;

	internal:
		/// <summary>
		/// The native vowpal wabbit data structure.
		/// </summary>
		vw* m_vw;

		/// <summary>
		/// Gets or creates a native example from a CLR maintained, but natively allocated pool.
		/// </summary>
		/// <returns>A ready to use cleared native example data structure.</returns>
        example* GetOrCreateNativeExample();

		/// <summary>
		/// Puts a native example data structure back into the pool.
		/// </summary>
		/// <param name="ex">The example to be returned.</param>
        void ReturnExampleToPool(example* ex);
				
	protected:
		/// <summary>
		/// True if all nativedata structures are disposed.
		/// </summary>
		bool m_isDisposed;

		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitBase"/> instance. 
		/// </summary>
		/// <param name="args">Command line arguments.</param>
		VowpalWabbitBase(String^ args);

		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitBase"/> instance. 
		/// </summary>
		/// <param name="args">Command line arguments.</param>
		/// <param name="model">Model used for initialization.</param>
		VowpalWabbitBase(String^ args, System::IO::Stream^ model);

		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitBase"/> instance. 
		/// </summary>
		/// <param name="vw">Shared native vowpwal wabbit data structure.</param>
		VowpalWabbitBase(vw* vw);

		/// <summary>
		/// Internal dipose using reference counting to delay disposal of shared native data structures. 
		/// </summary>
		void InternalDispose();

	public:
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
	};

	/// <summary>
	/// VowpalWabbit model wrapper.
	/// </summary>
	public ref class VowpalWabbitModel : public VowpalWabbitBase
	{
	private:
		/// <summary>
		/// Reference count.
		/// </summary>
		System::Int32 m_instanceCount;

	internal:
		/// <summary>
		/// Thread-safe increment of reference count.
		/// </summary>
		void IncrementReference();

		/// <summary>
		/// Thread-safe decrement of reference count.
		/// </summary>
		void DecrementReference();

		/// <summary>
		/// Cleanup.
		/// </summary>
		!VowpalWabbitModel();

	public:
		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitModel"/> instance. 
		/// </summary>
		/// <param name="args">Command line arguments.</param>
		VowpalWabbitModel(String^ args);

		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitModel"/> instance. 
		/// </summary>
		/// <param name="args">Command line arguments.</param>
		/// <param name="model">Model used for initialization.</param>
		VowpalWabbitModel(String^ args, System::IO::Stream^ model);

		/// <summary>
		/// Cleanup.
		/// </summary>
		virtual ~VowpalWabbitModel();
	};

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
	};

	/// <summary>
	/// Helper class to ease construction of native vowpal wabbit example data structure.
	/// </summary>
	public ref class VowpalWabbitExampleBuilder
	{
	private:
		/// <summary>
		/// The parent vowpal wabbit instance.
		/// </summary>
		vw* const m_vw;

		/// <summary>
		/// The resulting native example data structure.
		/// </summary>
		example* m_example;

		/// <summary>
		/// The resulting CLR example data structure.
		/// </summary>
		VowpalWabbitExample^ m_clrExample;

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
		VowpalWabbitExampleBuilder(VowpalWabbitBase^ vw);

		/// <summary>
		/// Cleanup.
		/// </summary>
		~VowpalWabbitExampleBuilder();

		/// <summary>
		/// Creates the managed example representation.
		/// </summary>
		VowpalWabbitExample^ CreateExample();

		/// <summary>
		/// Sets the lael for the resulting example.  
		/// </summary>
		property String^ Label
		{
			void set(String^ value);
		}

		/// <summary>
		/// Creates and adds a new namespace to this example.
		/// </summary>
		VowpalWabbitNamespaceBuilder^ AddNamespace(System::Byte featureGroup);
	};

	/// <summary>
	/// Simple string example based wrapper for vowpal wabbit.  
	/// </summary>
	/// <remarks>If possible use VowpalWabbit{T} types as this wrapper suffers from marshalling performance wise.</remarks> 
	public ref class VowpalWabbit : VowpalWabbitBase
	{
	private:
		/// <summary>
		/// The shared vowpal wabbit model.
		/// </summary>
		VowpalWabbitModel^ m_model;

		/// <summary>
		/// Learns from or predicts the given example (<paramref name="line"/>) and returns the prediction result.
		/// </summary>
		/// <returns>The prediction result.</returns>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		TPrediction PredictOrLearn(String^ line, bool predict);

	protected:
		/// <summary>
		/// Cleanup.
		/// </summary>
		!VowpalWabbit();

	public:
		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitBase"/> instance. 
		/// </summary>
		/// <param name="args">Command line arguments.</param>
		VowpalWabbit(String^ args);

		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitBase"/> instance. 
		/// </summary>
		/// <param name="model">The shared model.</param>
		VowpalWabbit(VowpalWabbitModel^ model);

		/// <summary>
		/// Initializes a new <see cref="VowpalWabbitBase"/> instance. 
		/// </summary>
		/// <param name="args">Command line arguments.</param>
		/// <param name="model">Model used for initialization.</param>
		VowpalWabbit(String^ args, System::IO::Stream^ model);

		/// <summary>
		/// Cleanup.
		/// </summary>
		virtual ~VowpalWabbit();

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
		uint32_t HashFeature(String^ s, unsigned long u);

		/// <summary>
		/// Learns from the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		void Learn(String^ line);

		/// <summary>
		/// Predicts based on the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		void Predict(String^ line);

		/// <summary>
		/// Learns from the given example and returns the prediction for the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		TPrediction Learn(String^ line);

		/// <summary>
		/// Predicts based on the given example.
		/// </summary>
		/// <param name="line">The example in vowpal wabbit string format.</param>
		/// <typeparam name="TPrediction">The prediction result type.</typeparam>
		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		TPrediction Predict(String^ line);

		/// <summary>
		/// Invokes the driver.
		/// </summary>
		void Driver();
	};

	/// <summary>
	/// A managed wrapper for native vowpal wabbit exceptions.
	/// </summary>
	/// <remarks>
	/// As the default managed exception wrapping any native exception doesn't even capture exception::what()
	/// this wrapper was created.
	/// </remarks>
	[Serializable]
	public ref class VowpalWabbitException : Exception
	{
	private:
		/// <summary>
		/// The source filename in which the wrapped exception occurred.
		/// </summary>
		String^ m_filename;

		/// <summary>
		/// The line number in which the wrapped exception occurred.
		/// </summary>
		Int32 m_lineNumber;

	public:
		/// <summary>
		/// Initializes a new instance of <see cref="VowpalWabbitException"/>.
		/// </summary>
		/// <param name="ex">The native vowpal wabbit exception</param>
		VowpalWabbitException(const vw_exception& ex);

		/// <summary>
		/// Gets the source filename in which the wrapped exception occurred.
		/// </summary>
		property String^ Filename
		{
			String^ get();
		}

		/// <summary>
		/// Gets the line number in which the wrapped exception occurred.
		/// </summary>
		property Int32 LineNumber
		{
			Int32 get();
		}
	};
}

#define CATCHRETHROW \
catch (VW::vw_exception const& ex) \
{ throw gcnew VW::VowpalWabbitException(ex); } \
catch (std::exception const& ex) \
{ throw gcnew System::Exception(gcnew System::String(ex.what())); }