/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/

// TODO: check ifdefs...
//#define RAPIDJSON_SIMD
//#define RAPIDJSON_SSE42

#pragma once
#include "parse_example_json.h"

#include <rapidjson/error/en.h>
#include <rapidjson/reader.h>
#include <stack>
#include <vector>
#include "vw.h"
#include "v_array.h"
#include "cb.h"
#include "best_constant.h"

using namespace std;
using namespace rapidjson;

struct BaseState;
struct LabelState;
class LabelObjectState;
struct LabelSinglePropertyState;
struct LabelIndexState;
struct TextState;
struct MultiState;
struct IgnoreState;
class ArrayState;

struct Namespace
{
	char feature_group;
	feature_index namespace_hash;
	features* features;
	size_t feature_count;
	BaseState* return_state;

	void AddFeature(feature_value v, feature_index i)
	{
		// filter out 0-values
		if (v == 0)
			return;

		features->push_back(v, i);
		feature_count++;
	}

	void AddFeature(vw* all, const char* str)
	{
		features->push_back(
			1.,
			VW::hash_feature(*all, str, namespace_hash));
		feature_count++;
	}
};

struct Context
{
	vw* all;
	std::stringstream error;

	// last "<key>": encountered
	const char* key;
	SizeType key_length;

	BaseState* current_state;
	BaseState* previous_state;

	// the path of namespaces
	v_array<Namespace> namespace_path;

	v_array<example*>* examples;
	example* ex;
	InsituStringStream* stream;

	VW::example_factory_t example_factory;
	void* example_factory_context;

	// states
	BaseState* DefaultState;
	LabelState* LabelState;
	LabelObjectState* LabelObjectState;
	LabelSinglePropertyState* LabelSinglePropertyState;
	LabelIndexState* LabelIndexState;
	TextState* TextState;
	MultiState* MultiState;
	IgnoreState* IgnoreState;
	ArrayState* ArrayState;

	Context() : key(" "), key_length(1), previous_state(nullptr)
	{
		namespace_path = v_init<Namespace>();
	}

	void PushNamespace(const char* ns, BaseState* return_state)
	{
		Namespace n;
		n.feature_group = ns[0];
		n.namespace_hash = VW::hash_space(*all, ns);
		n.features = ex->feature_space + ns[0];
		n.feature_count = 0;
		n.return_state = return_state;

		namespace_path.push_back(n);
	}

	Namespace& CurrentNamespace() { return *(namespace_path._end - 1); }

	bool TransitionState(BaseState* next_state)
	{
		if (next_state == nullptr)
			return false;

		previous_state = current_state;
		current_state = next_state;

		return true;
	}
};

struct BaseState
{
	const char* name;

	BaseState(const char* pname) : name(pname)
	{ }

	virtual BaseState* Null(Context& ctx) 
	{ 
		ctx.error << "Unexpected token: null"; 
		return nullptr; 
	}

	virtual BaseState* Bool(Context& ctx, bool b) 
	{ 
		ctx.error << "Unexpected token: bool (" << (b ? "true":"false") << ")"; 
		return nullptr; 
	}

	virtual BaseState* Float(Context& ctx, float v) 
	{ 
		ctx.error << "Unexpected token: float (" << v << ")"; 
		return nullptr; 
	}

	virtual BaseState* Uint(Context& ctx, unsigned v) 
	{
		ctx.error << "Unexpected token: uint (" << v << ")";
		return nullptr; 
	}

	virtual BaseState* String(Context& ctx, const char* str, SizeType len, bool)
	{ 
		ctx.error << "Unexpected token: string('" << str << "' len: " << len <<")";
		return nullptr; 
	}
	virtual BaseState* StartObject(Context& ctx) 
	{ 
		ctx.error << "Unexpected token: {"; 
		return nullptr; 
	}

	virtual BaseState* Key(Context& ctx, const char* str, SizeType len, bool copy) 
	{ 
		ctx.error << "Unexpected token: key('" << str << "' len: " << len << ")";
		return nullptr;
	}

	virtual BaseState* EndObject(Context& ctx, SizeType) 
	{ 
		ctx.error << "Unexpected token: }"; 
		return nullptr; 
	}

	virtual BaseState* StartArray(Context& ctx) 
	{ 
		ctx.error << "Unexpected token: ["; 
		return nullptr; 
	}

