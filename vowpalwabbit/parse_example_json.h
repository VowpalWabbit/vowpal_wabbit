/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/

#pragma once

#include "parse_primitives.h"
#include "v_array.h"

// seems to help with skipping spaces
//#define RAPIDJSON_SIMD
//#define RAPIDJSON_SSE42

#include <rapidjson/reader.h>
#include <rapidjson/error/en.h>
#include "cb.h"
#include "best_constant.h"
#include <boost/algorithm/string.hpp>

// portability fun
#ifndef _WIN32
#define _stricmp strcasecmp
#endif

using namespace std;
using namespace rapidjson;

struct vw;

template<bool audit>
struct BaseState;

template<bool audit>
struct Context;

template<bool audit>
struct Namespace
{
	char feature_group;
	feature_index namespace_hash;
	features* ftrs;
	size_t feature_count;
	BaseState<audit>* return_state;
	const char* name;

	void AddFeature(feature_value v, feature_index i, const char* feature_name)
	{
		// filter out 0-values
		if (v == 0)
			return;

		ftrs->push_back(v, i);
		feature_count++;

		if (audit)
			ftrs->space_names.push_back(audit_strings_ptr(new audit_strings(name, feature_name)));
	}

	void AddFeature(vw* all, const char* str)
	{
		ftrs->push_back(
			1.,
			VW::hash_feature(*all, str, namespace_hash));
		feature_count++;

		if (audit)
			ftrs->space_names.push_back(audit_strings_ptr(new audit_strings(name, str)));
	}
};

template<bool audit>
struct BaseState
{
	const char* name;

	BaseState(const char* pname) : name(pname)
	{ }

	virtual BaseState<audit>* Null(Context<audit>& ctx)
	{
		// ignore Null by default and stay in the current state
		return ctx.previous_state == nullptr ? this : ctx.previous_state;
	}

	virtual BaseState<audit>* Bool(Context<audit>& ctx, bool b)
	{
		ctx.error << "Unexpected token: bool (" << (b ? "true" : "false") << ")";
		return nullptr;
	}

	virtual BaseState<audit>* Float(Context<audit>& ctx, float v)
	{
		ctx.error << "Unexpected token: float (" << v << ")";
		return nullptr;
	}

	virtual BaseState<audit>* Uint(Context<audit>& ctx, unsigned v)
	{
		ctx.error << "Unexpected token: uint (" << v << ")";
		return nullptr;
	}

	virtual BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType len, bool)
	{
		ctx.error << "Unexpected token: string('" << str << "' len: " << len << ")";
		return nullptr;
	}

	virtual BaseState<audit>* StartObject(Context<audit>& ctx)
	{
		ctx.error << "Unexpected token: {";
		return nullptr;
	}

	virtual BaseState<audit>* Key(Context<audit>& ctx, const char* str, rapidjson::SizeType len, bool copy)
	{
		ctx.error << "Unexpected token: key('" << str << "' len: " << len << ")";
		return nullptr;
	}

	virtual BaseState<audit>* EndObject(Context<audit>& ctx, rapidjson::SizeType)
	{
		ctx.error << "Unexpected token: }";
		return nullptr;
	}

	virtual BaseState<audit>* StartArray(Context<audit>& ctx)
	{
		ctx.error << "Unexpected token: [";
		return nullptr;
	}

	virtual BaseState<audit>* EndArray(Context<audit>& ctx, rapidjson::SizeType)
	{
		ctx.error << "Unexpected token: ]";
		return nullptr;
	}
};

template<bool audit>
class LabelObjectState : public BaseState<audit>
{
private:
	BaseState<audit>* return_state;
	CB::cb_class cb_label;
	bool found;
	bool found_cb;

public:
	LabelObjectState() : BaseState<audit>("LabelObject")
	{
	}

	void init(vw* all)
	{
		found = found_cb = false;

		cb_label = { 0.,0,0.,0. };
	}

