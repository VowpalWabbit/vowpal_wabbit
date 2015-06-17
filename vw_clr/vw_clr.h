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

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace Microsoft
{
	namespace Research 
	{
		namespace MachineLearning 
		{
			ref class VowpalWabbitExample;
            ref class VowpalWabbitBase;

			public ref class VowpalWabbitPrediction abstract
			{
			public:
				void ReadFromExample(VowpalWabbitExample^ example);

				virtual void ReadFromExample(vw* vw, example* ex) abstract;
			};

			public ref class VowpalWabbitScalarPrediction : VowpalWabbitPrediction
			{
			public:
				void ReadFromExample(vw* vw, example* ex) override;

				property float Value;
			};

			public ref class VowpalWabbitCostSensitivePrediction : VowpalWabbitPrediction
			{
			public:
				void ReadFromExample(vw* vw, example* ex) override;

				property float Value;
			};

			public ref class VowpalWabbitMultilabelPrediction : VowpalWabbitPrediction
			{
			public:
				void ReadFromExample(vw* vw, example* ex) override;

				property cli::array<int>^ Values;
			};

			public ref class VowpalWabbitTopicPrediction : VowpalWabbitPrediction
			{
			public:
				void ReadFromExample(vw* vw, example* ex) override;

				property cli::array<float>^ Values;
			};

			public ref class VowpalWabbitPredictionNone : VowpalWabbitPrediction
			{
			public:
				void ReadFromExample(vw* vw, example* ex) override;
			};

			public interface class IVowpalWabbitExample : public IDisposable
			{
			public:
				generic<typename TPrediction>
					where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
				virtual TPrediction Learn() = 0;

				generic<typename TPrediction>
					where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
				virtual TPrediction Predict() = 0;

				virtual property VowpalWabbitExample^ UnderlyingExample
				{
					VowpalWabbitExample^ get() = 0;
				}
			};

			public ref class VowpalWabbitExample : public IVowpalWabbitExample
			{
			private:
				generic<typename TPrediction>
					where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
				TPrediction PredictOrLearn(bool predict);

			protected:
				!VowpalWabbitExample();

			internal :
				VowpalWabbitExample(VowpalWabbitBase^ vw, example* example);
                initonly VowpalWabbitBase^ m_vw;
				example* m_example;

			public:

				~VowpalWabbitExample();

				generic<typename TPrediction>
					where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
				virtual TPrediction Learn();

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

            internal:
                example* GetOrCreateNativeExample();
                void ReturnExampleToPool(example*);
				
			protected:
				bool m_isDisposed;

				VowpalWabbitBase(System::String^ args);
				VowpalWabbitBase(System::String^ args, System::IO::Stream^ model);
				VowpalWabbitBase(vw* vw);

				void InternalDispose();

			public:
				void RunMultiPass();
				void SaveModel();
				void SaveModel(System::String^ filename);

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
				VowpalWabbitModel(System::String^ args);
				VowpalWabbitModel(System::String^ args, System::IO::Stream^ stream);
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

				property System::String^ Label
				{
					void set(System::String^ value);
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
				TPrediction PredictOrLearn(System::String^ line, bool predict);

			protected:
				!VowpalWabbit();

			public:
				VowpalWabbit(System::String^ args);
				VowpalWabbit(VowpalWabbitModel^ model);
				VowpalWabbit(System::String^ args, System::IO::Stream^ stream);
				virtual ~VowpalWabbit();

				uint32_t HashSpace(System::String^ s);
				uint32_t HashFeature(System::String^ s, unsigned long u);

				generic<typename TPrediction>
					where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
				TPrediction Learn(System::String^ line);

				generic<typename TPrediction>
					where TPrediction : VowpalWabbitPrediction, gcnew(), ref class
				TPrediction Predict(System::String^ line);

				void Driver();
			};
		}
	}
}