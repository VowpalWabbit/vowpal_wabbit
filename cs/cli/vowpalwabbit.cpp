// Copyright (c) by respective owners including Yahoo!, Microsoft, and
// individual contributors. All rights reserved. Released under a BSD (revised)
// license as described in the file LICENSE.

#include "vw_clr.h"
#include "vowpalwabbit.h"
#include "best_constant.h"
#include "parser.h"
#include "hash.h"
#include "vw_example.h"
#include "vw_allreduce.h"
#include "vw_builder.h"
#include "clr_io.h"
#include "lda_core.h"
#include "parse_example.h"
#include "parse_example_json.h"

using namespace System;
using namespace System::Collections::Generic;
using namespace System::Text;

namespace VW
{
VowpalWabbit::VowpalWabbit(VowpalWabbitSettings^ settings)
  : VowpalWabbitBase(settings)
{ if (settings == nullptr)
  { throw gcnew ArgumentNullException("settings");
  }

  if (settings->ParallelOptions != nullptr)
  { m_vw->all_reduce_type = AllReduceType::Thread;
    auto total = settings->ParallelOptions->MaxDegreeOfParallelism;

    if (settings->Root == nullptr)
    { m_vw->all_reduce = new AllReduceThreads(total, settings->Node);
    }
    else
    { auto parent_all_reduce = (AllReduceThreads*)settings->Root->m_vw->all_reduce;

      m_vw->all_reduce = new AllReduceThreads(parent_all_reduce, total, settings->Node);
    }
  }

  try
  { m_hasher = GetHasher();
  }
  CATCHRETHROW
}

VowpalWabbit::VowpalWabbit(String^ args)
  : VowpalWabbit(gcnew VowpalWabbitSettings(args))
{
}

void VowpalWabbit::Driver()
{ try
  { LEARNER::generic_driver(*m_vw);
  }
  CATCHRETHROW
}

void VowpalWabbit::RunMultiPass()
{ if (m_vw->numpasses > 1)
  { try
    { adjust_used_index(*m_vw);
      m_vw->do_reset_source = true;
      VW::start_parser(*m_vw);
      LEARNER::generic_driver(*m_vw);
      VW::end_parser(*m_vw);
    }
    CATCHRETHROW
  }
}

VowpalWabbitPerformanceStatistics^ VowpalWabbit::PerformanceStatistics::get()
{ // see parse_args.cc:finish(...)
  auto stats = gcnew VowpalWabbitPerformanceStatistics();

  if (m_vw->current_pass == 0)
  { stats->NumberOfExamplesPerPass = m_vw->sd->example_number;
  }
  else
  { stats->NumberOfExamplesPerPass = m_vw->sd->example_number / m_vw->current_pass;
  }

  stats->WeightedExampleSum = m_vw->sd->weighted_examples();
  stats->WeightedLabelSum = m_vw->sd->weighted_labels;

  if (m_vw->holdout_set_off)
	  if (m_vw->sd->weighted_labeled_examples > 0)
		  stats->AverageLoss = m_vw->sd->sum_loss / m_vw->sd->weighted_labeled_examples;
	  else
		  stats->AverageLoss = System::Double::NaN;
  else if ((m_vw->sd->holdout_best_loss == FLT_MAX) || (m_vw->sd->holdout_best_loss == FLT_MAX * 0.5))
	  stats->AverageLoss = System::Double::NaN;
  else
	  stats->AverageLoss = m_vw->sd->holdout_best_loss;

  float best_constant; float best_constant_loss;
  if (get_best_constant(*m_vw, best_constant, best_constant_loss))
  { stats->BestConstant = best_constant;
    if (best_constant_loss != FLT_MIN)
    { stats->BestConstantLoss = best_constant_loss;
    }
  }

  stats->TotalNumberOfFeatures = m_vw->sd->total_features;

  return stats;
}

uint64_t VowpalWabbit::HashSpace(String^ s)
{ auto newHash = m_hasher(s, 0);

#ifdef _DEBUG
  auto oldHash = HashSpaceNative(s);
  assert(newHash == oldHash);
#endif

  return (uint32_t)newHash;
}

uint64_t VowpalWabbit::HashFeature(String^ s, size_t u)
{ auto newHash = m_hasher(s, u) & m_vw->parse_mask;

#ifdef _DEBUG
  auto oldHash = HashFeatureNative(s, u);
  assert(newHash == oldHash);
#endif

  return (uint64_t)newHash;
}

uint64_t VowpalWabbit::HashSpaceNative(String^ s)
{ auto bytes = System::Text::Encoding::UTF8->GetBytes(s);
  auto handle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

  try
  { return VW::hash_space(*m_vw, reinterpret_cast<char*>(handle.AddrOfPinnedObject().ToPointer()));
  }
  CATCHRETHROW
  finally
  { handle.Free();
  }
}

uint64_t VowpalWabbit::HashFeatureNative(String^ s, uint64_t u)
{ auto bytes = System::Text::Encoding::UTF8->GetBytes(s);
  auto handle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

  try
  { return VW::hash_feature(*m_vw, reinterpret_cast<char*>(handle.AddrOfPinnedObject().ToPointer()), u);
  }
  CATCHRETHROW
  finally
  { handle.Free();
  }
}

void VowpalWabbit::Learn(List<VowpalWabbitExample^>^ examples)
{
  multi_ex ex_coll;
  try
  {
    for each (auto ex in examples)
    {
      example* pex = ex->m_example;
      ex_coll.push_back(pex);
    }

    m_vw->learn(ex_coll);

    // as this is not a ring-based example it is not freed
    as_multiline(m_vw->l)->finish_example(*m_vw, ex_coll);
  }
  CATCHRETHROW
  finally{ }
}

void VowpalWabbit::Predict(List<VowpalWabbitExample^>^ examples)
{
  multi_ex ex_coll;
  try
  {
    for each (auto ex in examples)
    {
      example* pex = ex->m_example;
      ex_coll.push_back(pex);
    }

    as_multiline(m_vw->l)->predict(ex_coll);

    // as this is not a ring-based example it is not freed
    as_multiline(m_vw->l)->finish_example(*m_vw, ex_coll);
  }
  CATCHRETHROW
    finally{ }
}

void VowpalWabbit::Learn(VowpalWabbitExample^ ex)
{
#if _DEBUG
  if (ex == nullptr)
  { throw gcnew ArgumentNullException("ex");
  }
#endif

  try
  { m_vw->learn(*ex->m_example);

    // as this is not a ring-based example it is not free'd
    as_singleline(m_vw->l)->finish_example(*m_vw, *ex->m_example);
  }
  CATCHRETHROW
}

generic<typename T> T VowpalWabbit::Learn(VowpalWabbitExample^ ex, IVowpalWabbitPredictionFactory<T>^ predictionFactory)
{
#if _DEBUG
  if (ex == nullptr)
    throw gcnew ArgumentNullException("ex");

  if (nullptr == predictionFactory)
    throw gcnew ArgumentNullException("predictionFactory");
#endif

  try
  { m_vw->learn(*ex->m_example);

    auto prediction = predictionFactory->Create(m_vw, ex->m_example);

    // as this is not a ring-based example it is not free'd
    as_singleline(m_vw->l)->finish_example(*m_vw, *ex->m_example);

    return prediction;
  }
  CATCHRETHROW
}

void VowpalWabbit::Predict(VowpalWabbitExample^ ex)
{
#if _DEBUG
  if (ex == nullptr)
    throw gcnew ArgumentNullException("ex");
#endif

  try
  { as_singleline(m_vw->l)->predict(*ex->m_example);

    // as this is not a ring-based example it is not free'd
  as_singleline(m_vw->l)->finish_example(*m_vw, *ex->m_example);
  }
  CATCHRETHROW
}

generic<typename T> T VowpalWabbit::Predict(VowpalWabbitExample^ ex, IVowpalWabbitPredictionFactory<T>^ predictionFactory)
{
#if _DEBUG
  if (ex == nullptr)
    throw gcnew ArgumentNullException("ex");
#endif

  try
  { as_singleline(m_vw->l)->predict(*ex->m_example);

    auto prediction = predictionFactory->Create(m_vw, ex->m_example);

    // as this is not a ring-based example it is not free'd
    as_singleline(m_vw->l)->finish_example(*m_vw, *ex->m_example);

    return prediction;
  }
  CATCHRETHROW
}

public ref struct ParseJsonState
{ VowpalWabbit^ vw;
  List<VowpalWabbitExample^>^ examples;
};

example& get_example_from_pool(void* v)
{ interior_ptr<ParseJsonState^> state = (interior_ptr<ParseJsonState^>)v;

  auto ex = (*state)->vw->GetOrCreateNativeExample();
  (*state)->examples->Add(ex);

  return *ex->m_example;
}

List<VowpalWabbitExample^>^ VowpalWabbit::ParseDecisionServiceJson(cli::array<Byte>^ json, int offset, int length, bool copyJson, [Out] VowpalWabbitDecisionServiceInteractionHeader^% header)
{
#if _DEBUG
	if (json == nullptr)
		throw gcnew ArgumentNullException("json");
#endif

	try
	{
		header = gcnew VowpalWabbitDecisionServiceInteractionHeader();

		ParseJsonState^ state = gcnew ParseJsonState();
		state->vw = this;
		state->examples = gcnew List<VowpalWabbitExample^>();

		try
		{
			auto ex = GetOrCreateNativeExample();
			state->examples->Add(ex);

			v_array<example*> examples = v_init<example*>();
			example* native_example = ex->m_example;
			examples.push_back(native_example);

			interior_ptr<ParseJsonState^> state_ptr = &state;

			pin_ptr<unsigned char> data = &json[0];
			data += offset;

			DecisionServiceInteraction interaction;

			if (m_vw->audit)
				VW::read_line_decision_service_json<true>(*m_vw, examples, reinterpret_cast<char*>(data), length, copyJson, get_example_from_pool, &state, &interaction);
			else
				VW::read_line_decision_service_json<false>(*m_vw, examples, reinterpret_cast<char*>(data), length, copyJson, get_example_from_pool, &state, &interaction);

			// finalize example
			VW::setup_examples(*m_vw, examples);

			// delete native array of pointers, keep examples
			examples.delete_v();

			header->EventId = gcnew String(interaction.eventId.c_str());
			header->Actions = gcnew cli::array<int>((int)interaction.actions.size());
			int index = 0;
			for (auto a : interaction.actions)
				header->Actions[index++] = (int)a;

			header->Probabilities = gcnew cli::array<float>((int)interaction.probabilities.size());
			index = 0;
			for (auto p : interaction.probabilities)
				header->Probabilities[index++] = p;

			header->ProbabilityOfDrop = interaction.probabilityOfDrop;
			header->SkipLearn = interaction.skipLearn;

			return state->examples;
		}
		catch (...)
		{
			// cleanup
			for each (auto ex in state->examples)
				delete ex;
			throw;
		}
	}
	CATCHRETHROW
}