	BaseState<audit>* StartObject(Context<audit>& ctx)
	{
		ctx.all->p->lp.default_label(&ctx.ex->l);

		// don't allow { { { } } }
		if (ctx.previous_state == this)
		{
			ctx.error << "invalid label object. nested objected.";
			return nullptr;
		}

		// keep previous state
		return_state = ctx.previous_state;

		return this;
	}

	BaseState<audit>* Key(Context<audit>& ctx, const char* str, rapidjson::SizeType len, bool copy)
	{
		ctx.key = str;
		ctx.key_length = len;
		return this;
	}

	BaseState<audit>* Float(Context<audit>& ctx, float v)
	{
		// simple
		if (!_stricmp(ctx.key, "Label"))
		{
			ctx.ex->l.simple.label = v;
			found = true;
		}
		else if (!_stricmp(ctx.key, "Initial"))
		{
			ctx.ex->l.simple.initial = v;
			found = true;
		}
		else if (!_stricmp(ctx.key, "Weight"))
		{
			ctx.ex->l.simple.weight = v;
			found = true;
		}
		// CB
		else if (!_stricmp(ctx.key, "Action"))
		{
			cb_label.action = (uint32_t)v;
			found_cb = true;
		}
		else if (!_stricmp(ctx.key, "Cost"))
		{
			cb_label.cost = v;
			found_cb = true;
		}
		else if (!_stricmp(ctx.key, "Probability"))
		{
			cb_label.probability = v;
			found_cb = true;
		}
		else
		{
			ctx.error << "Unsupported label property: '" << ctx.key << "' len: " << ctx.key_length;
			return nullptr;
		}

		return this;
	}

	BaseState<audit>* Uint(Context<audit>& ctx, unsigned v)
	{
		return Float(ctx, (float)v);
	}

	BaseState<audit>* EndObject(Context<audit>& ctx, rapidjson::SizeType)
	{
		if (found_cb)
		{
			CB::label* ld = (CB::label*)&ctx.ex->l;
			ld->costs.push_back(cb_label);

			found_cb = false;
			cb_label = { 0.,0,0.,0. };
		}
		else if (found)
		{
			count_label(ctx.all->sd, ctx.ex->l.simple.label);

			found = false;
		}

		return return_state;
	}
};

// "_label_*":
template<bool audit>
struct LabelSinglePropertyState : BaseState<audit>
{
	LabelSinglePropertyState() : BaseState<audit>("LabelSingleProperty")
	{ }

	// forward _label
	BaseState<audit>* Float(Context<audit>& ctx, float v)
	{
		// skip "_label_"
		ctx.key += 7;
		ctx.key_length -= 7;

		if (ctx.label_object_state.Float(ctx, v) == nullptr)
			return nullptr;

		return ctx.previous_state;
	}

	BaseState<audit>* Uint(Context<audit>& ctx, unsigned v)
	{
		// skip "_label_"
		ctx.key += 7;
		ctx.key_length -= 7;

		if (ctx.label_object_state.Uint(ctx, v) == nullptr)
			return nullptr;

		return ctx.previous_state;
	}
};

template<bool audit>
struct LabelIndexState : BaseState<audit>
{
	int index;

	LabelIndexState() : BaseState<audit>("LabelIndex"), index(-1)
	{ }

	BaseState<audit>* Uint(Context<audit>& ctx, unsigned int v)
	{
		index = v;
		return ctx.previous_state;
	}
};

// "_label":"1"
// Note: doesn't support labelIndex
template<bool audit>
struct LabelState : BaseState<audit>
{
	LabelState() : BaseState<audit>("Label")
	{ }

	BaseState<audit>* StartObject(Context<audit>& ctx)
	{
		return ctx.label_object_state.StartObject(ctx);
	}

	BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType len, bool copy)
	{
		// only to be used with copy=false
		assert(!copy);

		VW::parse_example_label(*ctx.all, *ctx.ex, str);
		return ctx.previous_state;
	}

	BaseState<audit>* Float(Context<audit>& ctx, float v)
	{
		// TODO: once we introduce label types, check here
		ctx.ex->l.simple.label = v;
		return ctx.previous_state;
	}

	BaseState<audit>* Uint(Context<audit>& ctx, unsigned v)
	{
		// TODO: once we introduce label types, check here
		ctx.ex->l.simple.label = (float)v;
		return ctx.previous_state;
	}
};

// "_text":"a b c"
template<bool audit>
struct TextState : BaseState<audit>
{
	TextState() : BaseState<audit>("text")
	{ }

	BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType length, bool copy)
	{
		// only to be used with copy=false
		assert(!copy);

		auto& ns = ctx.CurrentNamespace();

		// split into individual features
		const char* start = str;
		const char* end = str + length;
		for (char* p = (char*)str;p != end;p++)
		{
			switch (*p)
			{
				// split on space and tab
			case ' ':
			case '\t':
				*p = '\0';
				if (p - start > 0)
					ns.AddFeature(ctx.all, start);

				start = p + 1;
				break;
				// escape chars
			case ':':
			case '|':
				*p = '_';
				break;
			}
		}

		if (start < end)
			ns.AddFeature(ctx.all, start);

		return ctx.previous_state;
	}
};

template<bool audit>
struct TagState : BaseState<audit>
{
	// "_tag":"abc"
	TagState() : BaseState<audit>("tag")
	{ }

	BaseState<audit>* String(Context<audit>& ctx, const char* str, SizeType length, bool copy)
	{
		// only to be used with copy=false
		assert(!copy);

		push_many(ctx.ex->tag, str, length);

		return ctx.previous_state;
	}
};

template<bool audit>
struct MultiState : BaseState<audit>
{
	MultiState() : BaseState<audit>("Multi")
	{ }

	BaseState<audit>* StartArray(Context<audit>& ctx)
	{
		// mark shared example
		// TODO: how to check if we're in CB mode (--cb?)
		// if (ctx.all->p->lp == CB::cb_label) // not sure how to compare
		{
			CB::label* ld = (CB::label*)&ctx.ex->l;
			CB::cb_class f;

			f.partial_prediction = 0.;
			f.action = (uint32_t)uniform_hash("shared", 6, 0);
			f.cost = FLT_MAX;
			f.probability = -1.f;

			ld->costs.push_back(f);
		}

		return this;
	}

	BaseState<audit>* StartObject(Context<audit>& ctx)
	{
		// allocate new example
		ctx.ex = &(*ctx.example_factory)(ctx.example_factory_context);
		ctx.all->p->lp.default_label(&ctx.ex->l);
		ctx.examples->push_back(ctx.ex);

		// setup default namespace
		ctx.PushNamespace(" ", this);

		return &ctx.default_state;
	}

	BaseState<audit>* EndArray(Context<audit>& ctx, rapidjson::SizeType)
	{
		// return to shared example
		ctx.ex = (*ctx.examples)[0];

		return &ctx.default_state;
	}
};

// "...":[Numbers only]
template<bool audit>
class ArrayState : public BaseState<audit>
{
	feature_index array_hash;

public:
	ArrayState() : BaseState<audit>("Array")
	{ }

	BaseState<audit>* StartArray(Context<audit>& ctx)
	{
		if (ctx.previous_state == this)
		{
			ctx.error << "Nested arrays are not supported";
			return nullptr;
		}

		ctx.PushNamespace(ctx.key, ctx.previous_state);

		array_hash = ctx.CurrentNamespace().namespace_hash;

		return this;
	}

	BaseState<audit>* Float(Context<audit>& ctx, float f)
	{
		if (audit)
		{
			stringstream str;
			str << '[' << (array_hash - ctx.CurrentNamespace().namespace_hash) << ']';

			ctx.CurrentNamespace().AddFeature(f, array_hash, str.str().c_str());
		}
		else
			ctx.CurrentNamespace().AddFeature(f, array_hash, nullptr);
		array_hash++;

		return this;
	}

