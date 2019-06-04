#include "jni_spark_vw_generated.h"
#include "vw.h"

// some JNI helper

// properly de-alloc resource also in case of exceptions
class StringGuard
{
  JNIEnv* _env;
  jstring _source;
  const char* _cstr;

 public:
  StringGuard(JNIEnv* env, jstring source);
  ~StringGuard();

  const char* c_str();
};

// properly de-alloc resource also in case of exceptions
class CriticalArrayGuard
{
  JNIEnv* _env;
  jarray _arr;
  void* _arr0;

 public:
  CriticalArrayGuard(JNIEnv* env, jarray arr);
  ~CriticalArrayGuard();

  void* data();
};

// bind VW instance and example together to reduce the number of variables passed around
class VowpalWabbitExampleWrapper
{
 public:
  vw* _all;
  example* _example;

  VowpalWabbitExampleWrapper(vw* all, example* example) : _all(all), _example(example) {}
};