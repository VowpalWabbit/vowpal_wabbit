#include "vowpalWabbit_learner_VWPDFLearner.h"

#include "jni_base_learner.h"
#include "vw/core/vw.h"

jobject pdf_prediction(example* ex, JNIEnv* env)
{
  jclass pdfSegmentClass = env->FindClass("vowpalWabbit/responses/PDFSegment");
  jmethodID ctrPdfSegment = env->GetMethodID(pdfSegmentClass, "<init>", "(FFF)V");

  jclass pdfClass = env->FindClass("vowpalWabbit/responses/PDF");
  jmethodID ctrPdf = env->GetMethodID(pdfClass, "<init>", "([LvowpalWabbit/responses/PDFSegment;)V");

  auto& pdf = ex->pred.pdf;
  size_t num_segments = pdf.size();
  jobjectArray j_segments = env->NewObjectArray(num_segments, pdfSegmentClass, 0);

  for (size_t i = 0; i < num_segments; ++i)
  {
    auto& segment = pdf[i];
    jobject j_segment = env->NewObject(pdfSegmentClass, ctrPdfSegment, segment.left, segment.right, segment.pdf_value);
    env->SetObjectArrayElement(j_segments, i, j_segment);
  }

  return env->NewObject(pdfClass, ctrPdf, j_segments);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWPDFLearner_predict(
    JNIEnv* env, jobject obj, jstring example_string, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_string, learn, vwPtr, pdf_prediction);
}

JNIEXPORT jobject JNICALL Java_vowpalWabbit_learner_VWPDFLearner_predictMultiline(
    JNIEnv* env, jobject obj, jobjectArray example_strings, jboolean learn, jlong vwPtr)
{
  return base_predict<jobject>(env, example_strings, learn, vwPtr, pdf_prediction);
}