	BaseState<audit>* Uint(Context<audit>& ctx, unsigned f)
	{
		return Float(ctx, (float)f);
	}

	BaseState<audit>* Null(Context<audit>& ctx)
	{
		// ignore null values and stay in current state
		return this;
	}

	BaseState<audit>* StartObject(Context<audit>& ctx)
	{
		// parse properties
		ctx.PushNamespace(ctx.namespace_path.size() > 0 ? ctx.CurrentNamespace().name : " ", this);

		return &ctx.default_state;
	}

	BaseState<audit>* EndArray(Context<audit>& ctx, rapidjson::SizeType elementCount)
	{
		return ctx.PopNamespace();
	}
};

// only 0 is valid as DefaultState::Ignore injected that into the source stream
template<bool audit>
struct IgnoreState : BaseState<audit>
{
	IgnoreState() : BaseState<audit>("Ignore")
	{ }

	BaseState<audit>* Uint(Context<audit>& ctx, unsigned)
	{
		return ctx.previous_state;
	}
};

template<bool audit>
class DefaultState : public BaseState<audit>
{
public:
	DefaultState() : BaseState<audit>("Default")
	{ }

	BaseState<audit>* Ignore(Context<audit>& ctx, rapidjson::SizeType length)
	{
		// fast ignore
		// skip key + \0 + "
		char* head = ctx.stream->src_ + length + 2;

		if (*head != ':')
		{
			ctx.error << "Expected ':' found '" << *head << "'";
			return nullptr;
		}
		head++;

		// scan for ,}
		// support { { ... } }
		int depth = 0, sq_depth = 0;
		bool stop = false;
		while (!stop)
		{
			switch (*head)
			{
			case '"':
			{
				// skip strings
				bool stopInner = false;
				while (!stopInner)
				{
					head++;
					switch (*head)
					{
					case '\\':
						head++;
						break;
					case '"':
						stopInner = true;
						break;
					}
				}
				break;
			}
			case '{':
				depth++;
				break;
			case '}':
				if (depth == 0 && sq_depth == 0)
					stop = true;
				else
					depth--;
				break;
			case '[':
				sq_depth++;
				break;
			case ']':
				if (depth == 0 && sq_depth == 0)
					stop = true;
				else
					sq_depth--;
				break;
			case ',':
				if (depth == 0 && sq_depth == 0)
					stop = true;
				break;
			}
			head++;
		}

		// skip key + \0 + ":
		char* value = ctx.stream->src_ + length + 3;
		*value = '0';
		value++;
		memset(value, ' ', head - value - 1);

		return &ctx.ignore_state;
	}

	BaseState<audit>* Key(Context<audit>& ctx, const char* str, rapidjson::SizeType length, bool copy)
	{
		// only to be used with copy=false
		assert(!copy);

		ctx.key = str;
		ctx.key_length = length;

		if (length > 0 && str[0] == '_')
		{
			// match _label*
			if (ctx.key_length >= 6 && !strncmp(str, "_label", 6))
			{
				if (ctx.key_length >= 7 && ctx.key[6] == '_')
					return &ctx.label_single_property_state;
				else if (ctx.key_length == 6)
					return &ctx.label_state;
				else if (ctx.key_length == 11 && !_stricmp(str, "_labelIndex"))
					return &ctx.label_index_state;
				else
				{
					ctx.error << "Unsupported key '" << str << "' len: " << length;
					return nullptr;
				}
			}

			if (ctx.key_length == 5 && !strcmp(str, "_text"))
				return &ctx.text_state;

			// TODO: _multi in _multi...
			if (ctx.key_length == 6 && !strcmp(str, "_multi"))
				return &ctx.multi_state;

			if (ctx.key_length == 4 && !_stricmp(ctx.key, "_tag"))
				return &ctx.tag_state;

			return Ignore(ctx, length);
		}

		return this;
	}

	BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType length, bool copy)
	{
		assert(!copy);

		// string escape
		const char* end = str + length;
		for (char* p = (char*)str;p != end;p++)
		{
			switch (*p)
			{
			case ' ':
			case '\t':
			case '|':
			case ':':
				*p = '_';
			}
		}

		char* prepend = (char*)str - ctx.key_length;
		memmove(prepend, ctx.key, ctx.key_length);

		ctx.CurrentNamespace().AddFeature(ctx.all, prepend);

		return this;
	}

	BaseState<audit>* Bool(Context<audit>& ctx, bool b)
	{
		if (b)
			ctx.CurrentNamespace().AddFeature(ctx.all, ctx.key);

		return this;
	}

	BaseState<audit>* StartObject(Context<audit>& ctx)
	{
		ctx.PushNamespace(ctx.key, this);
		return this;
	}

	BaseState<audit>* EndObject(Context<audit>& ctx, rapidjson::SizeType memberCount)
	{
		BaseState<audit>* return_state = ctx.PopNamespace();

		if (ctx.namespace_path.empty())
		{
			int label_index = ctx.label_index_state.index;
			// we're at the end of the example
			if (label_index >= 0)
			{
				// skip shared example
				label_index++;
				if (label_index >= (int)ctx.examples->size())
				{
					ctx.error << "_label_index out of bounds: " << (label_index - 1) << " examples available: " << ctx.examples->size() - 1;
					return nullptr;
				}

				// apply labelIndex
				ctx.ex = (*ctx.examples)[label_index];

				// reset for next example
				ctx.label_index_state.index = -1;
			}

			// inject label
			ctx.label_object_state.EndObject(ctx, memberCount);
		}

		// if we're at the top-level go back to ds_state
		return ctx.namespace_path.empty() ? ctx.root_state : return_state;
	}

	BaseState<audit>* Float(Context<audit>& ctx, float f)
	{
		auto& ns = ctx.CurrentNamespace();
		ns.AddFeature(f, VW::hash_feature(*ctx.all, ctx.key, ns.namespace_hash), ctx.key);

		return this;
	}

	BaseState<audit>* Uint(Context<audit>& ctx, unsigned f)
	{
		return Float(ctx, (float)f);
	}

	BaseState<audit>* StartArray(Context<audit>& ctx)
	{
		return ctx.array_state.StartArray(ctx);
	}
};

template<bool audit, typename T>
class ArrayToVectorState : public BaseState<audit>
{
public:
  ArrayToVectorState() : BaseState<audit>("ArrayToVectorState")
  { }

  std::vector<T>* output_array;

  BaseState<audit>* StartArray(Context<audit>& ctx)
  {
    if (ctx.previous_state == this)
    {
      ctx.error << "Nested arrays are not supported";
      return nullptr;
    }

    return this;
  }

  BaseState<audit>* Uint(Context<audit>& ctx, unsigned f)
  {
    output_array->push_back((T)f);
    return this;
  }

  BaseState<audit>* Float(Context<audit>& ctx, float f)
  {
    output_array->push_back((T)f);
    return this;
  }

  BaseState<audit>* Null(Context<audit>& ctx)
  {
	  // ignore null values and stay in current state
	  return this;
  }

  BaseState<audit>* EndArray(Context<audit>& ctx, rapidjson::SizeType)
  {
    // TODO: introduce return_state
    return &ctx.decision_service_state;
  }
};

template<bool audit>
class StringToStringState : public BaseState<audit>
{
public:
  StringToStringState() : BaseState<audit>("StringToStringState")
  { }

  std::string* output_string;

  BaseState<audit>* String(Context<audit>& ctx, const char* str, rapidjson::SizeType length, bool copy)
  {
    output_string->assign(str, str + length);

    // TODO: introduce return_state
    return &ctx.decision_service_state;
  }

  BaseState<audit>* Null(Context<audit>& ctx)
  {
	  // ignore null values and stay in current state
	  // TODO: introduce return_state
	  return &ctx.decision_service_state;
  }
};

