#include "vowpalWabbit_learner_VWNoPredLearner.h"

#include "jni_base_learner.h"
#include "vw/core/vw.h"

jobject nopred_prediction(example* vec, JNIEnv* env) { return nullptr; }

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWNoPredLearner_predict(
    JNIEnv* env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_string, learn, vwPtr, nopred_prediction);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWNoPredLearner_predictMultiline(
    JNIEnv* env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_strings, learn, vwPtr, nopred_prediction);
}
