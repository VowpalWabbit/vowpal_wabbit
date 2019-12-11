// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved.  Released under a BSD
// license as described in the file LICENSE.

#include <memory>
#include <codecvt>
#include <locale>
#include <string>

#include "vwdll.h"
#include "parser.h"
#include "simple_label.h"
#include "parse_args.h"
#include "vw.h"

// This interface now provides "wide" functions for compatibility with .NET interop
// The default functions assume a wide (16 bit char pointer) that is converted to a utf8-string and passed to
// a function which takes a narrow (8 bit char pointer) function. Both are exposed in the c/c++ API
// so that programs using 8 bit wide characters can use the direct call without conversion and
//  programs using 16 bit characters can use the default wide versions of the functions.
// "Ansi versions  (FcnA instead of Fcn) have only been written for functions which handle strings.

// a future optimization would be to write an inner version of hash feature which either hashed the
// wide string directly (and live with the different hash values) or incorporate the UTF-16 to UTF-8 conversion
// in the hashing to avoid allocating an intermediate string.

#if _MSC_VER >= 1900
// VS 2015 Bug: https://social.msdn.microsoft.com/Forums/en-US/8f40dcd8-c67f-4eba-9134-a19b9178e481/vs-2015-rc-linker-stdcodecvt-error?forum=vcgeneral
std::string utf16_to_utf8(std::u16string utf16_string)
{
  std::wstring_convert<std::codecvt_utf8_utf16<int16_t>, int16_t> convert;
  auto p = reinterpret_cast<const int16_t *>(utf16_string.data());
  return convert.to_bytes(p, p + utf16_string.size());
}

#else
std::string utf16_to_utf8(std::u16string utf16_string)
{
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> convert;
  return convert.to_bytes(utf16_string);
}
#endif