	virtual BaseState* EndArray(Context& ctx, SizeType) 
	{ 
		ctx.error << "Unexpected token: ]";  
		return nullptr; 
	}
};

class LabelObjectState : public BaseState
{
private:
	BaseState* return_state;
	polylabel label;
	CB::cb_class cb_label;
	bool found;
	bool found_cb;

public:
	LabelObjectState(vw* all) : BaseState("LabelObject")
	{
		found = found_cb = false;
		all->p->lp.default_label(&label);
	}

	BaseState* StartObject(Context& ctx)
	{
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

	BaseState* Key(Context& ctx, const char* str, SizeType len, bool copy) 
	{
		ctx.key = str;
		ctx.key_length = len;
		return this;
	}
	
	BaseState* Float(Context& ctx, float v) 
	{ 
		// simple
		if (!_stricmp(ctx.key, "Label"))
		{
			label.simple.label = v;
			found = true;
		}
		else if (!_stricmp(ctx.key, "Initial"))
		{
			label.simple.initial = v;
			found = true;
		}
		else if (!_stricmp(ctx.key, "Weight"))
		{
			label.simple.weight = v;
			found = true;
		}
		// CB
		else if (!_stricmp(ctx.key, "Action"))
		{
			cb_label.action = v;
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

	BaseState* Uint(Context& ctx, unsigned v) { return Float(ctx, v); }

	BaseState* EndObject(Context& ctx, SizeType) 
	{ 
		if (found_cb)
		{
			CB::label* ld = (CB::label*)&ctx.ex->l;
			ld->costs.push_back(cb_label);

			found_cb = false;
			ctx.all->p->lp.default_label(&label);
		}
		else if (found)
		{
			ctx.ex->l = label;
			count_label(label.simple.label);

			found = false;
			ctx.all->p->lp.default_label(&label);
		}

		return return_state; 
	}
};

// "_label_*":
struct LabelSinglePropertyState : BaseState
{
	LabelSinglePropertyState() : BaseState("LabelSingleProperty")
	{ }

	// forward _label
	BaseState* Float(Context& ctx, float v)
	{
		// skip "_label_"
		ctx.key += 7;
		ctx.key_length -= 7;

		if (ctx.LabelObjectState->Float(ctx, v) == nullptr)
			return nullptr;

		return ctx.previous_state;
	}

	BaseState* Uint(Context& ctx, unsigned v)
	{
		// skip "_label_"
		ctx.key += 7;
		ctx.key_length -= 7;

		if (ctx.LabelObjectState->Uint(ctx, v) == nullptr)
			return nullptr;

		return ctx.previous_state;
	}
};

struct LabelIndexState : BaseState
{
	int index;

	LabelIndexState() : BaseState("LabelIndex"), index(-1)
	{ } 

	BaseState* Uint(Context& ctx, unsigned int v)
	{
		index = v;
		return ctx.previous_state;
	}
};

// "_label":"1"
// Note: doesn't support labelIndex
struct LabelState : BaseState
{
	LabelState() : BaseState("Label")
	{ }

	BaseState* StartObject(Context& ctx) { return ctx.LabelObjectState->StartObject(ctx); }

	BaseState* String(Context& ctx, const char* str, SizeType len, bool copy) 
	{ 
		// only to be used with copy=false
		assert(!copy);

		VW::parse_example_label(*ctx.all, *ctx.ex, str);
		return ctx.previous_state;
	}

	BaseState* Float(Context& ctx, float v)
	{
		// TODO: once we introduce label types, check here
		ctx.ex->l.simple.label = v;
		return ctx.previous_state;
	}

	BaseState* Uint(Context& ctx, unsigned v)
	{
		// TODO: once we introduce label types, check here
		ctx.ex->l.simple.label = v;
		return ctx.previous_state;
	}
};

// "_text":"a b c" 
struct TextState : BaseState
{
	TextState() : BaseState("text")
	{ }

	BaseState* String(Context& ctx, const char* str, SizeType length, bool copy)
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

// "_multi":[{...},{...},...]
struct MultiState : BaseState
{
	MultiState() : BaseState("Multi")
	{ }

	BaseState* StartArray(Context& ctx) 
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

	BaseState* StartObject(Context& ctx) 
	{
		// allocate new example
		ctx.ex = (*ctx.example_factory)(ctx.example_factory_context);
		ctx.examples->push_back(ctx.ex);

		// setup default namespace
		ctx.PushNamespace(" ", this);

		return ctx.DefaultState;
	}

	BaseState* EndArray(Context& ctx, SizeType)
	{
		// return to shared example
		ctx.ex = (*ctx.examples)[0];

		return ctx.DefaultState;
	}
};

// "...":[Numbers only]
class ArrayState : public BaseState
{
	feature_index array_hash;
	BaseState* return_state;

public:
	ArrayState() : BaseState("Array"), return_state(nullptr)
	{ }

	BaseState* StartArray(Context &ctx)
	{
		if (ctx.previous_state == this)
		{
			ctx.error << "Nested arrays are not supported";
			return nullptr;
		}

		array_hash = ctx.CurrentNamespace().namespace_hash;
		return_state = ctx.previous_state;

		return this;
	}

	BaseState* Float(Context& ctx, float f)
	{
		ctx.CurrentNamespace().AddFeature(f, array_hash);
		array_hash++;

		return this;
	}

	BaseState* Uint(Context& ctx, unsigned f) { return Float(ctx, f); }

	BaseState* EndArray(Context& ctx, SizeType elementCount) { return return_state; }
};

// only 0 is valid as DefaultState::Ignore injected that into the source stream
struct IgnoreState : BaseState
{
	IgnoreState() : BaseState("Ignore")
	{ }

	BaseState* Uint(Context& ctx, unsigned) { return ctx.previous_state; }
};

class DefaultState : public BaseState
{
private:
	v_array<char> string_buffer;

	BaseState* Ignore(Context& ctx, SizeType length)
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
		int depth = 0, sq_depth =0 ;
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

		return ctx.IgnoreState;
	}

	void InsertNamespace(Context& ctx)
	{
		auto& ns = ctx.CurrentNamespace();
		if (ns.feature_count > 0)
		{
			auto feature_group = ns.feature_group;
			// avoid duplicate insertion
			for (unsigned char ns : ctx.ex->indices)
				if (ns == feature_group)
					return;

			ctx.ex->indices.push_back(feature_group);
		}
	}

public:
	DefaultState() : BaseState("Default")
	{ }

	BaseState* Key(Context& ctx, const char* str, SizeType length, bool copy)
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
					return ctx.LabelSinglePropertyState;
				else if (ctx.key_length == 6)
					return ctx.LabelState;
				else if (!_stricmp(str, "_labelIndex"))
					return ctx.LabelIndexState;
				else
				{
					ctx.error << "Unsupported key '" << str << "' len: " << length;
					return nullptr;
				}
			}

			if (!strcmp(str, "_text"))
				return ctx.TextState;

			// TODO: _multi in _multi... 
			if (!strcmp(str, "_multi"))
				return ctx.MultiState;

			return Ignore(ctx, length);
		}

		return this;
	}

