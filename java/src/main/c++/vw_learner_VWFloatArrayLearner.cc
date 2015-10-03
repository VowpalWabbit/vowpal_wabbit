#include "../../../../vowpalwabbit/parser.h"
#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWFloatArrayLearner.h"

JNIEXPORT jfloatArray JNICALL Java_vw_learner_VWFloatArrayLearner_predict(JNIEnv *env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr) {
    auto predictor = [env](example* vec) {
        v_array<float> predictions = vec->topic_predictions;
        size_t num_predictions = predictions.size();
        jfloatArray r = env->NewFloatArray(num_predictions);
        env->SetFloatArrayRegion(r, 0, num_predictions, predictions.begin);
        return r;
    };
    return base_predict<jfloatArray>(env, obj, example_string, learn, vwPtr, predictor);
}
