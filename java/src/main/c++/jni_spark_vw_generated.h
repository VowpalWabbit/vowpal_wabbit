/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_vowpalwabbit_spark_VowpalWabbitNative */

#ifndef _Included_org_vowpalwabbit_spark_VowpalWabbitNative
#  define _Included_org_vowpalwabbit_spark_VowpalWabbitNative
#  ifdef __cplusplus
extern "C"
{
#  endif
  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    initialize
   * Signature: (Ljava/lang/String;)J
   */
  JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_initialize(JNIEnv*, jclass, jstring);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    initializeFromModel
   * Signature: (Ljava/lang/String;[B)J
   */
  JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_initializeFromModel(
      JNIEnv*, jclass, jstring, jbyteArray);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    learn
   * Signature: ([)J
   */
  JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_learn(JNIEnv*, jobject, jobjectArray);

  JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_learnFromString(JNIEnv *, jobject, jstring);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    predict
   * Signature: ([])J
   */
  JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_predict(JNIEnv*, jobject, jobjectArray);

  JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_predictFromString(JNIEnv *, jobject, jstring);


  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    performRemainingPasses
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_performRemainingPasses(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    getModel
   * Signature: ()[B
   */
  JNIEXPORT jbyteArray JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getModel(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    getArguments
   * Signature: ()Lorg/vowpalwabbit/spark/VowpalWabbitArguments;
   */
  JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getArguments(JNIEnv*, jobject);

  JNIEXPORT jstring JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getOutputPredictionType(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    getPerformanceStatistic
   * Signature: ()Lorg/vowpalwabbit/bare/VowpalWabbitPerformanceStatistics;
   */
  JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getPerformanceStatistics(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    endPass
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_endPass(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    finish
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_finish(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    hash
   * Signature: ([BIII)I
   */
  JNIEXPORT jint JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_hash(
      JNIEnv*, jclass, jbyteArray, jint, jint, jint);

#  ifdef __cplusplus
}
#  endif
#endif
/* Header for class org_vowpalwabbit_spark_VowpalWabbitExample */

#ifndef _Included_org_vowpalwabbit_spark_VowpalWabbitExample
#  define _Included_org_vowpalwabbit_spark_VowpalWabbitExample
#  ifdef __cplusplus
extern "C"
{
#  endif
  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    initialize
   * Signature: (JZ)J
   */
  JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_initialize(JNIEnv*, jclass, jlong, jboolean);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    finish
   * Signature: ()J
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_finish(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    clear
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_clear(JNIEnv*, jobject);

  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setDefaultLabel(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    addToNamespaceDense
   * Signature: (CI[D)V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_addToNamespaceDense(
      JNIEnv*, jobject, jchar, jint, jdoubleArray);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    addToNamespaceSparse
   * Signature: (C[I[D)V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_addToNamespaceSparse(
      JNIEnv*, jobject, jchar, jintArray, jdoubleArray);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    setLabel
   * Signature: (FF)V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setLabel(JNIEnv*, jobject, jfloat, jfloat);

  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setMulticlassLabel(
      JNIEnv*, jobject, jfloat, jint);

  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setCostSensitiveLabels(
      JNIEnv*, jobject, jfloatArray, jintArray);

  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setContextualBanditContinuousLabel(
      JNIEnv*, jobject, jfloatArray, jfloatArray, jfloatArray);

  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setMultiLabels(JNIEnv*, jobject, jintArray);

  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setContextualBanditLabel(
      JNIEnv*, jobject, jint, jdouble, jdouble);

  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSharedLabel(JNIEnv*, jobject);

  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSlatesSharedLabel(JNIEnv*, jobject, jfloat);

  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSlatesActionLabel(JNIEnv*, jobject, jint);

  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSlatesSlotLabel(
      JNIEnv*, jobject, jintArray, jfloatArray);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    learn
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_learn(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    getPrediction
   * Signature: ()Ljava/lang/Object;
   */
  JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_getPrediction(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    toString
   * Signature: ()Ljava/lang/String;
   */
  JNIEXPORT jstring JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_toString(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    predict
   * Signature: ()Ljava/lang/Object;
   */
  JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_predict(JNIEnv*, jobject);

#  ifdef __cplusplus
}
#  endif
#endif
/* Header for class org_vowpalwabbit_spark_ClusterSpanningTree */

#ifndef _Included_org_vowpalwabbit_spark_ClusterSpanningTree
#  define _Included_org_vowpalwabbit_spark_ClusterSpanningTree
#  ifdef __cplusplus
extern "C"
{
#  endif
  /*
   * Class:     org_vowpalwabbit_spark_ClusterSpanningTree
   * Method:    create
   * Signature: (IZ)J
   */
  JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_create(JNIEnv*, jclass, jint, jboolean);

  /*
   * Class:     org_vowpalwabbit_spark_ClusterSpanningTree
   * Method:    delete
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_delete(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_ClusterSpanningTree
   * Method:    start
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_start(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_ClusterSpanningTree
   * Method:    stop
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_stop(JNIEnv*, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_ClusterSpanningTree
   * Method:    getPort
   * Signature: ()I
   */
  JNIEXPORT jint JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_getPort(JNIEnv*, jobject);

#  ifdef __cplusplus
}
#  endif
#endif
