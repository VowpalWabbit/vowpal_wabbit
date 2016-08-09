#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWFloatArrayLearner.h"

JNIEXPORT jfloatArray JNICALL Java_vw_learner_VWFloatArrayLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr, jlong predictorPtr)
{ return base_predict<jfloatArray, jfloatArray (example*, JNIEnv*)>(env, obj, example_string, learn, vwPtr, predictorPtr);
}

JNIEXPORT jfloatArray JNICALL Java_vw_learner_VWFloatArrayLearner_predictMultiline(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr, jlong predictorPtr)
{ return base_predict<jfloatArray, jfloatArray (example*, JNIEnv*)>(env, obj, example_strings, learn, vwPtr, predictorPtr);
}