	BaseState* String(Context& ctx, const char* str, SizeType length, bool copy)
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

	virtual BaseState* Bool(Context& ctx, bool b)
	{
		if (b)
			ctx.CurrentNamespace().AddFeature(ctx.all, ctx.key);

		return this;
	}

	BaseState* StartObject(Context& ctx)
	{
		ctx.PushNamespace(ctx.key, this);
		return this;
	}

	BaseState* EndObject(Context& ctx, SizeType memberCount)
	{
		InsertNamespace(ctx);
		BaseState* return_state = ctx.namespace_path.pop().return_state;

		if (ctx.namespace_path.empty())
		{
			int label_index = ctx.LabelIndexState->index;
			// we're at the end of the example
			if (label_index >= 0)
			{
				// skip shared example
				label_index++;
				if (label_index >= ctx.examples->size())
				{
					ctx.error << "_label_index out of bounds: " << (label_index - 1) << " examples available: " << ctx.examples->size() - 1;
					return nullptr;
				}

				// apply labelIndex
				ctx.ex = (*ctx.examples)[label_index];
			}

			// inject label
			ctx.LabelObjectState->EndObject(ctx, memberCount);
		}

		return return_state;
	}

	BaseState* Float(Context& ctx, float f)
	{
		auto& ns = ctx.CurrentNamespace();
		ns.AddFeature(f, VW::hash_feature(*ctx.all, ctx.key, ns.namespace_hash));

		return this;
	}

