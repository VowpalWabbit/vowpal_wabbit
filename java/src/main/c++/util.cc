#include <exception>
#include "vw.h"
#include "util.h"

// assume that the passed in object has a field "nativePointer" of type long
jlong get_native_pointer(JNIEnv *env, jobject obj)
{ jfieldID f = env->GetFieldID(env->GetObjectClass(obj), "nativePointer", "J");
  return env->GetLongField(obj, f);
}