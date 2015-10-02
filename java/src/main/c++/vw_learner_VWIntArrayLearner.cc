#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_errors.h"
#include "vw_learner_VWIntArrayLearner.h"

JNIEXPORT jintArray JNICALL Java_vw_learner_VWIntArrayLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
    jintArray r = env->NewIntArray(0);
    try {
        vw* vwInstance = (vw*)vwPtr;
        const char *utf_string = env->GetStringUTFChars(example_string, NULL);
        example *vec = VW::read_example(*vwInstance, utf_string);

        if (learn)
            vwInstance->l->learn(*vec);
        else
            vwInstance->l->predict(*vec);
            
        v_array<uint32_t> predictions = vec->pred.multilabels.label_v;
        size_t num_predictions = predictions.size();
        r = env->NewIntArray(num_predictions);

        env->SetIntArrayRegion(r, 0, num_predictions, (int*)predictions.begin);

        VW::finish_example(*vwInstance, vec);
        env->ReleaseStringUTFChars(example_string, utf_string);
        env->DeleteLocalRef(example_string);
    } catch (...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return r;
}
