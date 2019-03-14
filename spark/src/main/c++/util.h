#include <jni.h>

void throw_java_exception(JNIEnv *env, const char* name, const char* msg);

void rethrow_cpp_exception_as_java_exception(JNIEnv *env);