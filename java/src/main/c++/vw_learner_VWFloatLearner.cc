#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_errors.h"
#include "vw_learner_VWFloatLearner.h"
#include "vw_base_learner.h"

JNIEXPORT jfloat JNICALL Java_vw_learner_VWFloatLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
    auto predictor = [](example* vec) { return vec->pred.scalar; };
    return VW_Base<jfloat>(env, obj, example_string, learn, vwPtr, predictor);
}
