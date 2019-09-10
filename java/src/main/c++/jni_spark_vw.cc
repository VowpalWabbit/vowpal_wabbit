#include "jni_spark_vw.h"
#include "vw_exception.h"
#include "best_constant.h"
#include "vector_io_buf.h"
#include "util.h"
#include "options_serializer_boost_po.h"
#include <algorithm>

// Guards
StringGuard::StringGuard(JNIEnv* env, jstring source) : _env(env), _source(source), _cstr(nullptr)
{
  _cstr = _env->GetStringUTFChars(source, 0);
}

StringGuard::~StringGuard()
{
  if (_cstr)
  {
    _env->ReleaseStringUTFChars(_source, _cstr);
    _env->DeleteLocalRef(_source);
  }
}

const char* StringGuard::c_str() { return _cstr; }

CriticalArrayGuard::CriticalArrayGuard(JNIEnv* env, jarray arr) : _env(env), _arr(arr), _arr0(nullptr)
{
  _arr0 = env->GetPrimitiveArrayCritical(arr, nullptr);
}

CriticalArrayGuard::~CriticalArrayGuard()
{
  if (_arr0)
  {
    _env->ReleasePrimitiveArrayCritical(_arr, _arr0, JNI_ABORT);
  }
}

void* CriticalArrayGuard::data() { return _arr0; }

