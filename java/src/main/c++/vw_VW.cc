#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_VW.h"

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

JNIEXPORT jstring JNICALL Java_vw_VW_version(JNIEnv *env, jobject obj) {
    return env->NewStringUTF(PACKAGE_VERSION);
}

JNIEXPORT jlong JNICALL Java_vw_VW_initialize(JNIEnv *env, jobject obj, jstring command) {
    jlong vwPtr = 0;
    try {
        vw* vwInstance = VW::initialize(env->GetStringUTFChars(command, NULL));
        vwPtr = (jlong)vwInstance;
        if (vwInstance->multilabel_prediction) {
            Java_vw_VW_closeInstance(env, obj, vwPtr);
            vwPtr = 0;
            throw_java_exception(env, "vw/exception/IllegalVWInput", "VW JNI layer only supports simple and multiclass predictions");
        }
    }
    catch(...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return vwPtr;
}

JNIEXPORT jfloat JNICALL Java_vw_VW_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
    float prediction = 0.0f;
    try {
        vw* vwInstance = (vw*)vwPtr;
        const char *utf_string = env->GetStringUTFChars(example_string, NULL);
        example *vec2 = VW::read_example(*vwInstance, utf_string);

        if (learn)
            vwInstance->l->learn(*vec2);
        else
            vwInstance->l->predict(*vec2);

        if (vwInstance->p->lp.parse_label == simple_label.parse_label)
            prediction = vec2->pred.scalar;
        else
            prediction = vec2->pred.multiclass;

        VW::finish_example(*vwInstance, vec2);
        env->ReleaseStringUTFChars(example_string, utf_string);
        env->DeleteLocalRef(example_string);
    }
    catch (...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return prediction;
}

JNIEXPORT jfloatArray JNICALL Java_vw_VW_multipredict(JNIEnv *env, jobject obj, jstring example_string, jlong vwPtr) {
  jfloatArray r = env->NewFloatArray(0);
  try {
    vw* vwInstance = (vw*)vwPtr;
    const char *utf_string = env->GetStringUTFChars(example_string, NULL);
    example *ec = VW::read_example(*vwInstance, utf_string);

    vwInstance->l->predict(*ec);

    r = env->NewFloatArray(vwInstance->lda);
    float *predictions = ec->topic_predictions.begin;
    jfloat *weights = new jfloat[vwInstance->lda];
    for (size_t k = 0; k < vwInstance->lda; k++) {
      weights[k] = predictions[k];
    }
    env->SetFloatArrayRegion(r, 0, vwInstance->lda, weights);
    delete weights;

    ec->in_use = true; // temporary workaround for issue #800
    VW::finish_example(*vwInstance, ec);
    env->ReleaseStringUTFChars(example_string, utf_string);
    env->DeleteLocalRef(example_string);
  } catch (...) {
    rethrow_cpp_exception_as_java_exception(env);
  }
  return r;
}

JNIEXPORT void JNICALL Java_vw_VW_closeInstance(JNIEnv *env, jobject obj, jlong vwPtr) {
    try {
        VW::finish(*((vw*)vwPtr));
    }
    catch(...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
}
