#include "jni_spark_vw_generated.h"
#include "util.h"
#include "vw/spanning_tree/spanning_tree.h"

JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_create(
    JNIEnv *env, jclass, jint port, jboolean quiet)
{
  try
  {
    return (jlong) new VW::SpanningTree(port, quiet);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return 0;
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_delete(JNIEnv *env, jobject clusterObj)
{
  auto tree = (VW::SpanningTree *)get_native_pointer(env, clusterObj);

  try
  {
    delete tree;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_start(JNIEnv *env, jobject clusterObj)
{
  auto tree = (VW::SpanningTree *)get_native_pointer(env, clusterObj);

  try
  {
    tree->Start();
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_stop(JNIEnv *env, jobject clusterObj)
{
  auto tree = (VW::SpanningTree *)get_native_pointer(env, clusterObj);

  try
  {
    tree->Stop();
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jint JNICALL Java_org_vowpalwabbit_spark_ClusterSpanningTree_getPort(JNIEnv *env, jobject clusterObj)
{
  auto tree = (VW::SpanningTree *)get_native_pointer(env, clusterObj);

  try
  {
    return tree->BoundPort();
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return 0;
  }
}