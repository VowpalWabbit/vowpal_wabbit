#include <exception>
#include "vw.h"
#include "util.h"

// assume that the passed in object has a field "nativePointer" of type long
jlong get_native_pointer(JNIEnv* env, jobject obj)
{
  jfieldID f = env->GetFieldID(env->GetObjectClass(obj), "nativePointer", "J");
  return env->GetLongField(obj, f);
}

void throw_java_exception(JNIEnv* env, const char* name, const char* msg)
{
  jclass jc = env->FindClass(name);
  if (jc)
    env->ThrowNew(jc, msg);
}

void rethrow_cpp_exception_as_java_exception(JNIEnv* env)
{
  try
  {
    throw;
  }
  catch (const std::bad_alloc& e)
  {
    throw_java_exception(env, "java/lang/OutOfMemoryError", e.what());
  }
  catch (const VW::vw_unrecognised_option_exception& e)
  {
    throw_java_exception(env, "java/lang/IllegalArgumentException", e.what());
  }
  catch (const std::exception& e)
  {
    throw_java_exception(env, "java/lang/Exception", e.what());
  }

  catch (...)
  {
    throw_java_exception(env, "java/lang/Error",
        "Unidentified exception => "
        "rethrow_cpp_exception_as_java_exception "
        "may require some completion...");
  }
}
