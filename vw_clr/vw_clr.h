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

			public ref class VowpalWabbitExample
			{
			private:
				vw* const m_vw;
				example* const m_example;
				bool m_isEmpty;

			protected:
				bool m_isDisposed;
				!VowpalWabbitExample();

			public:
				VowpalWabbitExample(vw* vw, example* example);

				VowpalWabbitExample(vw* vw, example* example, bool isEmpty);

				~VowpalWabbitExample();

				property bool IsEmpty
				{
					bool get();
				}

				property float CostSensitivePrediction
				{
					float get();
				}
					
				property cli::array<int>^ MultilabelPredictions
				{
					cli::array<int>^ get();
				}

				void AddLabel(System::String^ label);

				void AddLabel(float label);
				
				void AddLabel(float label, float weight);

				void AddLabel(float label, float weight, float base);

				float Learn();

				float Predict();
			};

			public ref class VowpalWabbit
			{
			private:
				vw* m_vw;
				bool m_isDisposed;

			protected:
				!VowpalWabbit();

			public:
				VowpalWabbit(System::String^ pArgs);
				~VowpalWabbit();
				
				uint32_t HashSpace(System::String^ s);
				uint32_t HashFeature(System::String^ s, unsigned long u);

				VowpalWabbitExample^ ReadExample(System::String^ line);
				VowpalWabbitExample^ ImportExample(cli::array<FeatureSpace^>^ featureSpaces);
				VowpalWabbitExample^ CreateEmptyExample();

				//void Foo()
				//{
				//	VW::add_label((example*)nullptr, 0);

				//	// (VW::primitive_feature_space*)

				//	cli::array<int>^ test = gcnew cli::array<int>(5);
				//	
				//	pin_ptr<int> native_test = &test[0];

				//	(int*)native_test
				//}

				// TODO: Add your methods for this class here.
			};

		}
	}
}

