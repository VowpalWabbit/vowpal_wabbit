#include "vowpalWabbit_learner_VWScalarLearner.h"
#include "vw.h"
#include "jni_base_learner.h"

jfloat scalar_predictor(example *vec, JNIEnv *env) { return vec->pred.scalar; }

JNIEXPORT jfloat JNICALL Java_vowpalWabbit_learner_VWScalarLearner_predict(
    JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{
  return base_predict<jfloat>(env, example_string, learn, vwPtr, scalar_predictor);
}

JNIEXPORT jfloat JNICALL Java_vowpalWabbit_learner_VWScalarLearner_predictMultiline(
    JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{
  return base_predict<jfloat>(env, example_strings, learn, vwPtr, scalar_predictor);
}