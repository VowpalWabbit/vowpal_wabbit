#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWProbsLearner.h"

jfloatArray probs_predictor(example* vec, JNIEnv *env)
{ float* probs = vec->pred.probs;
  size_t num_values = 0;//probs.size();
  jfloatArray r = env->NewFloatArray(num_values);
  env->SetFloatArrayRegion(r, 0, num_values, probs);
  return r;
}

JNIEXPORT jfloatArray JNICALL Java_vw_learner_VWProbsLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{ return base_predict<jfloatArray>(env, example_string, learn, vwPtr, probs_predictor);
}

JNIEXPORT jfloatArray JNICALL Java_vw_learner_VWProbsLearner_predictMultiline(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{ return base_predict<jfloatArray>(env, example_strings, learn, vwPtr, probs_predictor);
}
