#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_errors.h"
#include "vw_learner_VWIntLearner.h"
#include "vw_base_learner.h"

JNIEXPORT jint JNICALL Java_vw_learner_VWIntLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
    auto predictor = [](example* vec) { return vec->pred.multiclass; };
    return VW_Base<jint>(env, obj, example_string, learn, vwPtr, predictor);
}
