#include "../../../../vowpalwabbit/vw.h"

#include "jni_base_learner.h"

void throw_java_exception(JNIEnv *env, const char* name, const char* msg)
{ jclass jc = env->FindClass(name);
  if (jc)
    env->ThrowNew(jc, msg);
}

void rethrow_cpp_exception_as_java_exception(JNIEnv *env)
{ try
  { throw;
  }
  catch(const std::bad_alloc& e)
  { throw_java_exception(env, "java/lang/OutOfMemoryError", e.what());
  }
  catch(const boost::program_options::error& e)
  { throw_java_exception(env, "java/lang/IllegalArgumentException", e.what());
  }
  catch(const std::exception& e)
  { throw_java_exception(env, "java/lang/Exception", e.what());
  }
  catch (...)
  { throw_java_exception(env, "java/lang/Error", "Unidentified exception => "
                         "rethrow_cpp_exception_as_java_exception "
                         "may require some completion...");
  }
}

example* read_example(JNIEnv *env, jstring example_string, vw* vwInstance)
{ const char* utf_string = env->GetStringUTFChars(example_string, NULL);
  example* ex = read_example(utf_string, vwInstance);

  env->ReleaseStringUTFChars(example_string, utf_string);
  env->DeleteLocalRef(example_string);

  return ex;
}

example* read_example(const char* example_string, vw* vwInstance)
{ return VW::read_example(*vwInstance, example_string);
}