	BaseState* Uint(Context& ctx, unsigned f) { return Float(ctx, f); }

	BaseState* StartArray(Context& ctx) {  return ctx.ArrayState->StartArray(ctx); }
};

struct VWReaderHandler : public BaseReaderHandler<UTF8<>, VWReaderHandler>
{
	Context ctx;

	DefaultState default_state;
	LabelState label_state;
	LabelObjectState label_object_state;
	LabelSinglePropertyState label_single_property_state;
	LabelIndexState label_index_state;
	TextState text_state;
	MultiState multi_state;
	IgnoreState ignore_state;
	ArrayState array_state;

	VWReaderHandler(vw* all, v_array<example*>* examples, InsituStringStream* stream, VW::example_factory_t example_factory, void* example_factory_context)
		: label_object_state(all)
	{
		ctx.all = all;
		ctx.examples = examples;
		ctx.ex = (*examples)[0];

		ctx.stream = stream;
		ctx.example_factory = example_factory;
		ctx.example_factory_context = example_factory_context;

		ctx.DefaultState = &default_state;
		ctx.LabelState = &label_state;
		ctx.LabelObjectState = &label_object_state;
		ctx.LabelSinglePropertyState = &label_single_property_state;
		ctx.LabelIndexState = &label_index_state;
		ctx.TextState = &text_state;
		ctx.MultiState = &multi_state;
		ctx.IgnoreState = &ignore_state;
		ctx.ArrayState = &array_state;

		ctx.current_state = &default_state;
	}

	// virtual dispatch to current state
	bool Bool(bool v) { return ctx.TransitionState(ctx.current_state->Bool(ctx, v)); }
	bool Int(int v) { return ctx.TransitionState(ctx.current_state->Float(ctx, v)); }
	bool Uint(unsigned v) { return ctx.TransitionState(ctx.current_state->Uint(ctx, v));}
	bool Int64(int64_t v) { return ctx.TransitionState(ctx.current_state->Float(ctx, v)); }
	bool Uint64(uint64_t v) { return ctx.TransitionState(ctx.current_state->Float(ctx, v)); }
	bool Double(double v) { return ctx.TransitionState(ctx.current_state->Float(ctx, v)); }
	bool String(const Ch* str, SizeType len, bool copy) { return ctx.TransitionState(ctx.current_state->String(ctx, str, len, copy)); }
	bool StartObject() { return ctx.TransitionState(ctx.current_state->StartObject(ctx)); }
	bool Key(const Ch* str, SizeType len, bool copy) { return ctx.TransitionState(ctx.current_state->Key(ctx, str, len, copy)); }
	bool EndObject(SizeType count) { return ctx.TransitionState(ctx.current_state->EndObject(ctx, count)); }
	bool StartArray() { return ctx.TransitionState(ctx.current_state->StartArray(ctx)); }
	bool EndArray(SizeType count) { return ctx.TransitionState(ctx.current_state->EndArray(ctx, count)); }

	bool Null() { return true; }
	bool Default() { return false; }

	// alternative to above if we want to re-use the VW float parser...
	bool RawNumber(const Ch* str, SizeType length, bool copy) { return false; }

	std::stringstream& error() { return ctx.error; }

	BaseState* current_state() { return ctx.current_state;  }
};

namespace VW
{
	void read_line_json(vw& all, v_array<example*>& examples, char* line, example_factory_t example_factory, void* ex_factory_context)
	{
		// reader can be re-used
		Reader reader;

		// destructive parsing
		InsituStringStream ss(line);

		VWReaderHandler handler(&all, &examples, &ss, example_factory, ex_factory_context);
		ParseResult result = reader.Parse<kParseInsituFlag, InsituStringStream, VWReaderHandler>(ss, handler);
		if (!result.IsError())
			return;
		
		BaseState* current_state = handler.current_state();

		THROW("JSON parser error at " << result.Offset() << ": " << GetParseError_En(result.Code()) << ". " 
			  "Handler: " << handler.error().str() <<
			  "State: " << (current_state ? current_state->name : "null"));
	}
}

