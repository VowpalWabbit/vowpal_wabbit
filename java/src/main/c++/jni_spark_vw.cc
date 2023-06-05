#include "jni_spark_vw.h"

#include "org_vowpalwabbit_spark_VowpalWabbitExample.h"
#include "org_vowpalwabbit_spark_VowpalWabbitNative.h"
#include "util.h"
#include "vw/common/future_compat.h"
#include "vw/common/vw_exception.h"
#include "vw/config/cli_options_serializer.h"
#include "vw/config/options.h"
#include "vw/core/best_constant.h"
#include "vw/core/global_data.h"
#include "vw/core/learner.h"
#include "vw/core/merge.h"
#include "vw/core/shared_data.h"
#include "vw/core/simple_label_parser.h"
#include "vw/core/vw_fwd.h"
#include "vw/text_parser/parse_example_text.h"

#include <algorithm>
#include <exception>

jobject getJavaPrediction(JNIEnv* env, VW::workspace* all, example* ex);

// Will finish the examples too
template <bool isLearn>
jobject callLearner(JNIEnv* env, VW::workspace* all, VW::multi_ex& examples)
{
  assert(all != nullptr);
  jobject prediction = nullptr;
  if (all->l->is_multiline())
  {
    if VW_STD17_CONSTEXPR (isLearn) { all->learn(examples); }
    else { all->predict(examples); }
    // prediction is in the first example
    prediction = getJavaPrediction(env, all, examples[0]);
    all->finish_example(examples);
  }
  else
  {
    assert(examples.size() == 1);
    if VW_STD17_CONSTEXPR (isLearn) { all->learn(*examples[0]); }
    else { all->predict(*examples[0]); }
    prediction = getJavaPrediction(env, all, examples[0]);
    all->finish_example(*examples[0]);
  }
  assert(prediction != nullptr);
  return prediction;
}

