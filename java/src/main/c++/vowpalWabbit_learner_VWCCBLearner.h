/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class vowpalWabbit_learner_VWActionScoresLearner */

#ifndef _Included_vowpalWabbit_learner_VWCCBLearner
#  define _Included_vowpalWabbit_learner_VWCCBLearner
#  ifdef __cplusplus
extern "C"
{
#  endif
  /*
   * Class:     vowpalWabbit_learner_VWCCBLearner
   * Method:    predict
   * Signature: (Ljava/lang/String;ZJ)LvowpalWabbit/responses/DecisionScores;
   */
  JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWCCBLearner_predict(JNIEnv *, jobject, jstring, jboolean, jlong);

  /*
   * Class:     vowpalWabbit_learner_VWCCBLearner
   * Method:    predictMultiline
   * Signature: ([Ljava/lang/String;ZJ)LvowpalWabbit/responses/DecisionScores;
   */
  JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWCCBLearner_predictMultiline(
      JNIEnv *, jobject, jobjectArray, jboolean, jlong);

#  ifdef __cplusplus
}
#  endif
#endif
