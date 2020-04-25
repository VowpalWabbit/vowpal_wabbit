#include "vowpalWabbit_learner_VWMultilabelsLearner.h"
#include "vw.h"
#include "jni_base_learner.h"

jobject multilabel_predictor(example *vec, JNIEnv *env)
{
  auto& labels = vec->pred.multilabels.label_v;
  size_t num_values = labels.size();
  jintArray j_labels = env->NewIntArray(num_values);
  env->SetIntArrayRegion(j_labels, 0, num_values, (int *)labels.begin());

  jclass clazz = env->FindClass("vowpalWabbit/responses/Multilabels");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "([I)V");
  return env->NewObject(clazz, constructor, j_labels);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWMultilabelsLearner_predict(
    JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_string, learn, vwPtr, multilabel_predictor);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWMultilabelsLearner_predictMultiline(
    JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_strings, learn, vwPtr, multilabel_predictor);
}