// Guards
StringGuard::StringGuard(JNIEnv* env, jstring source) : _env(env), _source(source), _cstr(nullptr)
{
  _cstr = _env->GetStringUTFChars(source, 0);
  _length = static_cast<size_t>(_env->GetStringUTFLength(source));
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
size_t StringGuard::length() { return _length; }

CriticalArrayGuard::CriticalArrayGuard(JNIEnv* env, jarray arr) : _env(env), _arr(arr), _arr0(nullptr)
{
  _arr0 = env->GetPrimitiveArrayCritical(arr, nullptr);
  _length = env->GetArrayLength(arr);
}

CriticalArrayGuard::~CriticalArrayGuard()
{
  if (_arr0 != nullptr) { _env->ReleasePrimitiveArrayCritical(_arr, _arr0, JNI_ABORT); }
}

void* CriticalArrayGuard::data() { return _arr0; }
size_t CriticalArrayGuard::length() const { return _length; }

// VW
JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_initialize(JNIEnv* env, jclass, jstring args)
{
  StringGuard g_args(env, args);

  try
  {
    return reinterpret_cast<jlong>(VW::initialize(g_args.c_str()));
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return 0;
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
    auto* model0 = reinterpret_cast<const char*>(modelGuard.data());

    VW::io_buf buffer;
    buffer.add_file(VW::io::create_buffer_view(model0, size));

    return reinterpret_cast<jlong>(VW::initialize(g_args.c_str(), &buffer));
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return 0;
  }
}

void populateMultiEx(JNIEnv* env, jobjectArray examples, VW::workspace& all, multi_ex& ex_coll)
{
  bool fieldIdInitialized = false;

  int length = env->GetArrayLength(examples);
  if (length > 0)
  {
    jobject jex = env->GetObjectArrayElement(examples, 0);

    jclass cls = env->GetObjectClass(jex);
    jfieldID fieldId = env->GetFieldID(cls, "nativePointer", "J");

    for (int i = 0; i < length; i++)
    {
      jex = env->GetObjectArrayElement(examples, i);

      // JavaObject VowpalWabbitExampleWrapper -> example*
      auto exWrapper = (VowpalWabbitExampleWrapper*)get_native_pointer(env, jex);
      VW::setup_example(all, exWrapper->_example);

      ex_coll.push_back(exWrapper->_example);
    }
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_learn(
    JNIEnv* env, jobject vwObj, jobjectArray examples)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  multi_ex ex_coll;
  try
  {
    populateMultiEx(env, examples, *all, ex_coll);
    return callLearner<true>(env, all, ex_coll);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_learnFromString(
    JNIEnv* env, jobject vwObj, jstring examplesString)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));
  StringGuard exampleStringGuard(env, examplesString);

  try
  {
    VW::multi_ex ex_coll;
    ex_coll.push_back(&VW::get_unused_example(all));
    all->parser_runtime.example_parser->text_reader(
        all, VW::string_view(exampleStringGuard.c_str(), exampleStringGuard.length()), ex_coll);
    VW::setup_examples(*all, ex_coll);
    return callLearner<true>(env, all, ex_coll);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_predict(
    JNIEnv* env, jobject vwObj, jobjectArray examples)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  multi_ex ex_coll;
  try
  {
    populateMultiEx(env, examples, *all, ex_coll);
    return callLearner<false>(env, all, ex_coll);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_predictFromString(
    JNIEnv* env, jobject vwObj, jstring examplesString)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));
  StringGuard exampleStringGuard(env, examplesString);

  try
  {
    VW::multi_ex ex_coll;
    ex_coll.push_back(&VW::get_unused_example(all));
    all->parser_runtime.example_parser->text_reader(
        all, VW::string_view(exampleStringGuard.c_str(), exampleStringGuard.length()), ex_coll);
    VW::setup_examples(*all, ex_coll);
    return callLearner<false>(env, all, ex_coll);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_performRemainingPasses(JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  try
  {
    if (all->runtime_config.numpasses > 1)
    {
      all->runtime_state.do_reset_source = true;
      VW::start_parser(*all);
      VW::LEARNER::generic_driver(*all);
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
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  try
  {  // save in stl::vector
    auto model_buffer = std::make_shared<std::vector<char>>();
    VW::io_buf buffer;
    buffer.add_file(VW::io::create_vector_writer(model_buffer));
    VW::save_predictor(*all, buffer);

    // copy to Java
    jbyteArray ret = env->NewByteArray(model_buffer->size());
    CHECK_JNI_EXCEPTION(nullptr);

    env->SetByteArrayRegion(ret, 0, model_buffer->size(), (const jbyte*)&model_buffer->data()[0]);
    CHECK_JNI_EXCEPTION(nullptr);

    return ret;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getArguments(JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  // serialize the command line
  VW::config::cli_options_serializer serializer;
  for (auto const& option : all->options->get_all_options())
  {
    if (all->options->was_supplied(option->m_name)) { serializer.add(*option); }
  }

  // move it to Java
  // Note: don't keep serializer.str().c_str() around in some variable. it get's deleted after str() is de-allocated
  jstring args = env->NewStringUTF(serializer.str().c_str());
  CHECK_JNI_EXCEPTION(nullptr);

  jclass clazz = env->FindClass("org/vowpalwabbit/spark/VowpalWabbitArguments");
  CHECK_JNI_EXCEPTION(nullptr);

  jmethodID ctor = env->GetMethodID(clazz, "<init>", "(IILjava/lang/String;DD)V");
  CHECK_JNI_EXCEPTION(nullptr);

  return env->NewObject(clazz, ctor, all->initial_weights_config.num_bits, all->runtime_config.hash_seed, args,
      all->update_rule_config.eta, all->update_rule_config.power_t);
}

JNIEXPORT jstring JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getOutputPredictionType(
    JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  // produce string to avoid replication of enum types
  return env->NewStringUTF(std::string(VW::to_string(all->l->get_output_prediction_type())).c_str());
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_getPerformanceStatistics(
    JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  long numberOfExamplesPerPass;
  double weightedExampleSum;
  double weightedLabelSum;
  double averageLoss;
  float bestConstant;
  float bestConstantLoss;
  long totalNumberOfFeatures;

  if (all->passes_config.current_pass == 0)
    numberOfExamplesPerPass = all->sd->example_number;
  else
    numberOfExamplesPerPass = all->sd->example_number / all->passes_config.current_pass;

  weightedExampleSum = all->sd->weighted_examples();
  weightedLabelSum = all->sd->weighted_labels;

  if (all->passes_config.holdout_set_off)
    if (all->sd->weighted_labeled_examples > 0)
      averageLoss = all->sd->sum_loss / all->sd->weighted_labeled_examples;
    else
      averageLoss = 0;  // TODO should report NaN, but not clear how to do in platform independent manner
  else if ((all->sd->holdout_best_loss == FLT_MAX) || (all->sd->holdout_best_loss == FLT_MAX * 0.5))
    averageLoss = 0;  // TODO should report NaN, but not clear how to do in platform independent manner
  else
    averageLoss = all->sd->holdout_best_loss;

  VW::get_best_constant(*all->loss_config.loss, *all->sd, bestConstant, bestConstantLoss);
  totalNumberOfFeatures = all->sd->total_features;

  jclass clazz = env->FindClass("org/vowpalwabbit/spark/VowpalWabbitPerformanceStatistics");
  CHECK_JNI_EXCEPTION(nullptr);

  jmethodID ctor = env->GetMethodID(clazz, "<init>", "(JDDDFFJ)V");
  CHECK_JNI_EXCEPTION(nullptr);

  return env->NewObject(clazz, ctor, numberOfExamplesPerPass, weightedExampleSum, weightedLabelSum, averageLoss,
      bestConstant, bestConstantLoss, totalNumberOfFeatures);
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_endPass(JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  try
  {
    // note: this code duplication seems bound for trouble
    // from parse_dispatch_loop.h:26
    // from learner.cc:41
    VW::details::reset_source(*all, all->initial_weights_config.num_bits);
    all->runtime_state.do_reset_source = false;
    all->runtime_state.passes_complete++;

    all->passes_config.current_pass++;
    all->l->end_pass();
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_finish(JNIEnv* env, jobject vwObj)
{
  auto* all = reinterpret_cast<VW::workspace*>(get_native_pointer(env, vwObj));

  try
  {
    VW::sync_stats(*all);
    all->finish();
    delete all;
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

  return (jint)VW::uniform_hash(values0 + offset, len, seed);
}

// VW Example
#define INIT_VARS                                                                                      \
  auto exWrapper = reinterpret_cast<VowpalWabbitExampleWrapper*>(get_native_pointer(env, exampleObj)); \
  VW::workspace* all = exWrapper->_all;                                                                \
  example* ex = exWrapper->_example;

JNIEXPORT jlong JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_initialize(
    JNIEnv* env, jclass, jlong vwPtr, jboolean isEmpty)
{
  auto* all = reinterpret_cast<VW::workspace*>(vwPtr);

  try
  {
    example* ex = new VW::example;
    ex->interactions = &all->feature_tweaks_config.interactions;
    ex->extent_interactions = &all->feature_tweaks_config.extent_interactions;

    if (isEmpty)
    {
      char empty = '\0';
      VW::parsers::text::read_line(*all, ex, &empty);
    }
    else
      all->parser_runtime.example_parser->lbl_parser.default_label(ex->l);

    return reinterpret_cast<jlong>(new VowpalWabbitExampleWrapper(all, ex));
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return 0;
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_finish(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    delete ex;
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
    all->parser_runtime.example_parser->lbl_parser.default_label(ex->l);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

void addNamespaceIfNotExists(VW::workspace* all, example* ex, char ns)
{
  if (std::find(ex->indices.begin(), ex->indices.end(), ns) == ex->indices.end()) { ex->indices.push_back(ns); }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_addToNamespaceDense(
    JNIEnv* env, jobject exampleObj, jchar ns, jint weight_index_base, jdoubleArray values)
{
  INIT_VARS

  try
  {
    addNamespaceIfNotExists(all, ex, ns);

    auto features = ex->feature_space.data() + ns;

    CriticalArrayGuard valuesGuard(env, values);
    double* values0 = (double*)valuesGuard.data();

    int size = env->GetArrayLength(values);
    int mask = (1 << all->initial_weights_config.num_bits) - 1;

    // pre-allocate
    features->values.reserve(features->values.capacity() + size);
    features->indices.reserve(features->indices.capacity() + size);

    double* values_itr = values0;
    double* values_end = values0 + size;
    for (; values_itr != values_end; ++values_itr, ++weight_index_base)
    {
      float x = *values_itr;
      if (x != 0)
      {
        features->values.push_back_unchecked(x);
        features->indices.push_back_unchecked(weight_index_base & mask);
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

    auto features = ex->feature_space.data() + ns;

    CriticalArrayGuard indicesGuard(env, indices);
    int* indices0 = (int*)indicesGuard.data();

    CriticalArrayGuard valuesGuard(env, values);
    double* values0 = (double*)valuesGuard.data();

    int size = env->GetArrayLength(indices);
    int mask = (1 << all->initial_weights_config.num_bits) - 1;

    // pre-allocate
    features->values.reserve(features->values.capacity() + size);
    features->indices.reserve(features->indices.capacity() + size);

    int* indices_itr = indices0;
    int* indices_end = indices0 + size;
    double* values_itr = values0;
    for (; indices_itr != indices_end; ++indices_itr, ++values_itr)
    {
      float x = *values_itr;
      if (x != 0)
      {
        features->values.push_back_unchecked(x);
        features->indices.push_back_unchecked(*indices_itr & mask);
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
    auto* ld = &ex->l.simple;
    ld->label = label;
    auto& red_fts = ex->ex_reduction_features.template get<VW::simple_label_reduction_features>();
    red_fts.weight = weight;

    VW::count_label(*all->sd, ld->label);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setDefaultLabel(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    all->parser_runtime.example_parser->lbl_parser.default_label(ex->l);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setMulticlassLabel(
    JNIEnv* env, jobject exampleObj, jfloat weight, jint label)
{
  INIT_VARS

  try
  {
    VW::multiclass_label* ld = &ex->l.multi;

    ld->label = label;
    ld->weight = weight;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setCostSensitiveLabels(
    JNIEnv* env, jobject exampleObj, jfloatArray costs, jintArray classes)
{
  INIT_VARS

  try
  {
    VW::cs_label* ld = &ex->l.cs;

    int sizeCosts = env->GetArrayLength(costs);
    int sizeClasses = env->GetArrayLength(classes);

    if (sizeCosts != sizeClasses)
    {
      env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "costs and classes length must match");
      return;
    }

    CriticalArrayGuard costsGuard(env, costs);
    float* costs0 = (float*)costsGuard.data();

    CriticalArrayGuard classesGuard(env, classes);
    int* classes0 = (int*)classesGuard.data();

    // loop over weights/labels
    for (int i = 0; i < sizeCosts; i++)
    {
      VW::cs_class w;
      w.x = costs0[i];
      w.class_index = classes0[i];

      ld->costs.push_back(w);
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setMultiLabels(
    JNIEnv* env, jobject exampleObj, jintArray classes)
{
  INIT_VARS

  try
  {
    auto* ld = &ex->l.multilabels;

    CriticalArrayGuard classesGuard(env, classes);
    int* classes0 = (int*)classesGuard.data();

    int size = env->GetArrayLength(classes);

    for (int i = 0; i < size; i++) ld->label_v.push_back(classes0[i]);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setContextualBanditContinuousLabel(
    JNIEnv* env, jobject exampleObj, jfloatArray actions, jfloatArray costs, jfloatArray pdfValues)
{
  INIT_VARS

  try
  {
    int sizeActions = env->GetArrayLength(actions);
    int sizeCosts = env->GetArrayLength(costs);
    int sizePdfValues = env->GetArrayLength(pdfValues);

    if (sizeActions != sizeCosts || sizeCosts != sizePdfValues)
    {
      env->ThrowNew(
          env->FindClass("java/lang/IllegalArgumentException"), "actions, costs and pdfValues length must match");
      return;
    }

    VW::cb_continuous::continuous_label* ld = &ex->l.cb_cont;

    CriticalArrayGuard actionsGuard(env, actions);
    float* actions0 = (float*)actionsGuard.data();

    CriticalArrayGuard costsGuard(env, costs);
    float* costs0 = (float*)costsGuard.data();

    CriticalArrayGuard pdfValuesGuard(env, pdfValues);
    float* pdfValues0 = (float*)pdfValuesGuard.data();

    for (int i = 0; i < sizeActions; i++)
    {
      VW::cb_continuous::continuous_label_elm elm;
      elm.action = actions0[i];
      elm.cost = costs0[i];
      elm.pdf_value = pdfValues0[i];
      ld->costs.push_back(elm);
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setContextualBanditLabel(
    JNIEnv* env, jobject exampleObj, jint action, jdouble cost, jdouble probability)
{
  INIT_VARS

  try
  {
    VW::cb_label* ld = &ex->l.cb;
    VW::cb_class f;

    f.action = (uint32_t)action;
    f.cost = (float)cost;
    f.probability = (float)probability;

    ld->costs.push_back(f);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSharedLabel(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    // https://github.com/VowpalWabbit/vowpal_wabbit/blob/master/vowpalwabbit/parse_example_json.h#L437
    VW::cb_label* ld = &ex->l.cb;
    VW::cb_class f;

    f.partial_prediction = 0.;
    f.action = (uint32_t)VW::uniform_hash("shared", 6 /*length of string*/, 0);
    f.cost = FLT_MAX;
    f.probability = -1.f;

    ld->costs.push_back(f);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSlatesSharedLabel(
    JNIEnv* env, jobject exampleObj, jfloat cost)
{
  INIT_VARS

  try
  {
    auto* ld = &ex->l.slates;
    ld->reset_to_default();
    ld->type = VW::slates::example_type::SHARED;
    ld->cost = cost;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSlatesActionLabel(
    JNIEnv* env, jobject exampleObj, jint slot_id)
{
  INIT_VARS

  try
  {
    auto* ld = &ex->l.slates;
    ld->reset_to_default();
    ld->type = VW::slates::example_type::ACTION;
    ld->slot_id = slot_id;
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_setSlatesSlotLabel(
    JNIEnv* env, jobject exampleObj, jintArray actions, jfloatArray probs)
{
  INIT_VARS

  try
  {
    int sizeActions = env->GetArrayLength(actions);
    int sizeProbs = env->GetArrayLength(probs);

    if (sizeActions != sizeProbs)
    {
      env->ThrowNew(env->FindClass("java/lang/IllegalArgumentException"), "actions and probs length must match");
      return;
    }

    auto* ld = &ex->l.slates;
    ld->reset_to_default();
    ld->type = VW::slates::example_type::SLOT;

    CriticalArrayGuard actionsGuard(env, actions);
    float* actions0 = (float*)actionsGuard.data();

    CriticalArrayGuard probsGuard(env, probs);
    float* probs0 = (float*)probsGuard.data();

    for (int i = 0; i < sizeActions; i++)
    {
      VW::action_score as;
      as.action = actions0[i];
      as.score = probs0[i];
      ld->probabilities.push_back(as);
    }
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_getPrediction(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  return getJavaPrediction(env, all, ex);
}

JNIEXPORT void JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_learn(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    VW::setup_example(*all, ex);

    all->learn(*ex);

    // as this is not a ring-based example it is not free'd
    all->l->finish_example(*all, *ex);
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
    all->l->finish_example(*all, *ex);

    return getJavaPrediction(env, all, ex);
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

JNIEXPORT jstring JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitExample_toString(JNIEnv* env, jobject exampleObj)
{
  INIT_VARS

  try
  {
    std::ostringstream ostr;

    ostr << "VowpalWabbitExample(label=";
    auto lp = all->parser_runtime.example_parser->lbl_parser;

    if (!memcmp(&lp, &VW::simple_label_parser_global, sizeof(lp)))
    {
      auto* ld = &ex->l.simple;
      const auto& red_fts = ex->ex_reduction_features.template get<VW::simple_label_reduction_features>();
      ostr << "simple " << ld->label << ":" << red_fts.weight << ":" << red_fts.initial;
    }
    else if (!memcmp(&lp, &VW::cb_label_parser_global, sizeof(lp)))
    {
      VW::cb_label* ld = &ex->l.cb;
      ostr << "CB " << ld->costs.size();

      if (ld->costs.size() > 0)
      {
        ostr << " ";

        VW::cb_class& f = ld->costs[0];

        // Ignore checking if f.action == VW::uniform_hash("shared")
        if (f.partial_prediction == 0 && f.cost == FLT_MAX && f.probability == -1.f)
          ostr << "shared";
        else
          ostr << f.action << ":" << f.cost << ":" << f.probability;
      }
    }
    else { ostr << "unsupported label"; }

    ostr << ";";
    for (auto& ns : ex->indices)
    {
      if (ns == 0)
        ostr << "NULL:0,";
      else
      {
        if ((ns >= 'a' && ns <= 'z') || (ns >= 'A' && ns <= 'Z')) ostr << "'" << (char)ns << "':";

        ostr << (int)ns << ",";
      }

      for (auto& f : ex->feature_space[ns])
      {
        auto idx = f.index();
        ostr << (idx & all->weights.mask()) << "/" << idx << ":" << f.value() << ", ";
      }
    }

    ostr << ")";

    return env->NewStringUTF(ostr.str().c_str());
  }
  catch (...)
  {
    rethrow_cpp_exception_as_java_exception(env);
    return nullptr;
  }
}

// re-use prediction conversation methods
jobject multilabel_predictor(example* vec, JNIEnv* env);
jfloatArray scalars_predictor(example* vec, JNIEnv* env);
jobject action_scores_prediction(example* vec, JNIEnv* env);
jobject action_probs_prediction(example* vec, JNIEnv* env);
jobject decision_scores_prediction(example* vec, JNIEnv* env);

jobject probability_density_function_value(example* ex, JNIEnv* env)
{
  jclass predClass = env->FindClass("vowpalWabbit/responses/PDFValue");
  CHECK_JNI_EXCEPTION(nullptr);

  jmethodID ctr = env->GetMethodID(predClass, "<init>", "(FF)V");
  CHECK_JNI_EXCEPTION(nullptr);

  return env->NewObject(predClass, ctr, ex->pred.pdf_value.action, ex->pred.pdf_value.pdf_value);
}

jobject probability_density_function(example* ex, JNIEnv* env)
{
  jclass pdfSegmentClass = env->FindClass("vowpalWabbit/responses/PDFSegment");
  CHECK_JNI_EXCEPTION(nullptr);

  jmethodID ctrPdfSegment = env->GetMethodID(pdfSegmentClass, "<init>", "(FFF)V");
  CHECK_JNI_EXCEPTION(nullptr);

  jclass pdfClass = env->FindClass("vowpalWabbit/responses/PDF");
  CHECK_JNI_EXCEPTION(nullptr);

  jmethodID ctrPdf = env->GetMethodID(pdfClass, "<init>", "([LvowpalWabbit/responses/PDFSegment;)V");
  CHECK_JNI_EXCEPTION(nullptr);

  auto& pdf = ex->pred.pdf;

  jobjectArray pdfSegments = env->NewObjectArray(pdf.size(), pdfSegmentClass, 0);
  for (uint32_t i = 0; i < pdf.size(); ++i)
  {
    auto& pdfSegment = pdf[i];

    jobject pdfSegmentObj =
        env->NewObject(pdfSegmentClass, ctrPdfSegment, pdfSegment.left, pdfSegment.right, pdfSegment.pdf_value);

    env->SetObjectArrayElement(pdfSegments, i, pdfSegmentObj);
  }

  return env->NewObject(pdfClass, ctrPdf, pdfSegments);
}

jobject getJavaPrediction(JNIEnv* env, VW::workspace* all, example* ex)
{
  jclass predClass;
  jmethodID ctr;
  switch (all->l->get_output_prediction_type())
  {
    case VW::prediction_type_t::SCALAR:
      predClass = env->FindClass("org/vowpalwabbit/spark/prediction/ScalarPrediction");
      CHECK_JNI_EXCEPTION(nullptr);

      ctr = env->GetMethodID(predClass, "<init>", "(FF)V");
      CHECK_JNI_EXCEPTION(nullptr);

      return env->NewObject(predClass, ctr, VW::get_prediction(ex), ex->confidence);

    case VW::prediction_type_t::PROB:
      predClass = env->FindClass("java/lang/Float");
      CHECK_JNI_EXCEPTION(nullptr);

      ctr = env->GetMethodID(predClass, "<init>", "(F)V");
      CHECK_JNI_EXCEPTION(nullptr);

      return env->NewObject(predClass, ctr, ex->pred.prob);

    case VW::prediction_type_t::MULTICLASS:
      predClass = env->FindClass("java/lang/Integer");
      CHECK_JNI_EXCEPTION(nullptr);

      ctr = env->GetMethodID(predClass, "<init>", "(I)V");
      CHECK_JNI_EXCEPTION(nullptr);

      return env->NewObject(predClass, ctr, ex->pred.multiclass);

    case VW::prediction_type_t::SCALARS:
      return scalars_predictor(ex, env);

    case VW::prediction_type_t::ACTION_PROBS:
      return action_probs_prediction(ex, env);

    case VW::prediction_type_t::ACTION_SCORES:
      return action_scores_prediction(ex, env);

    case VW::prediction_type_t::MULTILABELS:
      return multilabel_predictor(ex, env);

    case VW::prediction_type_t::DECISION_PROBS:
      return decision_scores_prediction(ex, env);

    case VW::prediction_type_t::PDF:
      return probability_density_function(ex, env);

    case VW::prediction_type_t::ACTION_PDF_VALUE:
      return probability_density_function_value(ex, env);

    default:
    {
      std::ostringstream ostr;
      ostr << "prediction type '" << VW::to_string(all->l->get_output_prediction_type()) << "' is not supported";

      env->ThrowNew(env->FindClass("java/lang/UnsupportedOperationException"), ostr.str().c_str());
      return nullptr;
    }
  }
}

JNIEXPORT jobject JNICALL Java_org_vowpalwabbit_spark_VowpalWabbitNative_mergeModels(
    JNIEnv* env, jclass, jobject baseWorkspace, jobjectArray workspacePointers)
try
{
  VW::workspace* base = nullptr;
  if (baseWorkspace != nullptr) { base = reinterpret_cast<VW::workspace*>(get_native_pointer(env, baseWorkspace)); }

  std::vector<const VW::workspace*> workspaces;
  int length = env->GetArrayLength(workspacePointers);
  if (length > 0)
  {
    workspaces.reserve(length);
    jobject jworkspace = env->GetObjectArrayElement(workspacePointers, 0);
    jclass cls = env->GetObjectClass(jworkspace);
    jfieldID fieldId = env->GetFieldID(cls, "nativePointer", "J");
    for (int i = 0; i < length; i++)
    {
      const auto* workspace = reinterpret_cast<const VW::workspace*>(get_native_pointer(env, jworkspace));
      workspaces.push_back(workspace);
    }
  }

  auto result = VW::merge_models(base, workspaces);

  jclass clazz = env->FindClass("org/vowpalwabbit/spark/VowpalWabbitNative");
  CHECK_JNI_EXCEPTION(nullptr);

  jmethodID ctor = env->GetMethodID(clazz, "<init>", "(J)V");
  CHECK_JNI_EXCEPTION(nullptr);

  return env->NewObject(clazz, ctor, reinterpret_cast<jlong>(result.release()));
}
catch (...)
{
  rethrow_cpp_exception_as_java_exception(env);
  return nullptr;
}
