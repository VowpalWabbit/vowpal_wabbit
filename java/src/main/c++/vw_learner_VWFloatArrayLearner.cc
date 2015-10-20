#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWFloatArrayLearner.h"

jfloatArray floatArrayPredictor(vw* vwInstance, example* vec, JNIEnv *env)
{ v_array<float> predictions = vec->topic_predictions;
  size_t num_predictions = predictions.size();
  jfloatArray r = env->NewFloatArray(num_predictions);
  env->SetFloatArrayRegion(r, 0, num_predictions, predictions.begin);
  // The LDA algorithm calls finish_example because it's a minibatch algorithm.
  // All other learner types will require finish_example to be called.
  if (!vwInstance->lda)
      VW::finish_example(*vwInstance, vec);
  return r;
}

JNIEXPORT jfloatArray JNICALL Java_vw_learner_VWFloatArrayLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{ return base_predict<jfloatArray>(env, obj, example_string, learn, vwPtr, floatArrayPredictor);
}
