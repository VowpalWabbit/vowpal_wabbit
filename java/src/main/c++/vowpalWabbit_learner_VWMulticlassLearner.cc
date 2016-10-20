#include "vowpalWabbit_learner_VWMulticlassLearner.h"
#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"

jint multiclass_predictor(example* vec, JNIEnv *env){ return vec->pred.multiclass;}

jdoubleArray multiclass_raw_predictor(example* vec, JNIEnv *env){
  size_t num_values = vec->l.cs.costs.size();
  jdoubleArray j_labels = env->NewDoubleArray(num_values);
  for(int i=0;i<num_values;i++) {
    jdouble f[] = {vec->l.cs.costs[i].partial_prediction};
    env->SetDoubleArrayRegion(j_labels, i, 1, (double*)f);
  }
  return j_labels;
}

JNIEXPORT jint JNICALL Java_vowpalWabbit_learner_VWMulticlassLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{ return base_predict<jint>(env, example_string, learn, vwPtr, multiclass_predictor);
}

JNIEXPORT jint JNICALL Java_vowpalWabbit_learner_VWMulticlassLearner_predictMultiline(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{ return base_predict<jint>(env, example_strings, learn, vwPtr, multiclass_predictor);
}

JNIEXPORT jdoubleArray JNICALL Java_vowpalWabbit_learner_VWMulticlassLearner_rawPredict
  (JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{
  return base_predict<jdoubleArray>(env, example_string, learn, vwPtr, multiclass_raw_predictor);
}
