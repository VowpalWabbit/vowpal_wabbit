#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWIntArrayLearner.h"

jintArray intArrayPredictor(example* vec, JNIEnv *env)
{ v_array<uint32_t> predictions = vec->pred.multilabels.label_v;
  size_t num_predictions = predictions.size();
  jintArray r = env->NewIntArray(num_predictions);
  env->SetIntArrayRegion(r, 0, num_predictions, (int*)predictions.begin);
  return r;
}

JNIEXPORT jintArray JNICALL Java_vw_learner_VWIntArrayLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{ return base_predict<jintArray>(env, obj, example_string, learn, vwPtr, intArrayPredictor);
}