template<bool audit>
class FloatToFloatState : public BaseState<audit>
{
public:
  FloatToFloatState() : BaseState<audit>("FloatToFloatState")
  { }

  float* output_float;

  BaseState<audit>* Float(Context<audit>& ctx, float f)
  {
    *output_float = f;

    // TODO: introduce return_state
    return &ctx.decision_service_state;
  }

  BaseState<audit>* Null(Context<audit>& ctx)
  {
	  *output_float = 0.f;

	  // TODO: introduce return_state
	  return &ctx.decision_service_state;
  }
};

// Decision Service JSON header information - required to construct final label
struct DecisionServiceInteraction
{
	std::string eventId;

	std::vector<unsigned> actions;

	std::vector<float> probabilities;

	float probabilityOfDrop;

	DecisionServiceInteraction() : probabilityOfDrop(0.f)
	{ }
};

template<bool audit>
class DecisionServiceState : public BaseState<audit>
{

public:
  DecisionServiceState() : BaseState<audit>("DecisionService")
  { }

  DecisionServiceInteraction* data;

  BaseState<audit>* StartObject(Context<audit>& ctx)
  {
	  // TODO: improve validation
	  return this;
  }

  BaseState<audit>* EndObject(Context<audit>& ctx, rapidjson::SizeType memberCount)
  {
	  // TODO: improve validation
	  return this;
  }

  BaseState<audit>* Key(Context<audit>& ctx, const char* str, rapidjson::SizeType length, bool copy)
  {
    if (length == 1)
    {
      switch (str[0])
      {
      case 'a':
        ctx.array_uint_state.output_array = &data->actions;
        return &ctx.array_uint_state;
      case 'p':
        ctx.array_float_state.output_array = &data->probabilities;
        return &ctx.array_float_state;
      case 'c':
		ctx.key = " ";
		ctx.key_length = 1;
        return &ctx.default_state;
      }
    }
    else if (length == 5 && !strcmp(str, "pdrop"))
    {
      ctx.float_state.output_float = &data->probabilityOfDrop;
      return &ctx.float_state;
    }
    else if(length == 7 && !strcmp(str, "EventId"))
    {
      ctx.string_state.output_string = &data->eventId;
      return &ctx.string_state;
    }
	else if (length > 0 && str[0] == '_')
	{
		// match _label*
		if (length >= 6 && !strncmp(str, "_label", 6))
		{
			ctx.key = str;
			ctx.key_length = length;
			if (length >= 7 && ctx.key[6] == '_')
				return &ctx.label_single_property_state;
			else if (length == 6)
				return &ctx.label_state;
			else if (length == 11 && !_stricmp(str, "_labelIndex"))
				return &ctx.label_index_state;
		}
	}

	// ignore unknown properties
	return ctx.default_state.Ignore(ctx, length);
  }
};

template<bool audit>
struct Context
{ vw* all;
  std::stringstream error;

  // last "<key>": encountered
  const char* key;
  rapidjson::SizeType key_length;

  BaseState<audit>* current_state;
  BaseState<audit>* previous_state;

  // the path of namespaces
  v_array<Namespace<audit>> namespace_path;

  v_array<example*>* examples;
  example* ex;
  rapidjson::InsituStringStream* stream;

  VW::example_factory_t example_factory;
  void* example_factory_context;

	// states
	DefaultState<audit> default_state;
	LabelState<audit> label_state;
	LabelObjectState<audit> label_object_state;
	LabelSinglePropertyState<audit> label_single_property_state;
	LabelIndexState<audit> label_index_state;
	TextState<audit> text_state;
	TagState<audit> tag_state;
	MultiState<audit> multi_state;
	IgnoreState<audit> ignore_state;
	ArrayState<audit> array_state;

