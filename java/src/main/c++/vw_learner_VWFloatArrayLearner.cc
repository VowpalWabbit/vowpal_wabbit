#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWFloatArrayLearner.h"

jfloatArray floatArrayPredictor(example* vec, JNIEnv *env)
{ v_array<float> predictions = vec->topic_predictions;
  size_t num_predictions = predictions.size();
  jfloatArray r = env->NewFloatArray(num_predictions);
  env->SetFloatArrayRegion(r, 0, num_predictions, predictions.begin());
  return r;
}

JNIEXPORT jfloatArray JNICALL Java_vw_learner_VWFloatArrayLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{ return base_predict<jfloatArray>(env, obj, example_string, learn, vwPtr, floatArrayPredictor);
}

JNIEXPORT jfloatArray JNICALL Java_vw_learner_VWFloatArrayLearner_predictMultiline(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{ return base_predict<jfloatArray>(env, obj, example_strings, learn, vwPtr, floatArrayPredictor);
}
