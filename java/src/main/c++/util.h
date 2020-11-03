#include <jni.h>

// Java JNI exception check (if another JNI function is invoked it segfaults)
#define CHECK_JNI_EXCEPTION(ret) \
  if (env->ExceptionCheck())     \
    return ret;

jlong get_native_pointer(JNIEnv *env, jobject obj);

void throw_java_exception(JNIEnv *env, const char *name, const char *msg);

void rethrow_cpp_exception_as_java_exception(JNIEnv *env);