extern "C"
{ using namespace std;

#ifdef USE_CODECVT
  VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_Initialize(const char16_t * pstrArgs)
{ return VW_InitializeA(utf16_to_utf8(pstrArgs).c_str());
}

VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_InitializeEscaped(const char16_t* pstrArgs)
{
  return VW_InitializeEscapedA(utf16_to_utf8(pstrArgs).c_str());
}
#endif


VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_InitializeA(const char * pstrArgs)
{ string s(pstrArgs);
  vw* all = VW::initialize(s);
  return static_cast<VW_HANDLE>(all);
}

VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_InitializeEscapedA(const char* pstrArgs)
{
  std::string s(pstrArgs);
  auto all = VW::initialize_escaped(s);
  return static_cast<VW_HANDLE>(all);
}

VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_SeedWithModel(VW_HANDLE handle, const char * extraArgs)
{
  string s(extraArgs);
  vw* origmodel = static_cast<vw*>(handle);
  vw* newmodel = VW::seed_vw_model(origmodel, s);
  return static_cast<VW_HANDLE>(newmodel);
}

VW_DLL_MEMBER void      VW_CALLING_CONV VW_Finish_Passes(VW_HANDLE handle)
{ vw * pointer = static_cast<vw*>(handle);
  if (pointer->numpasses > 1)
  { adjust_used_index(*pointer);
    pointer->do_reset_source = true;
    VW::start_parser(*pointer);
    LEARNER::generic_driver(*pointer);
    VW::end_parser(*pointer);
  }
}

VW_DLL_MEMBER void      VW_CALLING_CONV VW_Finish(VW_HANDLE handle)
{ vw * pointer = static_cast<vw*>(handle);
  VW::finish(*pointer);
}

VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ImportExample(VW_HANDLE handle, const char * label, VW_FEATURE_SPACE features, size_t len)
{ vw * pointer = static_cast<vw*>(handle);
  VW::primitive_feature_space * f = reinterpret_cast<VW::primitive_feature_space*>( features );
  return static_cast<VW_EXAMPLE>(VW::import_example(*pointer, label, f, len));
}

VW_DLL_MEMBER VW_FEATURE_SPACE VW_CALLING_CONV VW_InitializeFeatureSpaces(size_t len)
{
  return static_cast<VW_FEATURE_SPACE>(new VW::primitive_feature_space[len]);
}

VW_DLL_MEMBER VW_FEATURE_SPACE VW_CALLING_CONV VW_GetFeatureSpace(VW_FEATURE_SPACE first, size_t index)
{
  VW::primitive_feature_space* f = reinterpret_cast<VW::primitive_feature_space*>(first);
  return static_cast<VW_FEATURE_SPACE>(&f[index]);
}

VW_DLL_MEMBER VW_FEATURE_SPACE VW_CALLING_CONV VW_ExportExample(VW_HANDLE handle, VW_EXAMPLE e, size_t * plen)
{ vw* pointer = static_cast<vw*>(handle);
  example* ex = static_cast<example*>(e);
  return static_cast<VW_FEATURE_SPACE>(VW::export_example(*pointer, ex, *plen));
}

VW_DLL_MEMBER void VW_CALLING_CONV VW_ReleaseFeatureSpace(VW_FEATURE_SPACE features, size_t len)
{ VW::primitive_feature_space * f = reinterpret_cast<VW::primitive_feature_space*>( features );
  VW::releaseFeatureSpace(f, len);
}
#ifdef USE_CODECVT
VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExample(VW_HANDLE handle, const char16_t * line)
{ return VW_ReadExampleA(handle, utf16_to_utf8(line).c_str());
}
#endif
VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_ReadExampleA(VW_HANDLE handle, const char * line)
{ vw * pointer = static_cast<vw*>(handle);
  // BUGBUG: I really dislike this const_cast. should VW really change the input string?
  return static_cast<VW_EXAMPLE>(VW::read_example(*pointer, const_cast<char*>(line)));
}

VW_DLL_MEMBER void VW_CALLING_CONV VW_StartParser(VW_HANDLE handle)
{ vw * pointer = static_cast<vw*>(handle);
  VW::start_parser(*pointer);
}

VW_DLL_MEMBER void VW_CALLING_CONV VW_EndParser(VW_HANDLE handle)
{ vw * pointer = static_cast<vw*>(handle);
  VW::end_parser(*pointer);
}

VW_DLL_MEMBER VW_EXAMPLE VW_CALLING_CONV VW_GetExample(VW_HANDLE handle)
{ vw * pointer = static_cast<vw*>(handle);
  parser * parser_pointer = static_cast<parser *>(pointer->p);
  return static_cast<VW_EXAMPLE>(VW::get_example(parser_pointer));
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_GetLabel(VW_EXAMPLE e)
{ return VW::get_label(static_cast<example*>(e));
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_GetTopicPrediction(VW_EXAMPLE e, size_t i)
{ return VW::get_topic_prediction(static_cast<example*>(e), i);
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_GetImportance(VW_EXAMPLE e)
{ return VW::get_importance(static_cast<example*>(e));
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_GetInitial(VW_EXAMPLE e)
{ return VW::get_initial(static_cast<example*>(e));
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_GetPrediction(VW_EXAMPLE e)
{ return VW::get_prediction(static_cast<example*>(e));
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_GetCostSensitivePrediction(VW_EXAMPLE e)
{ return VW::get_cost_sensitive_prediction(static_cast<example*>(e));
}

VW_DLL_MEMBER void* VW_CALLING_CONV VW_GetMultilabelPredictions(VW_EXAMPLE e, size_t* plen)
{ return VW::get_multilabel_predictions(static_cast<example*>(e), *plen);
}

VW_DLL_MEMBER size_t VW_CALLING_CONV VW_GetTagLength(VW_EXAMPLE e)
{ return VW::get_tag_length(static_cast<example*>(e));
}

VW_DLL_MEMBER const char* VW_CALLING_CONV VW_GetTag(VW_EXAMPLE e)
{ return VW::get_tag(static_cast<example*>(e));
}

VW_DLL_MEMBER size_t VW_CALLING_CONV VW_GetFeatureNumber(VW_EXAMPLE e)
{ return VW::get_feature_number(static_cast<example*>(e));
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_GetConfidence(VW_EXAMPLE e)
{ return VW::get_confidence(static_cast<example*>(e));
}

VW_DLL_MEMBER size_t VW_CALLING_CONV VW_SetFeatureSpace(VW_HANDLE handle, VW_FEATURE_SPACE feature_space, const char* name)
{ VW::primitive_feature_space* f = reinterpret_cast<VW::primitive_feature_space*>(feature_space);
  f->name = *name;
  return VW_HashSpaceA(handle, name);
}

VW_DLL_MEMBER void VW_CALLING_CONV VW_InitFeatures(VW_FEATURE_SPACE feature_space, size_t features_count)
{
  VW::primitive_feature_space* fs = reinterpret_cast<VW::primitive_feature_space*>(feature_space);
  VW::init_features(*fs, features_count);
}

VW_DLL_MEMBER VW_FEATURE VW_CALLING_CONV VW_GetFeature(VW_FEATURE_SPACE feature_space, size_t index)
{
  VW::primitive_feature_space* fs = reinterpret_cast<VW::primitive_feature_space*>(feature_space);
  return &(fs->fs[index]);
}

VW_DLL_MEMBER void VW_CALLING_CONV VW_SetFeature(VW_FEATURE_SPACE feature_space, size_t index, size_t feature_hash, float value)
{
  VW::primitive_feature_space* fs = reinterpret_cast<VW::primitive_feature_space*>(feature_space);
  VW::set_feature(*fs, index, feature_hash, value);
}

VW_DLL_MEMBER VW_FEATURE VW_CALLING_CONV VW_GetFeatures(VW_HANDLE handle, VW_EXAMPLE e, size_t* plen)
{ vw* pointer = static_cast<vw*>(handle);
  return VW::get_features(*pointer, static_cast<example*>(e), *plen);
}

VW_DLL_MEMBER void VW_CALLING_CONV VW_ReturnFeatures(VW_FEATURE f)
{ VW::return_features(static_cast<feature*>(f));
}
VW_DLL_MEMBER void VW_CALLING_CONV VW_FinishExample(VW_HANDLE handle, VW_EXAMPLE e)
{ vw * pointer = static_cast<vw*>(handle);
  VW::finish_example(*pointer, *(static_cast<example*>(e)));
}
#ifdef USE_CODECVT
VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashSpace(VW_HANDLE handle, const char16_t * s)
{ return VW_HashSpaceA(handle, utf16_to_utf8(s).c_str());
}

VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashSpaceStatic(const char16_t * s, const char16_t * h)
{ return VW_HashSpaceStaticA(utf16_to_utf8(s).c_str(), utf16_to_utf8(h).c_str());
}
#endif
VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashSpaceA(VW_HANDLE handle, const char * s)
{ vw * pointer = static_cast<vw*>(handle);
  string str(s);
  return VW::hash_space(*pointer, str);
}

VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashSpaceStaticA(const char * s, const char* h = "strings")
{ string str(s);
  string hash(h);
  return VW::hash_space_static(str, hash);
}

#ifdef USE_CODECVT
VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashFeature(VW_HANDLE handle, const char16_t* s, size_t u)
{ return VW_HashFeatureA(handle, utf16_to_utf8(s).c_str(),u);
}

VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashFeatureStatic(const char16_t * s, size_t u, const char16_t * h, unsigned int num_bits)
{ return VW_HashFeatureStaticA(utf16_to_utf8(s).c_str(), u, utf16_to_utf8(h).c_str(), num_bits);
}
#endif

VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashFeatureA(VW_HANDLE handle, const char * s, size_t u)
{ vw * pointer = static_cast<vw*>(handle);
  string str(s);
  return VW::hash_feature(*pointer, str, u);
}

VW_DLL_MEMBER size_t VW_CALLING_CONV VW_HashFeatureStaticA(const char * s, size_t u, const char * h = "strings", unsigned int num_bits = 18)
{ string str(s);
  string hash(h);
  return VW::hash_feature_static(str, u, hash, num_bits);
}

VW_DLL_MEMBER void  VW_CALLING_CONV VW_AddLabel(VW_EXAMPLE e, float label, float weight, float base)
{ example* ex = static_cast<example*>(e);
  return VW::add_label(ex, label, weight, base);
}

VW_DLL_MEMBER void VW_CALLING_CONV VW_AddStringLabel(VW_HANDLE handle, VW_EXAMPLE e, const char* label)
{ vw * pointer = static_cast<vw*>(handle);
  example* ex = static_cast<example*>(e);
  VW::parse_example_label(*pointer, *ex, label);
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_Learn(VW_HANDLE handle, VW_EXAMPLE e)
{ vw * pointer = static_cast<vw*>(handle);
  example * ex = static_cast<example*>(e);
  pointer->learn(*ex);
  return VW::get_prediction(ex);
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_GetActionScore(VW_EXAMPLE e, size_t i)
{ example * ex = static_cast<example*>(e);
  return VW::get_action_score(ex, i);
}

VW_DLL_MEMBER size_t VW_CALLING_CONV VW_GetActionScoreLength(VW_EXAMPLE e)
{ example * ex = static_cast<example*>(e);
  return VW::get_action_score_length(ex);
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_Predict(VW_HANDLE handle, VW_EXAMPLE e)
{ vw * pointer = static_cast<vw*>(handle);
  example * ex = static_cast<example*>(e);
  LEARNER::as_singleline(pointer->l)->predict(*ex);
  //BUG: The below method may return garbage as it assumes a certain structure for ex->ld
  //which may not be the actual one used (e.g., for cost-sensitive multi-class learning)
  return VW::get_prediction(ex);
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_PredictCostSensitive(VW_HANDLE handle, VW_EXAMPLE e)
{ vw * pointer = static_cast<vw*>(handle);
  example * ex = static_cast<example*>(e);
  LEARNER::as_singleline(pointer->l)->predict(*ex);
  return VW::get_cost_sensitive_prediction(ex);
}

VW_DLL_MEMBER float VW_CALLING_CONV VW_Get_Weight(VW_HANDLE handle, size_t index, size_t offset)
{ vw* pointer = static_cast<vw*>(handle);
  return VW::get_weight(*pointer, (uint32_t) index, (uint32_t) offset);
}

VW_DLL_MEMBER void VW_CALLING_CONV VW_Set_Weight(VW_HANDLE handle, size_t index, size_t offset, float value)
{ vw* pointer = static_cast<vw*>(handle);
  return VW::set_weight(*pointer, (uint32_t) index, (uint32_t)offset, value);
}

VW_DLL_MEMBER size_t VW_CALLING_CONV VW_Num_Weights(VW_HANDLE handle)
{ vw* pointer = static_cast<vw*>(handle);
  return VW::num_weights(*pointer);
}

VW_DLL_MEMBER size_t VW_CALLING_CONV VW_Get_Stride(VW_HANDLE handle)
{ vw* pointer = static_cast<vw*>(handle);
  return VW::get_stride(*pointer);
}

VW_DLL_MEMBER void VW_CALLING_CONV VW_SaveModel(VW_HANDLE handle)
{ vw* pointer = static_cast<vw*>(handle);

  string name = pointer->final_regressor_name;
  if (name.empty())
  { return;
  }

  return VW::save_predictor(*pointer, name);
}


class memory_io_buf : public io_buf
{
public:
    memory_io_buf() : readOffset(0) {
        files.push_back(-1); // this is a hack because buf will do nothing if files is empty
    }

    virtual ssize_t write_file(int file, const void* buf, size_t nbytes) {
        auto byteBuf = reinterpret_cast<const char*>(buf);
        data.insert(data.end(), &byteBuf[0], &byteBuf[nbytes]);
        return nbytes;
    }

    virtual ssize_t read_file(int f, void* buf, size_t nbytes) {
        nbytes = min(nbytes, data.size()-readOffset);
        copy(data.data()+readOffset, data.data()+readOffset+nbytes, reinterpret_cast<char *>(buf));
        readOffset += nbytes;
        return nbytes;
    }

    virtual bool close_file() {
       if (files.size() > 0) {
            files.pop();
            return true;
        }
        return false;
    }

    char* GetDataPointer() {
        return data.data();
    }

    size_t GetDataSize() const {
        return data.size();
    }

private:
    vector<char> data;
    size_t readOffset;
};

VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_InitializeWithModel(const char * pstrArgs, const char * modelData, size_t modelDataSize)
{
    unique_ptr<memory_io_buf> buf(new memory_io_buf);
    buf->write_file(-1, modelData, modelDataSize);

    vw* all = VW::initialize(string(pstrArgs), buf.get());
    return static_cast<VW_HANDLE>(all);
}

VW_DLL_MEMBER VW_HANDLE VW_CALLING_CONV VW_InitializeWithModelEscaped(const char * pstrArgs, const char * modelData, size_t modelDataSize)
{
  std::unique_ptr<memory_io_buf> buf(new memory_io_buf());
  buf->write_file(-1, modelData, modelDataSize);

  auto all = VW::initialize_escaped(std::string(pstrArgs), buf.get());
  return static_cast<VW_HANDLE>(all);
}

VW_DLL_MEMBER void VW_CALLING_CONV VW_CopyModelData(VW_HANDLE handle, VW_IOBUF* outputBufferHandle, char** outputData, size_t* outputSize) {
    vw* pointer = static_cast<vw*>(handle);

    memory_io_buf* buf = new(memory_io_buf);
    VW::save_predictor(*pointer, *buf);

    *outputBufferHandle = buf;
    *outputSize = buf->GetDataSize();
    *outputData = buf->GetDataPointer();
}

VW_DLL_MEMBER void VW_CALLING_CONV VW_FreeIOBuf(VW_IOBUF bufferHandle) {
    delete static_cast<memory_io_buf*>(bufferHandle);
}

}
