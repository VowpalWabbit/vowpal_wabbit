#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_ScalarLearner.h"

jfloat scalar_predictor(example* vec, JNIEnv *env) { return vec->pred.scalar; }

JNIEXPORT jfloat JNICALL Java_vw_learner_ScalarLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{ return base_predict<jfloat>(env, example_string, learn, vwPtr, scalar_predictor);
}

JNIEXPORT jfloat JNICALL Java_vw_learner_ScalarLearner_predictMultiline(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{ return base_predict<jfloat>(env, example_strings, learn, vwPtr, scalar_predictor);
}
