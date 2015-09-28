#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_errors.h"
#include "vw_LDA.h"

JNIEXPORT jfloatArray JNICALL Java_vw_LDA_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
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

        VW::finish_example(*vwInstance, ec);
        env->ReleaseStringUTFChars(example_string, utf_string);
        env->DeleteLocalRef(example_string);
    } catch (...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return r;
}