    // DecisionServiceState
    DecisionServiceState<audit> decision_service_state;
    ArrayToVectorState<audit, float> array_float_state;
    ArrayToVectorState<audit, unsigned> array_uint_state;
    StringToStringState<audit> string_state;
    FloatToFloatState<audit> float_state;

	BaseState<audit>* root_state;

	Context()
	{
		namespace_path = v_init<Namespace<audit>>();
		current_state = root_state = &default_state;
	}

	~Context()
	{
		namespace_path.delete_v();
	}

	void init(vw* pall)
	{
		all = pall;
		key = " ";
		key_length = 1;
		previous_state = nullptr;
		label_object_state.init(pall);
	}

    void SetStartStateToDecisionService(DecisionServiceInteraction* data)
    {
		decision_service_state.data = data;
		current_state = root_state = &decision_service_state;
    }

	void PushNamespace(const char* ns, BaseState<audit>* return_state)
	{
		Namespace<audit> n;
		n.feature_group = ns[0];
		n.namespace_hash = VW::hash_space(*all, ns);
		n.ftrs = ex->feature_space + ns[0];
		n.feature_count = 0;
		n.return_state = return_state;

		n.name = ns;

		namespace_path.push_back(n);
	}

	BaseState<audit>* PopNamespace()
	{
		auto& ns = CurrentNamespace();
		if (ns.feature_count > 0)
		{
			auto feature_group = ns.feature_group;
			// avoid duplicate insertion
			for (unsigned char ns : ex->indices)
				if (ns == feature_group)
					goto done;

			ex->indices.push_back(feature_group);
		}

	done:
		return namespace_path.pop().return_state;
	}

	Namespace<audit>& CurrentNamespace()
	{
		return *(namespace_path._end - 1);
	}

	bool TransitionState(BaseState<audit>* next_state)
	{
		if (next_state == nullptr)
			return false;

		previous_state = current_state;
		current_state = next_state;

		return true;
	}
};

template<bool audit>
struct VWReaderHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, VWReaderHandler<audit>>
{
	Context<audit> ctx;

	void init(vw* all, v_array<example*>* examples, rapidjson::InsituStringStream* stream, VW::example_factory_t example_factory, void* example_factory_context)
	{
		ctx.init(all);
		ctx.examples = examples;
		ctx.ex = (*examples)[0];
		all->p->lp.default_label(&ctx.ex->l);

		ctx.stream = stream;
		ctx.example_factory = example_factory;
		ctx.example_factory_context = example_factory_context;
	}

	// virtual dispatch to current state
	bool Bool(bool v) { return ctx.TransitionState(ctx.current_state->Bool(ctx, v)); }
	bool Int(int v) { return ctx.TransitionState(ctx.current_state->Float(ctx, (float)v)); }
	bool Uint(unsigned v) { return ctx.TransitionState(ctx.current_state->Uint(ctx, v)); }
	bool Int64(int64_t v) { return ctx.TransitionState(ctx.current_state->Float(ctx, (float)v)); }
	bool Uint64(uint64_t v) { return ctx.TransitionState(ctx.current_state->Float(ctx, (float)v)); }
	bool Double(double v) { return ctx.TransitionState(ctx.current_state->Float(ctx, (float)v)); }
	bool String(const char* str, SizeType len, bool copy) { return ctx.TransitionState(ctx.current_state->String(ctx, str, len, copy)); }
	bool StartObject() { return ctx.TransitionState(ctx.current_state->StartObject(ctx)); }
	bool Key(const char* str, SizeType len, bool copy) { return ctx.TransitionState(ctx.current_state->Key(ctx, str, len, copy)); }
	bool EndObject(SizeType count) { return ctx.TransitionState(ctx.current_state->EndObject(ctx, count)); }
	bool StartArray() { return ctx.TransitionState(ctx.current_state->StartArray(ctx)); }
	bool EndArray(SizeType count) { return ctx.TransitionState(ctx.current_state->EndArray(ctx, count)); }
	bool Null() { return ctx.TransitionState(ctx.current_state->Null(ctx)); }

