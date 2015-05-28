// vw_clr.h

#pragma once

// #include <codecvt>

#include "vw.h"
#include "parser.h"
// #include "lda_core.h"

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace System::Runtime::InteropServices;

namespace Microsoft
{
	namespace Research 
	{
		namespace MachineLearning 
		{
			[StructLayout(LayoutKind::Sequential)]
			public value struct FEATURE
			{
			public:
				float x;
				UInt32 weight_index;
			};

			public ref class FeatureSpace
			{
			public: 
				property cli::array<FEATURE>^ Features;
				property unsigned char Name;
			};

			public interface class IVowpalWabbitExample : public IDisposable
			{
			public:
				property bool IsNewLine
				{
					virtual bool get() = 0;
				}

				property float CostSensitivePrediction
				{
					virtual float get() = 0;
				}

				property cli::array<int>^ MultilabelPredictions
				{
					virtual cli::array<int>^ get() = 0;
				}

				property cli::array<float>^ TopicPredictions
				{
					virtual cli::array<float>^ get() = 0;
				}

				virtual float Learn() = 0;

				virtual float Predict() = 0;

				virtual System::String^ Diff(IVowpalWabbitExample^ other, bool sameOrder) = 0;

				// this performs the finish_example of the learning algorithm
				// if examples are not allocated within VW's ring
				// memory will not be released. 
				// The actual release of memory is done in Dipose.
				virtual void Finish() = 0;
			};

			public ref class VowpalWabbitExample : public IVowpalWabbitExample
			{
			private:
				vw* const m_vw;
				example* m_example;

			protected:
				bool m_isDisposed;
				!VowpalWabbitExample();

			internal :
				VowpalWabbitExample(vw* vw, example* example);

			public:

				~VowpalWabbitExample();

				property bool IsNewLine
				{
					virtual bool get();
				}

				property float CostSensitivePrediction
				{
					virtual float get();
				}
					
				property cli::array<int>^ MultilabelPredictions
				{
					virtual cli::array<int>^ get();
				}

				property cli::array<float>^ TopicPredictions
				{
					virtual cli::array<float>^ get();
				}

				virtual float Learn();

				virtual float Predict();

				virtual System::String^ Diff(IVowpalWabbitExample^ other, bool sameOrder);

				virtual void Finish();
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
				
			protected:
				bool m_isDisposed;

				VowpalWabbitBase(System::String^ pArgs);
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
				VowpalWabbitModel(System::String^ pArgs);
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

			protected:
				!VowpalWabbit();

			public:
				VowpalWabbit(System::String^ pArgs);
				VowpalWabbit(VowpalWabbitModel^ model);
				virtual ~VowpalWabbit();

				uint32_t HashSpace(System::String^ s);
				uint32_t HashFeature(System::String^ s, unsigned long u);

				VowpalWabbitExample^ ReadExample(System::String^ line);
				VowpalWabbitExample^ CreateEmptyExample();
			};
		}
	}
}