// VW
JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_initialize(JNIEnv* env, jclass, jstring args)
{
  StringGuard g_args(env, args);

  try
  {
    return (jlong)VW::initialize(g_args.c_str());
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_initializeFromModel(
    JNIEnv* env, jclass, jstring args, jbyteArray model)
{
  StringGuard g_args(env, args);
  CriticalArrayGuard modelGuard(env, model);

  try
  {
    int size = env->GetArrayLength(model);
    char* model0 = (char*)modelGuard.data();

    // wrap the model inside a vector
    vector_io_buf buffer(model0, size);

    return (jlong)VW::initialize(g_args.c_str(), &buffer);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_performRemainingPasses(JNIEnv* env, jobject vwObj)
{
  auto all = (vw*)get_native_pointer(env, vwObj);

  try
  {
    if (all->numpasses > 1)
    {
      all->do_reset_source = true;
      VW::start_parser(*all);
      LEARNER::generic_driver(*all);
      VW::end_parser(*all);
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jbyteArray JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getModel(JNIEnv* env, jobject vwObj)
{
  auto all = (vw*)get_native_pointer(env, vwObj);

  try
  {  // save in stl::vector
    vector_io_buf buffer;
    VW::save_predictor(*all, buffer);

    // copy to Java
    jbyteArray ret = env->NewByteArray(buffer._buffer.size());
    env->SetByteArrayRegion(ret, 0, buffer._buffer.size(), (const jbyte*)&buffer._buffer[0]);

    return ret;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getArguments(JNIEnv* env, jobject vwObj)
{
  auto all = (vw*)get_native_pointer(env, vwObj);

  // serialize the command line
  VW::config::options_serializer_boost_po serializer;
  for (auto const& option : all->options->get_all_options())
  {
    if (all->options->was_supplied(option->m_name))
    {
      serializer.add(*option);
    }
  }

  // move it to Java
  // Note: don't keep serializer.str().c_str() around in some variable. it get's deleted after str() is de-allocated
  jstring args = env->NewStringUTF(serializer.str().c_str());

  jclass clazz = env->FindClass("org/vowpalwabbit/spark/VowpalWabbitArguments");
  jmethodID ctor = env->GetMethodID(clazz, "<init>", "(IILjava/lang/String;DD)V");

  return env->NewObject(clazz, ctor, all->num_bits, all->hash_seed, args, all->eta, all->power_t);
}

JNIEXPORT jdouble JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getPerformanceStatistic(
    JNIEnv* env, jobject vwObj, jstring key)
{
  StringGuard g_key(env, key);

  auto all = (vw*)get_native_pointer(env, vwObj);

  if (!strcmp(g_key.c_str(), "numberOfExamplesPerPass"))
  {
    if (all->current_pass == 0)
      return all->sd->example_number;
    else
      return all->sd->example_number / all->current_pass;
  }

  if (!strcmp(g_key.c_str(), "weightedExampleSum"))
  {
    return all->sd->weighted_examples();
  }

  if (!strcmp(g_key.c_str(), "weightedLabelSum"))
  {
    return all->sd->weighted_labels;
  }

  if (!strcmp(g_key.c_str(), "averageLoss"))
  {
    if (all->holdout_set_off)
      if (all->sd->weighted_labeled_examples > 0)
        return all->sd->sum_loss / all->sd->weighted_labeled_examples;
      else
        return 0;  // TODO should report NaN, but not clear how to do in platform independent manner
    else if ((all->sd->holdout_best_loss == FLT_MAX) || (all->sd->holdout_best_loss == FLT_MAX * 0.5))
      return 0;  // TODO should report NaN, but not clear how to do in platform independent manner
    else
      return all->sd->holdout_best_loss;
  }

  if (!strcmp(g_key.c_str(), "bestConstant") || !strcmp(g_key.c_str(), "bestConstantLoss"))
  {
    float best_constant;
    float best_constant_loss;
    if (!get_best_constant(*all, best_constant, best_constant_loss))
      return 0;

    if (!strcmp(g_key.c_str(), "bestConstant"))
      return best_constant;

    if (!strcmp(g_key.c_str(), "bestConstantLoss"))
      return best_constant_loss;
  }

  if (!strcmp(g_key.c_str(), "totalNumberOfFeatures"))
  {
    return all->sd->total_features;
  }

  return -1;
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_endPass(JNIEnv* env, jobject vwObj)
{
  auto all = (vw*)get_native_pointer(env, vwObj);

  try
  {
    // note: this code duplication seems bound for trouble
    // from parse_dispatch_loop.h:26
    // from learner.cc:41
    reset_source(*all, all->num_bits);
    all->do_reset_source = false;
    all->passes_complete++;

    all->current_pass++;
    all->l->end_pass();
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_finish(JNIEnv* env, jobject vwObj)
{
  auto all = (vw*)get_native_pointer(env, vwObj);

  try
  {
    VW::sync_stats(*all);
    VW::finish(*all);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jint JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_hash(
    JNIEnv* env, jclass, jbyteArray data, jint offset, jint len, jint seed)
{
  CriticalArrayGuard dataGuard(env, data);
  const char* values0 = (const char*)dataGuard.data();

  return (jint)uniform_hash(values0 + offset, len, seed);
}

// VW Example
#define INIT_VARS                                                                    \
  auto exWrapper = (VowpalWabbitExampleWrapper*)get_native_pointer(env, exampleObj); \
  vw* all = exWrapper->_all;                                                         \
  example* ex = exWrapper->_example;

JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_initialize(
    JNIEnv* env, jclass, jlong vwPtr, jboolean isEmpty)
{
  auto all = (vw*)vwPtr;

  try
  {
    example* ex = VW::alloc_examples(0, 1);
    ex->interactions = &all->interactions;

    if (isEmpty)
    {
      char empty = '\0';
      VW::read_line(*all, ex, &empty);
    }
    else
      all->p->lp.default_label(&ex->l);

    return (jlong) new VowpalWabbitExampleWrapper(all, ex);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_finish(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    VW::dealloc_example(all->p->lp.delete_label, *ex);
    ::free_it(ex);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_clear(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    VW::empty_example(*all, *ex);
    all->p->lp.default_label(&ex->l);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

void addNamespaceIfNotExists(vw* all, example* ex, char ns)
{
  if (std::find(ex->indices.begin(), ex->indices.end(), ns) == ex->indices.end())
  {
    ex->indices.push_back(ns);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_addToNamespaceDense(
    JNIEnv* env, jobject exampleObj, jchar ns, jint weight_index_base, jdoubleArray values)
{
  INIT_VARS

  try
  {
    addNamespaceIfNotExists(all, ex, ns);

    auto features = ex->feature_space + ns;

    CriticalArrayGuard valuesGuard(env, values);
    double* values0 = (double*)valuesGuard.data();

    int size = env->GetArrayLength(values);
    int mask = (1 << all->num_bits) - 1;

    // pre-allocate
    features->values.resize(features->values.end() - features->values.begin() + size);
    features->indicies.resize(features->indicies.end() - features->indicies.begin() + size);

    double* values_itr = values0;
    double* values_end = values0 + size;
    for (; values_itr != values_end; ++values_itr, ++weight_index_base)
    {
      float x = *values_itr;
      if (x != 0)
      {
        features->values.push_back_unchecked(x);
        features->indicies.push_back_unchecked(weight_index_base & mask);
      }
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_addToNamespaceSparse(
    JNIEnv* env, jobject exampleObj, jchar ns, jintArray indices, jdoubleArray values)
{
  INIT_VARS

  try
  {
    addNamespaceIfNotExists(all, ex, ns);

    auto features = ex->feature_space + ns;

    CriticalArrayGuard indicesGuard(env, indices);
    int* indices0 = (int*)indicesGuard.data();

    CriticalArrayGuard valuesGuard(env, values);
    double* values0 = (double*)valuesGuard.data();

    int size = env->GetArrayLength(indices);
    int mask = (1 << all->num_bits) - 1;

    // pre-allocate
    features->values.resize(features->values.end() - features->values.begin() + size);
    features->indicies.resize(features->indicies.end() - features->indicies.begin() + size);

    int* indices_itr = indices0;
    int* indices_end = indices0 + size;
    double* values_itr = values0;
    for (; indices_itr != indices_end; ++indices_itr, ++values_itr)
    {
      float x = *values_itr;
      if (x != 0)
      {
        features->values.push_back_unchecked(x);
        features->indicies.push_back_unchecked(*indices_itr & mask);
      }
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setLabel(
    JNIEnv* env, jobject exampleObj, jfloat weight, jfloat label)
{
  INIT_VARS

  try
  {
    label_data* ld = (label_data*)&ex->l;
    ld->label = label;
    ld->weight = weight;

    count_label(all->sd, ld->label);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

// re-use prediction conversation methods
jobject multilabel_predictor(example* vec, JNIEnv* env);
jfloatArray scalars_predictor(example* vec, JNIEnv* env);
jobject action_scores_prediction(example* vec, JNIEnv* env);
jobject action_probs_prediction(example* vec, JNIEnv* env);

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_getPrediction(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  jclass predClass;
  jmethodID ctr;
  switch (all->l->pred_type)
  {
    case prediction_type::prediction_type_t::scalar:
      predClass = env->FindClass("org/vowpalwabbit/spark/prediction/ScalarPrediction");
      ctr = env->GetMethodID(predClass, "<init>", "(FF)V");

      return env->NewObject(predClass, ctr, VW::get_prediction(ex), ex->confidence);

    case prediction_type::prediction_type_t::prob:
      predClass = env->FindClass("java/lang/Float");
      ctr = env->GetMethodID(predClass, "<init>", "(F)V");

      return env->NewObject(predClass, ctr, ex->pred.prob);

    case prediction_type::prediction_type_t::multiclass:
      predClass = env->FindClass("java/lang/Integer");
      ctr = env->GetMethodID(predClass, "<init>", "(I)V");

      return env->NewObject(predClass, ctr, ex->pred.multiclass);

    case prediction_type::prediction_type_t::scalars:
      return scalars_predictor(ex, env);

    case prediction_type::prediction_type_t::action_probs:
      return action_probs_prediction(ex, env);

    case prediction_type::prediction_type_t::action_scores:
      return action_scores_prediction(ex, env);

    case prediction_type::prediction_type_t::multilabels:
      return multilabel_predictor(ex, env);

    default:
      return nullptr;
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_learn(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    VW::setup_example(*all, ex);

    all->learn(*ex);

    // as this is not a ring-based example it is not free'd
    LEARNER::as_singleline(all->l)->finish_example(*all, *ex);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_predict(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    VW::setup_example(*all, ex);

    all->predict(*ex);

    // as this is not a ring-based example it is not free'd
    LEARNER::as_singleline(all->l)->finish_example(*all, *ex);

    return Java_org_vowpalwabbit_spark_VowpalWabbitExample_getPrediction(env, exampleObj);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}