	bool VWReaderHandlerNull() { return true; }
	bool VWReaderHandlerDefault() { return false; }

	// alternative to above if we want to re-use the VW float parser...
	bool RawNumber(const char* str, rapidjson::SizeType length, bool copy) { return false; }

	std::stringstream& error() { return ctx.error; }

	BaseState<audit>* current_state() { return ctx.current_state; }
};

template<bool audit>
struct json_parser
{
	rapidjson::Reader reader;
	VWReaderHandler<audit> handler;
};

namespace VW
{
	template<bool audit>
	void read_line_json(vw& all, v_array<example*>& examples, char* line, example_factory_t example_factory, void* ex_factory_context)
	{
		// string line_copy(line);
		// destructive parsing
		InsituStringStream ss(line);
		json_parser<audit>* parser = (json_parser<audit>*)all.p->jsonp;

		VWReaderHandler<audit>& handler = parser->handler;
		handler.init(&all, &examples, &ss, example_factory, ex_factory_context);

		ParseResult result = parser->reader.template Parse<kParseInsituFlag, InsituStringStream, VWReaderHandler<audit>>(ss, handler);
		if (!result.IsError())
			return;

		BaseState<audit>* current_state = handler.current_state();

		THROW("JSON parser error at " << result.Offset() << ": " << GetParseError_En(result.Code()) << ". "
			"Handler: " << handler.error().str() <<
			"State: " << (current_state ? current_state->name : "null")); // <<
			// "Line: '"<< line_copy << "'");
	}

    template<bool audit>
    void read_line_decision_service_json(vw& all, v_array<example*>& examples, char* line, size_t length, bool copy_line, example_factory_t example_factory, void* ex_factory_context, DecisionServiceInteraction* data)
    {
		std::vector<char> line_vec;
		if (copy_line)
		{
			line_vec.reserve(length);
			memcpy(&line_vec[0], line, length);
			line = &line_vec[0];
		}

      InsituStringStream ss(line);
      json_parser<audit> parser;

      VWReaderHandler<audit>& handler = parser.handler;
      handler.init(&all, &examples, &ss, example_factory, ex_factory_context);
      handler.ctx.SetStartStateToDecisionService(data);

      ParseResult result = parser.reader.template Parse<kParseInsituFlag, InsituStringStream, VWReaderHandler<audit>>(ss, handler);
      if (!result.IsError())
        return;

      BaseState<audit>* current_state = handler.current_state();

      THROW("JSON parser error at " << result.Offset() << ": " << GetParseError_En(result.Code()) << ". "
        "Handler: " << handler.error().str() <<
        "State: " << (current_state ? current_state->name : "null"));
    }
}

template<bool audit>
int read_features_json(vw* all, v_array<example*>& examples)
{
	char* line;
	size_t num_chars;
	size_t num_chars_initial = read_features(all, line, num_chars);
	if (num_chars_initial < 1)
		return (int)num_chars_initial;

	line[num_chars] = '\0';
	if (all->p->decision_service_json)
	{
		DecisionServiceInteraction interaction;
		VW::template read_line_decision_service_json<audit>(*all, examples, line, num_chars, false, reinterpret_cast<VW::example_factory_t>(&VW::get_unused_example), all, &interaction);
	}
	else
		VW::template read_line_json<audit>(*all, examples, line, reinterpret_cast<VW::example_factory_t>(&VW::get_unused_example), all);

	// note: the json parser does single pass parsing and cannot determine if a shared example is needed.
	// since the communication between the parsing thread the main learner expects examples to be requested in order (as they're layed out in memory)
	// there is no way to determine upfront if a shared example exists
	// thus even if there are no features for the shared example, still an empty example is returned.

	if (examples.size() > 1)
	{ // insert new line example at the end
		example& ae = VW::get_unused_example(all);
		char empty = '\0';
		substring example = { &empty, &empty };
		substring_to_example(all, &ae, example);

		examples.push_back(&ae);
	}

	return 1;
}
