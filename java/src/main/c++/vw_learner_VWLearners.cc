#include "../../../../vowpalwabbit/vw.h"
#include "jni_base_learner.h"
#include "vw_learner_VWLearners.h"

#define RETURN_TYPE "vw/learner/VWLearners$VWReturnType"
#define RETURN_TYPE_INSTANCE "L" RETURN_TYPE ";"

JNIEXPORT jlong JNICALL Java_vw_learner_VWLearners_initialize(JNIEnv *env, jobject obj, jstring command)
{ jlong vwPtr = 0;
  try
  { vw* vwInstance = VW::initialize(env->GetStringUTFChars(command, NULL));
    vwPtr = (jlong)vwInstance;
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
  return vwPtr;
}

JNIEXPORT void JNICALL Java_vw_learner_VWLearners_closeInstance(JNIEnv *env, jobject obj, jlong vwPtr)
{ try
  { VW::finish(*((vw*)vwPtr));
  }
  catch(...)
  { rethrow_cpp_exception_as_java_exception(env);
  }
}

bool args_contain(vw* vwInstance, const std::string& arg)
{ std::vector<std::string> args = vwInstance->args;
  return std::find(args.begin(), args.end(), arg) != args.end();
}

bool args_contains_cb_score(vw* vwInstance)
{ return args_contain(vwInstance, "--cb_explore_adf") ||
  args_contain(vwInstance, "--cb_explore") ||
  args_contain(vwInstance, "--rank_all");
}

JNIEXPORT jobject JNICALL Java_vw_learner_VWLearners_getReturnType(JNIEnv *env, jobject obj, jlong vwPtr)
{ jclass clVWReturnType = env->FindClass(RETURN_TYPE);
  jfieldID field;
  vw* vwInstance = (vw*)vwPtr;
  if (vwInstance->p->lp.parse_label == simple_label.parse_label)
  { if (vwInstance->lda > 0)
      field = env->GetStaticFieldID(clVWReturnType , "VWFloatArrayType", RETURN_TYPE_INSTANCE);
    else
      field = env->GetStaticFieldID(clVWReturnType , "VWFloatType", RETURN_TYPE_INSTANCE);
  }
  else if (vwInstance->p->lp.parse_label == MULTILABEL::multilabel.parse_label)
    field = env->GetStaticFieldID(clVWReturnType , "VWIntArrayType", RETURN_TYPE_INSTANCE);
  else if (args_contains_cb_score(vwInstance))
    field = env->GetStaticFieldID(clVWReturnType , "VWFloatArrayType", RETURN_TYPE_INSTANCE);
  else if (vwInstance->p->lp.parse_label == MULTICLASS::mc_label.parse_label ||
           vwInstance->p->lp.parse_label == CB::cb_label.parse_label ||
           vwInstance->p->lp.parse_label == CB_EVAL::cb_eval.parse_label ||
           vwInstance->p->lp.parse_label == COST_SENSITIVE::cs_label.parse_label)
    field = env->GetStaticFieldID(clVWReturnType , "VWIntType", RETURN_TYPE_INSTANCE);
  else
    field = env->GetStaticFieldID(clVWReturnType , "Unknown", RETURN_TYPE_INSTANCE);
  return env->GetStaticObjectField(clVWReturnType, field);
}

// Here are all the prediction functions we might want to use.  Add to this as more predictions are required.
jint multiclass_predictor(example* vec, JNIEnv *env) { return vec->pred.multiclass; }
jint cb_adf_best_predictor(example* vec, JNIEnv *env) { return vec->pred.a_s[0].action; }

jintArray multilabel_predictor(example* vec, JNIEnv *env)
{ v_array<uint32_t> predictions = vec->pred.multilabels.label_v;
  size_t num_predictions = predictions.size();
  jintArray r = env->NewIntArray(num_predictions);
  env->SetIntArrayRegion(r, 0, num_predictions, (int*)predictions.begin());
  return r;
}

jfloat scalar_predictor(example* vec, JNIEnv *env) { return vec->pred.scalar; }

jfloatArray lda_predictor(example* vec, JNIEnv *env)
{ v_array<float> predictions = vec->topic_predictions;
  size_t num_predictions = predictions.size();
  jfloatArray r = env->NewFloatArray(num_predictions);
  env->SetFloatArrayRegion(r, 0, num_predictions, predictions.begin());
  return r;
}

jfloatArray cb_adf_score_predictor(example* vec, JNIEnv *env)
{ ACTION_SCORE::action_scores actions = vec->pred.a_s;
  size_t num_actions = actions.size();
  jfloatArray r = env->NewFloatArray(num_actions);
  for (ACTION_SCORE::action_score* a = actions.begin(); a != actions.end(); ++a)
  { env->SetFloatArrayRegion(r, a->action, 1, &a->score);
  }
  return r;
}

JNIEXPORT jlong JNICALL Java_vw_learner_VWLearners_getPredictionFunction(
  JNIEnv *env,
  jobject obj,
  jlong vwPtr,
  jstring vw_return_type)
{ jlong functionPtr;
  vw* vwInstance = (vw*)vwPtr;
  const char* c_return_type = env->GetStringUTFChars(vw_return_type, NULL);

  if (strcmp(c_return_type, "VWIntType") == 0)
  { if (args_contain(vwInstance, "--cb_adf"))
      functionPtr = (jlong)cb_adf_best_predictor;
    else
      functionPtr = (jlong)multiclass_predictor;
  }
  else if (strcmp(c_return_type, "VWIntArrayType") == 0)
    functionPtr = (jlong)multilabel_predictor;
  else if (strcmp(c_return_type, "VWFloatType") == 0)
    functionPtr = (jlong)scalar_predictor;
  else if (strcmp(c_return_type, "VWFloatArrayType") == 0)
  { if (args_contains_cb_score(vwInstance))
      functionPtr = (jlong)cb_adf_score_predictor;
    else if (vwInstance->lda > 0)
      functionPtr = (jlong)lda_predictor;
    else
    { throw_java_exception(env, "java/lang/IllegalArgumentException", "Float array return types only supports LDA and CB.");
      functionPtr = 0;
    }
  }
  else
  { throw_java_exception(env, "java/lang/IllegalArgumentException", "Cannot create prediction function");
    functionPtr = 0;
  }

  env->ReleaseStringUTFChars(vw_return_type, c_return_type);
  env->DeleteLocalRef(vw_return_type);

  return functionPtr;
}
