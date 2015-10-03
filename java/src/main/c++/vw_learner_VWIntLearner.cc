#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWIntLearner.h"

JNIEXPORT jint JNICALL Java_vw_learner_VWIntLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
    auto predictor = [](example* vec) { return vec->pred.multiclass; };
    return base_predict<jint>(env, obj, example_string, learn, vwPtr, predictor);
}
