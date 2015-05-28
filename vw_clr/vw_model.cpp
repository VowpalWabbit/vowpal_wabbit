#include "vw_clr.h"

namespace Microsoft
{
	namespace Research
	{
		namespace MachineLearning
		{
			VowpalWabbitModel::VowpalWabbitModel(System::String^ pArgs)
				: VowpalWabbitBase(pArgs), m_instanceCount(0)
			{
			}

			VowpalWabbitModel::~VowpalWabbitModel()
			{
				this->!VowpalWabbitModel();
			}

			VowpalWabbitModel::!VowpalWabbitModel()
			{
				if (m_instanceCount <= 0)
				{
					this->InternalDispose();
				}
			}

			void VowpalWabbitModel::IncrementReference()
			{
				// thread-safe increase of model reference counter
				System::Threading::Interlocked::Increment(m_instanceCount);
			}

			void VowpalWabbitModel::DecrementReference()
			{
				// thread-safe decrease of model reference counter
				if (System::Threading::Interlocked::Decrement(m_instanceCount) <= 0)
				{
					this->InternalDispose();
				}
			}
		}
	}
}