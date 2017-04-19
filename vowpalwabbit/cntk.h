#pragma once



// trainer->Forward (Evaluate more params)
#include "parse_regressor.h"
#include "constant.h"
#include "interactions.h"
#include "array_parameters.h"

namespace VW_CNTK {
	LEARNER::base_learner* setup(vw& all);

	//struct cntk;
}

//void setup()
//{
//
//}
//
//void save()
//{
//}
//
//void load(void* ctx)
//{
//	// 
//
//
//
//	// predicitionError can be emitted
//}
//
//
//float predict(example* ex)
//{
//	// auto classifierOutput = LSTMSequenceClassiferNet(features, numOutputClasses, embeddingDim, hiddenDim, cellDim, device, L"classifierOutput");
//
//
//	// forward/evaluate
//}
//
//float learn(example * ex)
//{
//	// bakc and forth (=TrainMinibatch())
//}