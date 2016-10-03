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

#include <rapidjson/reader.h>
#include <stack>
#include <vector>
#include "vw.h"
#include "v_array.h"

using namespace std;
using namespace rapidjson;

struct BaseState;

struct Namespace
{
	char feature_group;
	feature_index namespace_hash;
	features* features;
	size_t feature_count;

	void AddFeature(feature_value v, feature_index i)
	{
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
	BaseState* LabelState;
	BaseState* TextState;
	BaseState* MultiState;
	BaseState* IgnoreState;
	BaseState* ArrayState;

	Context() : key(" "), key_length(1), previous_state(nullptr)
	{
		namespace_path = v_init<Namespace>();
	}

	void PushNamespace(const char* ns)
	{
		Namespace n;
		n.feature_group = ns[0];
		n.namespace_hash = VW::hash_space(*all, ns);
		n.features = ex->feature_space + ns[0];
		n.feature_count = 0;

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

//template<typename Derived = void>
struct BaseState
{
	//typedef typename internal::SelectIf<internal::IsSame<Derived, void>, BaseState, Derived>::Type Override;

	// interface
	//BaseState<>* DispatchDefault(Context& ctx) { return static_cast<Override&>(*this).Default(ctx); }
	//BaseState<>* DispatchNull(Context& ctx) { return static_cast<Override&>(*this).Null(ctx); }
	//BaseState<>* DispatchBool(Context& ctx, bool b) { return static_cast<Override&>(*this).Bool(ctx, b); }
	//BaseState<>* DispatchFloat(Context& ctx, float v) { return static_cast<Override&>(*this).Float(ctx, v); }
	//BaseState<>* DispatchUint(Context& ctx, unsigned v) { return static_cast<Override&>(*this).Uint(ctx, v); }
	//BaseState<>* DispatchString(Context& ctx, const char* str, SizeType len, bool copy) { return static_cast<Override&>(*this).String(ctx, str, len, copy); }
	//BaseState<>* DispatchStartObject(Context& ctx) { return static_cast<Override&>(*this).StartObject(ctx); }
	//BaseState<>* DispatchKey(Context& ctx, const char* str, SizeType len, bool copy) { return static_cast<Override&>(*this).Key(ctx, str, len, copy); }
	//BaseState<>* DispatchEndObject(Context& ctx, SizeType count) { return static_cast<Override&>(*this).EndObject(ctx, count); }
	//BaseState<>* DispatchStartArray(Context& ctx) { return static_cast<Override&>(*this).StartArray(ctx); }
	//BaseState<>* DispatchEndArray(Context& ctx, SizeType count) { return static_cast<Override&>(*this).EndArray(ctx, count); }

	//BaseState<>* Default() { return nullptr; }
	//BaseState<>* Null(Context& ctx) { return static_cast<Override&>(*this).Default(); }
	//BaseState<>* Bool(Context& ctx, bool) { return static_cast<Override&>(*this).Default(); }
	//BaseState<>* Float(Context& ctx, float) { return static_cast<Override&>(*this).Default(); }
	//BaseState<>* Uint(Context& ctx, unsigned) { return static_cast<Override&>(*this).Default(); }
	//BaseState<>* String(Context& ctx, const char*, SizeType, bool) { return static_cast<Override&>(*this).Default(); }
	//BaseState<>* StartObject(Context& ctx) { return static_cast<Override&>(*this).Default(); }
	//BaseState<>* Key(Context& ctx, const char* str, SizeType len, bool copy) { return static_cast<Override&>(*this).Default(); }
	//BaseState<>* EndObject(Context& ctx, SizeType) { return static_cast<Override&>(*this).Default(); }
	//BaseState<>* StartArray(Context& ctx) { return static_cast<Override&>(*this).Default(); }
	//BaseState<>* EndArray(Context& ctx, SizeType) { return static_cast<Override&>(*this).Default(); }

	virtual BaseState* Null(Context& ctx) { return nullptr; }
	virtual BaseState* Bool(Context& ctx, bool) { return nullptr; }
	virtual BaseState* Float(Context& ctx, float) { return nullptr; }
	virtual BaseState* Uint(Context& ctx, unsigned) { return nullptr; }
	virtual BaseState* String(Context& ctx, const char*, SizeType, bool) { return nullptr; }
	virtual BaseState* StartObject(Context& ctx) { return nullptr; }
	virtual BaseState* Key(Context& ctx, const char* str, SizeType len, bool copy) { return nullptr; }
	virtual BaseState* EndObject(Context& ctx, SizeType) { return nullptr; }
	virtual BaseState* StartArray(Context& ctx) { return nullptr; }
	virtual BaseState* EndArray(Context& ctx, SizeType) { return nullptr; }
};



// "_label":"1"
struct LabelState : BaseState
{
	BaseState* String(Context& ctx, const char* str, SizeType len, bool copy) 
	{ 
		// only to be used with copy=false
		assert(!copy);

		VW::parse_example_label(*ctx.all, *ctx.ex, str);
		return ctx.previous_state;
	}
};

// "_text":"a b c" 
struct TextState : BaseState
{
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
			if (*p == ' ' || *p == '\t')
			{
				*p = '\0';
				ns.AddFeature(ctx.all, start);

				start = p + 1;
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
	BaseState* StartArray(Context& ctx) { return this; }

	BaseState* StartObject(Context& ctx) 
	{
		// allocate new example
		ctx.ex = (*ctx.example_factory)(ctx.example_factory_context);
		ctx.examples->push_back(ctx.ex);

		// setup default namespace
		ctx.PushNamespace(" ");

		return ctx.DefaultState;
	}

	BaseState* EndObject(Context& ctx, SizeType) { return this; }

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
	ArrayState() : return_state(nullptr)
	{ }

	BaseState* StartArray(Context &ctx)
	{
		if (ctx.current_state == this)
		{
			// TODO: message. recursive arrays are not supported
			return nullptr;
		}

		array_hash = ctx.CurrentNamespace().namespace_hash;
		return_state = ctx.current_state;
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
			return nullptr;
		head++;

		// scan for ,}
		// support { { ... } }
		int depth = 0;
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
				if (depth == 0)
					stop = true;
				else
					depth--;
				break;
			case ',':
				if (depth == 0)
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
	BaseState* Key(Context& ctx, const char* str, SizeType length, bool copy)
	{
		// only to be used with copy=false
		assert(!copy);

		ctx.key = str;
		ctx.key_length = length;

		if (length > 0 && str[0] == '_')
		{
			if (!strncmp(str, "_label", max(6, ctx.key_length)))
				return ctx.LabelState;

			if (!strncmp(str, "_text", max(5, ctx.key_length)))
				return ctx.TextState;

			// TODO: _multi in _multi... 
			if (!strncmp(str, "_multi", max(6, ctx.key_length)))
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

		// concat key + value
		//string_buffer.erase();
		//size_t total_length = ctx.key_length + length;

		//if (string_buffer.size() < total_length)
		//	string_buffer.resize(total_length + 1);

		//memcpy(string_buffer.begin(), ctx.key, ctx.key_length);
		//memcpy(string_buffer.begin() + ctx.key_length, str, length);
		//string_buffer[total_length] = '\0';

		char* prepend = (char*)str - ctx.key_length;
		memmove(prepend, ctx.key, ctx.key_length);

		// ctx.CurrentNamespace().AddFeature(string_buffer.begin());
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
		ctx.PushNamespace(ctx.key);
		return this;
	}

	BaseState* EndObject(Context& ctx, SizeType memberCount)
	{
		InsertNamespace(ctx);

		// return to default namespace
		ctx.namespace_path.pop();

		return this;
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
	TextState text_state;
	MultiState multi_state;
	IgnoreState ignore_state;
	ArrayState array_state;

	VWReaderHandler(vw* all, v_array<example*>* examples, InsituStringStream* stream, VW::example_factory_t example_factory, void* example_factory_context)
	{
		ctx.all = all;
		ctx.examples = examples;
		ctx.ex = (*examples)[0];

		ctx.stream = stream;
		ctx.example_factory = example_factory;
		ctx.example_factory_context = example_factory_context;

		ctx.DefaultState = &default_state;
		ctx.LabelState = &label_state;
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
};

namespace VW
{
	void read_line_json(vw& all, v_array<example*>& examples, char* line, example_factory_t example_factory, void* ex_factory_context)
	{
		// TODO: keep handler in *all to cache string buffer
		// reader can be re-used
		Reader reader;

		// destructive parsing
		InsituStringStream ss(line);

		VWReaderHandler handler(&all, &examples, &ss, example_factory, ex_factory_context);
		reader.Parse<kParseInsituFlag, InsituStringStream, VWReaderHandler>(ss, handler);
	}

	//strongly typed version
	//template<typename T, example* (*example_factory)(T)>
	//void read_line_json(vw& all, v_array<example*>& examples, char* line, T ex_factory_context)
}

