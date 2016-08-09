#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWFloatLearner.h"

JNIEXPORT jfloat JNICALL Java_vw_learner_VWFloatLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr, jlong predictorPtr)
{ return base_predict<jfloat, jfloat (example*, JNIEnv*)>(env, example_string, learn, vwPtr, predictorPtr);
}

JNIEXPORT jfloat JNICALL Java_vw_learner_VWFloatLearner_predictMultiline(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr, jlong predictorPtr)
{ return base_predict<jfloat, jfloat (example*, JNIEnv*)>(env, example_strings, learn, vwPtr, predictorPtr);
}
