#include "vowpalWabbit_learner_VWMulticlassLearner.h"
#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"

jint multiclass_predictor(example* vec, JNIEnv *env) { return vec->pred.multiclass; }

JNIEXPORT jint JNICALL Java_vowpalWabbit_learner_VWMulticlassLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{ return base_predict<jint>(env, example_string, learn, vwPtr, multiclass_predictor);
}

JNIEXPORT jint JNICALL Java_vowpalWabbit_learner_VWMulticlassLearner_predictMultiline(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{ return base_predict<jint>(env, example_strings, learn, vwPtr, multiclass_predictor);
}
