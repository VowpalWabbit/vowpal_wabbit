#include <jni.h>

jlong get_native_pointer(JNIEnv *env, jobject obj);

void throw_java_exception(JNIEnv *env, const char *name, const char *msg);

void rethrow_cpp_exception_as_java_exception(JNIEnv *env);