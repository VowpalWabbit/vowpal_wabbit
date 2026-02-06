#include "vowpalWabbit_learner_VWActiveMulticlassLearner.h"

#include "jni_base_learner.h"
#include "vw/core/active_multiclass_prediction.h"
#include "vw/core/vw.h"

jobject active_multiclass_prediction(example* vec, JNIEnv* env)
{
  auto& am = vec->pred.active_multiclass;

  // Create int[] for more_info_required_for_classes
  size_t num_classes = am.more_info_required_for_classes.size();
  jintArray j_classes = env->NewIntArray(num_classes);
  if (num_classes > 0)
  {
    env->SetIntArrayRegion(j_classes, 0, num_classes, (jint*)am.more_info_required_for_classes.begin());
  }

  jclass clazz = env->FindClass("vowpalWabbit/responses/ActiveMulticlass");
  jmethodID constructor = env->GetMethodID(clazz, "<init>", "(I[I)V");
  return env->NewObject(clazz, constructor, (jint)am.predicted_class, j_classes);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWActiveMulticlassLearner_predict(
    JNIEnv* env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_string, learn, vwPtr, active_multiclass_prediction);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWActiveMulticlassLearner_predictMultiline(
    JNIEnv* env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_strings, learn, vwPtr, active_multiclass_prediction);
}
