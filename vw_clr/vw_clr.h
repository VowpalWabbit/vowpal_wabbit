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

				virtual void AddLabel(System::String^ label) = 0;

				virtual void AddLabel(float label) = 0;

				virtual void AddLabel(float label, float weight) = 0;

				virtual void AddLabel(float label, float weight, float base) = 0;

				virtual float Learn() = 0;

				virtual float Predict() = 0;

				virtual System::String^ Diff(IVowpalWabbitExample^ other, bool sameOrder) = 0;
			};

			public ref class VowpalWabbitExample : public IVowpalWabbitExample
			{
			private:
				vw* const m_vw;
				example* const m_example;

			protected:
				bool m_isDisposed;
				!VowpalWabbitExample();

			public:
				VowpalWabbitExample(vw* vw, example* example);

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

				virtual void AddLabel(System::String^ label);

				virtual void AddLabel(float label);
				
				virtual void AddLabel(float label, float weight);

				virtual void AddLabel(float label, float weight, float base);

				virtual float Learn();

				virtual float Predict();

				virtual System::String^ Diff(IVowpalWabbitExample^ other, bool sameOrder);
			};

			public ref class VowpalWabbitBase abstract
			{
			internal:
				vw* m_vw;
			
			protected:
				bool m_isDisposed;

				VowpalWabbitBase(System::String^ pArgs);
				VowpalWabbitBase(vw* vw);

				!VowpalWabbitBase();

			public:
				~VowpalWabbitBase();

				void RunMultiPass();
				void SaveModel();
				void SaveModel(System::String^ filename);
			};

			/// <summary>
			/// VowpalWabbit model wrapper.
			/// </summary>
			public ref class VowpalWabbitModel : VowpalWabbitBase
			{
			public:
				VowpalWabbitModel(System::String^ pArgs);
			};

			/// <summary>
			/// VowpalWabbit wrapper
			/// </summary>
			public ref class VowpalWabbit : VowpalWabbitBase
			{
			public:
				VowpalWabbit(System::String^ pArgs);
				VowpalWabbit(VowpalWabbitModel^ model);

				uint32_t HashSpace(System::String^ s);
				uint32_t HashFeature(System::String^ s, unsigned long u);

				VowpalWabbitExample^ ReadExample(System::String^ line);
				VowpalWabbitExample^ ImportExample(cli::array<FeatureSpace^>^ featureSpaces);
				VowpalWabbitExample^ CreateEmptyExample();
			};
		}
	}
}