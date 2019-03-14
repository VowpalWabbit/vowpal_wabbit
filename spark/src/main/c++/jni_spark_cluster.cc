#include "jni_spark_vw_generated.h"
#include "util.h"
#include "spanning_tree.h"

JNIEXPORT jlong JNICALL Java_vowpalwabbit_spark_ClusterSpanningTree_create
  (JNIEnv *env, jclass)
{
  try
  { return (jlong)new VW::SpanningTree();
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_ClusterSpanningTree_delete
  (JNIEnv *env, jclass, jlong ptr)
{
  auto tree = (VW::SpanningTree*)ptr;
  try
  { 
      delete tree;
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_ClusterSpanningTree_start
  (JNIEnv *env, jclass, jlong ptr)
{
  auto tree = (VW::SpanningTree*)ptr;
  try
  { tree->Start(); 
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_vowpalwabbit_spark_ClusterSpanningTree_stop
  (JNIEnv *env, jclass, jlong ptr)
{
  auto tree = (VW::SpanningTree*)ptr;
  try
  { tree->Stop();
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

