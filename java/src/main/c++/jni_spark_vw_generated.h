/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class org_vowpalwabbit_spark_VowpalWabbitNative */

#ifndef _Included_org_vowpalwabbit_spark_VowpalWabbitNative
#define _Included_org_vowpalwabbit_spark_VowpalWabbitNative
#ifdef __cplusplus
extern "C"
{
#endif
  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    initialize
   * Signature: (Ljava/lang/String;)J
   */
  JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_initialize(JNIEnv *, jclass, jstring);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    initializeFromModel
   * Signature: (Ljava/lang/String;[B)J
   */
  JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_initializeFromModel(
      JNIEnv *, jclass, jstring, jbyteArray);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    performRemainingPasses
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_performRemainingPasses(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    getModel
   * Signature: ()[B
   */
  JNIEXPORT jbyteArray JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getModel(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    getArguments
   * Signature: ()Lorg/vowpalwabbit/bare/VowpalWabbitArguments;
   */
  JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getArguments(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    getPerformanceStatistic
   * Signature: ()Ljava.lang.String;
   */
  JNIEXPORT jdouble JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getPerformanceStatistic(
      JNIEnv *, jobject, jstring);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    endPass
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_endPass(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    finish
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_finish(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitNative
   * Method:    hash
   * Signature: ([BIII)I
   */
  JNIEXPORT jint JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_hash(
      JNIEnv *, jclass, jbyteArray, jint, jint, jint);

#ifdef __cplusplus
}
#endif
#endif
/* Header for class org_vowpalwabbit_spark_VowpalWabbitExample */

#ifndef _Included_org_vowpalwabbit_spark_VowpalWabbitExample
#define _Included_org_vowpalwabbit_spark_VowpalWabbitExample
#ifdef __cplusplus
extern "C"
{
#endif
  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    initialize
   * Signature: (JZ)J
   */
  JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_initialize(JNIEnv *, jclass, jlong, jboolean);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    finish
   * Signature: ()J
   */
  JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_finish(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    clear
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_clear(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    addToNamespaceDense
   * Signature: (CI[D)V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_addToNamespaceDense(
      JNIEnv *, jobject, jchar, jint, jdoubleArray);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    addToNamespaceSparse
   * Signature: (C[I[D)V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_addToNamespaceSparse(
      JNIEnv *, jobject, jchar, jintArray, jdoubleArray);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    setLabel
   * Signature: (FF)V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setLabel(JNIEnv *, jobject, jfloat, jfloat);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    learn
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_learn(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    getPrediction
   * Signature: ()Ljava/lang/Object;
   */
  JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_getPrediction(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_VowpalWabbitExample
   * Method:    predict
   * Signature: ()Ljava/lang/Object;
   */
  JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_predict(JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
/* Header for class org_vowpalwabbit_spark_ClusterSpanningTree */

#ifndef _Included_org_vowpalwabbit_spark_ClusterSpanningTree
#define _Included_org_vowpalwabbit_spark_ClusterSpanningTree
#ifdef __cplusplus
extern "C"
{
#endif
  /*
   * Class:     org_vowpalwabbit_spark_ClusterSpanningTree
   * Method:    create
   * Signature: (IZ)J
   */
  JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_create(JNIEnv *, jclass, jint, jboolean);

  /*
   * Class:     org_vowpalwabbit_spark_ClusterSpanningTree
   * Method:    delete
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_delete(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_ClusterSpanningTree
   * Method:    start
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_start(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_ClusterSpanningTree
   * Method:    stop
   * Signature: ()V
   */
  JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_stop(JNIEnv *, jobject);

  /*
   * Class:     org_vowpalwabbit_spark_ClusterSpanningTree
   * Method:    getPort
   * Signature: ()I
   */
  JNIEXPORT jint JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_getPort(JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
