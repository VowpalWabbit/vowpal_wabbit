#include "vowpalWabbit_learner_VWScalarsLearner.h"
#include "vw.h"
#include "jni_base_learner.h"

jfloatArray scalars_predictor(example *vec, JNIEnv *env)
{
  auto& scalars = vec->pred.scalars;
  size_t num_values = scalars.size();
  jfloatArray r = env->NewFloatArray(num_values);
  env->SetFloatArrayRegion(r, 0, num_values, (float *)scalars.begin());
  return r;
}

JNIEXPORT jfloatArray JNICALL Java_vowpalWabbit_learner_VWScalarsLearner_predict(
    JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{
  return base_predict<jfloatArray>(env, example_string, learn, vwPtr, scalars_predictor);
}

JNIEXPORT jfloatArray JNICALL Java_vowpalWabbit_learner_VWScalarsLearner_predictMultiline(
    JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{
  return base_predict<jfloatArray>(env, example_strings, learn, vwPtr, scalars_predictor);
}
