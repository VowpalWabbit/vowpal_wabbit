#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_errors.h"
#include "vw_learner_VWFloatArrayLearner.h"

JNIEXPORT jfloatArray JNICALL Java_vw_learner_VWFloatArrayLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
    jfloatArray r = env->NewFloatArray(0);
    try {
        vw* vwInstance = (vw*)vwPtr;
        const char *utf_string = env->GetStringUTFChars(example_string, NULL);
        example *vec = VW::read_example(*vwInstance, utf_string);

        if (learn)
            vwInstance->l->learn(*vec);
        else
            vwInstance->l->predict(*vec);

        v_array<float> predictions = vec->topic_predictions;
        size_t num_predictions = predictions.size();
        r = env->NewFloatArray(num_predictions);
        env->SetFloatArrayRegion(r, 0, num_predictions, predictions.begin);

        VW::finish_example(*vwInstance, vec);
        env->ReleaseStringUTFChars(example_string, utf_string);
        env->DeleteLocalRef(example_string);
    } catch (...) {
        rethrow_cpp_exception_as_java_exception(env);
    }
    return r;
}
