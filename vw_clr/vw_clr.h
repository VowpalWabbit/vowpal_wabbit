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
		!VowpalWabbitExample();

	internal :
		VowpalWabbitExample(VowpalWabbitBase^ vw, example* example);
        initonly VowpalWabbitBase^ m_vw;
		example* m_example;

	public:

		~VowpalWabbitExample();

		virtual void Learn();
		virtual void PredictAndDiscard();

		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		virtual TPrediction LearnAndPredict();

		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		virtual TPrediction Predict();

		virtual property VowpalWabbitExample^ UnderlyingExample
		{
			VowpalWabbitExample^ get();
		}
	};
			
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

	// Since the model class must delay diposal of m_vw until all referencing
	// VowpalWabbit instances are disposed, the base class does not dispose m_vw
	public ref class VowpalWabbitBase abstract
	{
	internal:
		vw* m_vw;

        example* GetOrCreateNativeExample();
        void ReturnExampleToPool(example*);
				
	protected:
		bool m_isDisposed;

		VowpalWabbitBase(String^ args);
		VowpalWabbitBase(String^ args, System::IO::Stream^ model);
		VowpalWabbitBase(vw* vw);

		void InternalDispose();

	public:
		void RunMultiPass();
		void SaveModel();
		void SaveModel(String^ filename);

		property VowpalWabbitPerformanceStatistics^ PerformanceStatistics
		{
			VowpalWabbitPerformanceStatistics^ get();
		}

        stack<example*>* m_examples;
	};

	/// <summary>
	/// VowpalWabbit model wrapper.
	/// </summary>
	public ref class VowpalWabbitModel : public VowpalWabbitBase
	{
	internal:
		System::Int32 m_instanceCount;

		void IncrementReference();
		void DecrementReference();
		!VowpalWabbitModel();

	public:
		VowpalWabbitModel(String^ args);
		VowpalWabbitModel(String^ args, System::IO::Stream^ stream);
		virtual ~VowpalWabbitModel();
	};

	public ref class VowpalWabbitNamespaceBuilder
	{
	private:
		float* m_sum_feat_sq;
		v_array<feature>* m_atomic;
	internal:
		VowpalWabbitNamespaceBuilder(float* sum_feat_sq, v_array<feature>* atomic);

	public:
		void AddFeature(uint32_t weight_index, float x);
	};

	public ref class VowpalWabbitExampleBuilder
	{
	private:
		vw* const m_vw;
		example* m_example;
		VowpalWabbitExample^ m_clrExample;

	protected:
		!VowpalWabbitExampleBuilder();

	public:
		VowpalWabbitExampleBuilder(VowpalWabbitBase^ vw);
		~VowpalWabbitExampleBuilder();

		VowpalWabbitExample^ CreateExample();

		property String^ Label
		{
			void set(String^ value);
		}

		VowpalWabbitNamespaceBuilder^ AddNamespace(System::Byte featureGroup);
	};

	/// <summary>
	/// VowpalWabbit wrapper
	/// </summary>
	public ref class VowpalWabbit : VowpalWabbitBase
	{
	private:
		VowpalWabbitModel^ m_model;

		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		TPrediction PredictOrLearn(String^ line, bool predict);

	protected:
		!VowpalWabbit();

	public:
		VowpalWabbit(String^ args);
		VowpalWabbit(VowpalWabbitModel^ model);
		VowpalWabbit(String^ args, System::IO::Stream^ stream);
		virtual ~VowpalWabbit();

		uint32_t HashSpace(String^ s);
		uint32_t HashFeature(String^ s, unsigned long u);

		void Learn(String^ line);
		void Predict(String^ line);

		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		TPrediction Learn(String^ line);

		generic<typename TPrediction>
			where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
		TPrediction Predict(String^ line);

		void Driver();
	};

	[Serializable]
	public ref class VowpalWabbitException : Exception
	{
	private:
		String^ m_filename;
		Int32 m_lineNumber;

	public:
		VowpalWabbitException(const vw_exception& ex);

		property String^ Filename
		{
			String^ get();
		}

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