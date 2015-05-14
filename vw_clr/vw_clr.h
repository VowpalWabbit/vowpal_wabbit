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
			public value struct FEATURE_SPACE
			{
			public:
				Byte name;
				IntPtr features;     
				int len;
			};

			[StructLayout(LayoutKind::Sequential)]
			public value struct FEATURE
			{
			public:
				float x;
				UInt32 weight_index;
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

				bool IsEmpty();

				void AddLabel(string label)
				{

				}

				float Learn();

				float Predict();

				// void AddLabel(float label = float.MaxValue, float weight = 1, float initial = 0);
				void AddLabel(float label, float weight, float base)
				{
					VW::add_label(m_example, label, weight, base);
				}
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

				VowpalWabbitExample^ ReadExample(System::String^ line);

				VowpalWabbitExample^ ImportExample(cli::array<FEATURE_SPACE>^ featureSpace);

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

