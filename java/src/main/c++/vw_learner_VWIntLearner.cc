#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWIntLearner.h"

JNIEXPORT jint JNICALL Java_vw_learner_VWIntLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr, jlong predictorPtr)
{ return base_predict<jint, jint (example*, JNIEnv*)>(env, example_string, learn, vwPtr, predictorPtr);
}

JNIEXPORT jint JNICALL Java_vw_learner_VWIntLearner_predictMultiline(JNIEnv *env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr, jlong predictorPtr)
{ return base_predict<jint, jint (example*, JNIEnv*)>(env, example_strings, learn, vwPtr, predictorPtr);
}
