#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "vw_errors.h"
#include "vw_learner_VWIntArrayLearner.h"
#include "vw_base_learner.h"

JNIEXPORT jintArray JNICALL Java_vw_learner_VWIntArrayLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
    auto predictor = [env](example* vec) {
        v_array<uint32_t> predictions = vec->pred.multilabels.label_v;
        size_t num_predictions = predictions.size();
        jintArray r = env->NewIntArray(num_predictions);
        env->SetIntArrayRegion(r, 0, num_predictions, (int*)predictions.begin);
        return r;
    };
    return VW_Base<jintArray>(env, obj, example_string, learn, vwPtr, predictor);
}
