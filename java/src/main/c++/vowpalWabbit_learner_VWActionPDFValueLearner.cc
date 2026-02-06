#include "vowpalWabbit_learner_VWActionPDFValueLearner.h"

#include "jni_base_learner.h"
#include "vw/core/vw.h"

jobject pdf_value_prediction(example* ex, JNIEnv* env)
{
  jclass predClass = env->FindClass("vowpalWabbit/responses/PDFValue");
  jmethodID ctr = env->GetMethodID(predClass, "<init>", "(FF)V");
  return env->NewObject(predClass, ctr, ex->pred.pdf_value.action, ex->pred.pdf_value.pdf_value);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWActionPDFValueLearner_predict(
    JNIEnv* env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_string, learn, vwPtr, pdf_value_prediction);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWActionPDFValueLearner_predictMultiline(
    JNIEnv* env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_strings, learn, vwPtr, pdf_value_prediction);
}