  List<VowpalWabbitExample^>^ VowpalWabbit::ParseJson(String^ line)
  {
#if _DEBUG
	  if (line == nullptr)
		  throw gcnew ArgumentNullException("line");
#endif
	  auto bytes = System::Text::Encoding::UTF8->GetBytes(line);
	  auto valueHandle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

	  try
	  {
		  ParseJsonState^ state = gcnew ParseJsonState();
		  state->vw = this;
		  state->examples = gcnew List<VowpalWabbitExample^>();

		  try
		  {
			  auto ex = GetOrCreateNativeExample();
			  state->examples->Add(ex);

			  v_array<example*> examples = v_init<example*>();
			  example* native_example = ex->m_example;
			  examples.push_back(native_example);

			  interior_ptr<ParseJsonState^> state_ptr = &state;

			  if (m_vw->audit)
				VW::read_line_json<true>(*m_vw, examples, reinterpret_cast<char*>(valueHandle.AddrOfPinnedObject().ToPointer()), get_example_from_pool, &state);
			  else
				VW::read_line_json<false>(*m_vw, examples, reinterpret_cast<char*>(valueHandle.AddrOfPinnedObject().ToPointer()), get_example_from_pool, &state);

			  // finalize example
			  VW::setup_examples(*m_vw, examples);

			  // remember the input string for debugging purposes
			  ex->VowpalWabbitString = line;

			  return state->examples;
		  }
		  catch (...)
		  {
			  // cleanup
			  for each (auto ex in state->examples)
				  delete ex;
			  throw;
		  }
	  }
	  CATCHRETHROW
		  finally
	  {
		  valueHandle.Free();
	  }
  }

VowpalWabbitExample^ VowpalWabbit::ParseLine(String^ line)
{
#if _DEBUG
  if (line == nullptr)
    throw gcnew ArgumentNullException("line");
#endif

  auto ex = GetOrCreateNativeExample();
  auto bytes = System::Text::Encoding::UTF8->GetBytes(line);
  auto valueHandle = GCHandle::Alloc(bytes, GCHandleType::Pinned);

  try
  { try
    { VW::read_line(*m_vw, ex->m_example, reinterpret_cast<char*>(valueHandle.AddrOfPinnedObject().ToPointer()));

      // finalize example
      VW::setup_example(*m_vw, ex->m_example);

      // remember the input string for debugging purposes
      ex->VowpalWabbitString = line;

      return ex;
    }
    catch (...)
    { delete ex;
      throw;
    }
  }
  CATCHRETHROW
  finally
  { valueHandle.Free();
  }
}

void VowpalWabbit::Learn(String^ line)
{
#if _DEBUG
  if (String::IsNullOrEmpty(line))
    throw gcnew ArgumentException("lines must not be empty. For multi-line examples use Learn(IEnumerable<string>) overload.");
#endif

  VowpalWabbitExample^ example = nullptr;

  try
  { example = ParseLine(line);
    Learn(example);
  }
  finally
  { delete example;
  }
}

void VowpalWabbit::Predict(String^ line)
{
#if _DEBUG
  if (String::IsNullOrEmpty(line))
    throw gcnew ArgumentException("lines must not be empty. For multi-line examples use Predict(IEnumerable<string>) overload.");
#endif

  VowpalWabbitExample^ example = nullptr;

  try
  { example = ParseLine(line);
    Predict(example);
  }
  finally
  { delete example;
  }
}

generic<typename TPrediction> TPrediction VowpalWabbit::Learn(String^ line, IVowpalWabbitPredictionFactory<TPrediction>^ predictionFactory)
{
#if _DEBUG
  if (String::IsNullOrEmpty(line))
    throw gcnew ArgumentException("lines must not be empty. For multi-line examples use Learn(IEnumerable<string>) overload.");
#endif

  VowpalWabbitExample^ example = nullptr;

  try
  { example = ParseLine(line);
    return Learn(example, predictionFactory);
  }
  finally
  { delete example;
  }
}

generic<typename T> T VowpalWabbit::Predict(String^ line, IVowpalWabbitPredictionFactory<T>^ predictionFactory)
{
#if _DEBUG
  if (String::IsNullOrEmpty(line))
    throw gcnew ArgumentException("lines must not be empty. For multi-line examples use Learn(IEnumerable<string>) overload.");
#endif

  VowpalWabbitExample^ example = nullptr;

  try
  { example = ParseLine(line);
    return Predict(example, predictionFactory);
  }
  finally
  { delete example;
  }
}


void VowpalWabbit::CacheEmptyLine()
{
	auto empty = GetOrCreateNativeExample();
	empty->MakeEmpty(this);
	ReturnExampleToPool(empty);
}


void VowpalWabbit::Learn(IEnumerable<String^>^ lines)
{
#if _DEBUG
  if (lines == nullptr)
    throw gcnew ArgumentNullException("lines");
#endif

  auto examples = gcnew List<VowpalWabbitExample^>;

  try
  { for each (auto line in lines)
    { auto ex = ParseLine(line);
      examples->Add(ex);
    }

		// Need to add an empty line to cache file
		CacheEmptyLine();

    Learn(examples);
  }
  finally
  { for each (auto ex in examples)
    { delete ex;
    }
  }
}

void VowpalWabbit::Predict(IEnumerable<String^>^ lines)
{
#if _DEBUG
  if (lines == nullptr)
    throw gcnew ArgumentNullException("lines");
#endif

  auto examples = gcnew List<VowpalWabbitExample^>;

  try
  { for each (auto line in lines)
    { auto ex = ParseLine(line);
      examples->Add(ex);
    }

    // Need to add an empty line to cache file
    CacheEmptyLine();

    Predict(examples);
  }
  finally
  { for each (auto ex in examples)
    { delete ex;
    }
  }
}

generic<typename T> T VowpalWabbit::Learn(IEnumerable<String^>^ lines, IVowpalWabbitPredictionFactory<T>^ predictionFactory)
{
#if _DEBUG
  if (lines == nullptr)
    throw gcnew ArgumentNullException("lines");
#endif

  auto examples = gcnew List<VowpalWabbitExample^>;

  try
  { for each (auto line in lines)
    { auto ex = ParseLine(line);
      examples->Add(ex);

      Learn(ex);
    }

    auto empty = GetOrCreateNativeExample();
    examples->Add(empty);
    empty->MakeEmpty(this);
    Learn(empty);

    return examples[0]->GetPrediction(this, predictionFactory);
  }
  finally
  { for each (auto ex in examples)
    { delete ex;
    }
  }
}

generic<typename T> T VowpalWabbit::Predict(IEnumerable<String^>^ lines, IVowpalWabbitPredictionFactory<T>^ predictionFactory)
{
#if _DEBUG
  if (lines == nullptr)
    throw gcnew ArgumentNullException("lines");
#endif

  auto examples = gcnew List<VowpalWabbitExample^>;

  try
  { for each (auto line in lines)
    { auto ex = ParseLine(line);
      examples->Add(ex);

      Predict(ex);
    }

    auto empty = GetOrCreateNativeExample();
    examples->Add(empty);
    empty->MakeEmpty(this);
    Predict(empty);

    return examples[0]->GetPrediction(this, predictionFactory);
  }
  finally
  { for each (auto ex in examples)
    { delete ex;
    }
  }
}

void VowpalWabbit::EndOfPass()
{ try
  { m_vw->l->end_pass();
    sync_stats(*m_vw);
  }
  CATCHRETHROW
}

/// <summary>
/// Hashes the given value <paramref name="s"/>.
/// </summary>
/// <param name="s">String to be hashed.</param>
/// <param name="u">Hash offset.</param>
/// <returns>The resulting hash code.</returns>
//template<bool replaceSpace>
uint64_t hashall(String^ s, int offset, int count, uint64_t u)
{ // get raw bytes from string
  auto keys = gcnew cli::array<unsigned char>(Encoding::UTF8->GetMaxByteCount(count));
  int length = Encoding::UTF8->GetBytes(s, offset, count, keys, 0);

  // TOOD: benchmark and verify correctness
  //if (replaceSpace)
  //{
  //  for (int j = 0; j < length;)
  //  {
  //    var k = keys[j];
  //    if (k == ' ')
  //    {
  //      keys[j] = '_';
  //    }

  //    j++;

  //    // take care of UTF-8 multi-byte characters
  //    while (k & 0xC == 0xC)
  //    {
  //      j++;
  //      k <<= 1;
  //    }
  //  }
  //}

  uint32_t h1 = (uint32_t)u;
  uint32_t k1 = 0;

  const uint32_t c1 = 0xcc9e2d51;
  const uint32_t c2 = 0x1b873593;

  int i = 0;
  while (i <= length - 4)
  { // convert byte array to integer
    k1 = (uint32_t)(keys[i] | keys[i + 1] << 8 | keys[i + 2] << 16 | keys[i + 3] << 24);

    k1 *= c1;
    k1 = rotl32(k1, 15);
    k1 *= c2;

    h1 ^= k1;
    h1 = rotl32(h1, 13);
    h1 = h1 * 5 + 0xe6546b64;

    i += 4;
  }

  k1 = 0;
  int tail = length - length % 4;
  switch (length & 3)
  { case 3:
      k1 ^= (uint32_t)(keys[tail + 2] << 16);
    case 2:
      k1 ^= (uint32_t)(keys[tail + 1] << 8);
    case 1:
      k1 ^= (uint32_t)(keys[tail]);
      k1 *= c1;
      k1 = rotl32(k1, 15);
      k1 *= c2;
      h1 ^= k1;
      break;
  }

  // finalization
  h1 ^= (uint32_t)length;

  return MURMUR_HASH_3::fmix(h1);
}

uint64_t hashall(String^ s, uint64_t u)
{ return hashall(s, 0, s->Length, u);
}

/// <summary>
/// Hashes the given value <paramref name="s"/>.
/// </summary>
/// <param name="s">String to be hashed.</param>
/// <param name="u">Hash offset.</param>
/// <returns>The resulting hash code.</returns>
size_t hashstring(String^ s, size_t u)
{ int offset = 0;
  int end = s->Length;
  if (end == 0)
    return u;

  //trim leading whitespace but not UTF-8
  for (; offset < s->Length && s[offset] <= 0x20; offset++);
  for (; end >= offset && s[end - 1] <= 0x20; end--);

  int sInt = 0;
  for (int i = offset; i < end; i++)
  { auto c = s[i];
    if (c >= '0' && c <= '9')
      sInt = 10 * sInt + (c - '0');
    else
      return hashall(s, offset, end - offset, u);
  }

  return sInt + u;
}

Func<String^, size_t, size_t>^ VowpalWabbit::GetHasher()
{ //feature manipulation
  std::string hash_function("strings");
  if (m_vw->options->was_supplied("hash"))
  {
    hash_function = m_vw->options->get_typed_option<std::string>("hash").value();
  }

  if (hash_function == "strings")
  { return gcnew Func<String^, size_t, size_t>(&hashstring);
  }
  else if (hash_function == "all")
  { return gcnew Func<String^, size_t, size_t>(&hashall);
  }
  else
  { THROW("Unsupported hash function: " << hash_function);
  }
}

VowpalWabbit^ VowpalWabbit::Native::get()
{ return this;
}


VowpalWabbitExample^ VowpalWabbit::GetOrCreateNativeExample()
{ auto ex = m_examples->Remove();

  if (ex == nullptr)
  { try
    { auto ex = VW::alloc_examples(0, 1);
      m_vw->p->lp.default_label(&ex->l);
      return gcnew VowpalWabbitExample(this, ex);
    }
    CATCHRETHROW
  }

  try
  { VW::empty_example(*m_vw, *ex->m_example);
    m_vw->p->lp.default_label(&ex->m_example->l);

    return ex;
  }
  CATCHRETHROW
}

void VowpalWabbit::ReturnExampleToPool(VowpalWabbitExample^ ex)
{
#if _DEBUG
  if (m_vw == nullptr)
    throw gcnew ObjectDisposedException("VowpalWabbitExample was not properly disposed as the owner is already disposed");
#endif

  if (ex == nullptr)
    throw gcnew ArgumentNullException("ex");

  // make sure we're not a ring based example
  assert(!VW::is_ring_example(*m_vw, ex->m_example));

  // the bag might have reached it's limit
  if (m_examples != nullptr)
  { if (!m_examples->TryAdd(ex))
      DisposeExample(ex);
  }
#if _DEBUG
  else // this should not happen as m_vw is already set to null
    throw gcnew ObjectDisposedException("VowpalWabbitExample was disposed after the owner is disposed");
#endif
}

cli::array<List<VowpalWabbitFeature^>^>^ VowpalWabbit::GetTopicAllocation(int top)
{ uint64_t length = (uint64_t)1 << m_vw->num_bits;
  // using jagged array to enable LINQ
  auto K = (int)m_vw->lda;
  auto allocation = gcnew cli::array<List<VowpalWabbitFeature^>^>(K);

  // TODO: better way of peaking into lda?
  auto lda_rho = m_vw->options->get_typed_option<float>("lda_rho").value();

  std::vector<feature> top_weights;
  // over topics
  for (int topic = 0; topic < K; topic++)
  { get_top_weights(m_vw, top, topic, top_weights);

    auto clr_weights = gcnew List<VowpalWabbitFeature^>(top);
    allocation[topic] = clr_weights;
    for (auto& pair : top_weights)
      clr_weights->Add(gcnew VowpalWabbitFeature(this, pair.x, pair.weight_index));
  }
  return allocation;
}

template<typename T>
cli::array<cli::array<float>^>^ VowpalWabbit::FillTopicAllocation(T& weights)
{
	uint64_t length = (uint64_t)1 << m_vw->num_bits;

	// using jagged array to enable LINQ
	auto K = (int)m_vw->lda;
	auto allocation = gcnew cli::array<cli::array<float>^>(K);
	for (int k = 0; k < K; k++)
		allocation[k] = gcnew cli::array<float>((int)length);

	// TODO: better way of peaking into lda?
  auto lda_rho = m_vw->options->get_typed_option<float>("lda_rho").value();


	for (auto iter = weights.begin(); iter != weights.end(); ++iter)
	{   // over topics
		weight* wp = &(*iter);
		for (uint64_t k = 0; k < K; k++)
			allocation[(int)k][(int)iter.index()] = wp[k] + lda_rho;
	}

	return allocation;
}

cli::array<cli::array<float>^>^  VowpalWabbit::GetTopicAllocation()
{
	// over weights
	if (m_vw->weights.sparse)
		return FillTopicAllocation(m_vw->weights.sparse_weights);
	else
		return FillTopicAllocation(m_vw->weights.dense_weights);
  }
}
