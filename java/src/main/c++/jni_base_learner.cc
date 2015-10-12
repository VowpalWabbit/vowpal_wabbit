#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"

#include "jni_base_learner.h"

void throw_java_exception(JNIEnv *env, const char* name, const char* msg) {
     jclass jc = env->FindClass(name);
     if (jc)
        env->ThrowNew(jc, msg);
}

void rethrow_cpp_exception_as_java_exception(JNIEnv *env) {
    try {
        throw;
    }
    catch(const std::bad_alloc& e) {
        throw_java_exception(env, "java/lang/OutOfMemoryError", e.what());
    }
    catch(const boost::program_options::error& e) {
        throw_java_exception(env, "java/lang/IllegalArgumentException", e.what());
    }
    catch(const std::exception& e) {
        throw_java_exception(env, "java/lang/Exception", e.what());
    }
    catch (...) {
        throw_java_exception(env, "java/lang/Error", "Unidentified exception => "
                                   "rethrow_cpp_exception_as_java_exception "
                                   "may require some completion...");
    }
}
