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

struct vw;
struct BaseState;
struct Context;

struct Namespace
{
	char feature_group;
	feature_index namespace_hash;
	features* ftrs;
	size_t feature_count;
	BaseState* return_state;

	void AddFeature(feature_value v, feature_index i);

	void AddFeature(vw* all, const char* str);
};

struct BaseState
{
	const char* name;

	BaseState(const char* pname);

	virtual BaseState* Null(Context& ctx);
	virtual BaseState* Bool(Context& ctx, bool b);
	virtual BaseState* Float(Context& ctx, float v);
	virtual BaseState* Uint(Context& ctx, unsigned v);
	virtual BaseState* String(Context& ctx, const char* str, rapidjson::SizeType len, bool);
	virtual BaseState* StartObject(Context& ctx);
	virtual BaseState* Key(Context& ctx, const char* str, rapidjson::SizeType len, bool copy);
	virtual BaseState* EndObject(Context& ctx, rapidjson::SizeType);
	virtual BaseState* StartArray(Context& ctx);
	virtual BaseState* EndArray(Context& ctx, rapidjson::SizeType);
};

class LabelObjectState : public BaseState
{
private:
	BaseState* return_state;
	CB::cb_class cb_label;
	bool found;
	bool found_cb;

public:
	LabelObjectState();

	void init(vw* all);

	BaseState* StartObject(Context& ctx);
	BaseState* Key(Context& ctx, const char* str, rapidjson::SizeType len, bool copy);
	BaseState* Float(Context& ctx, float v);
	BaseState* Uint(Context& ctx, unsigned v);
	BaseState* EndObject(Context& ctx, rapidjson::SizeType);
};

// "_label_*":
struct LabelSinglePropertyState : BaseState
{
	LabelSinglePropertyState();

	// forward _label
	BaseState* Float(Context& ctx, float v);

	BaseState* Uint(Context& ctx, unsigned v);
};

struct LabelIndexState : BaseState
{
	int index;

	LabelIndexState();

	BaseState* Uint(Context& ctx, unsigned int v);
};

// "_label":"1"
// Note: doesn't support labelIndex
struct LabelState : BaseState
{
	LabelState();

	BaseState* StartObject(Context& ctx);
	BaseState* String(Context& ctx, const char* str, rapidjson::SizeType len, bool copy);
	BaseState* Float(Context& ctx, float v);
	BaseState* Uint(Context& ctx, unsigned v);
};

struct TextState : BaseState
{
	TextState();

	BaseState* String(Context& ctx, const char* str, rapidjson::SizeType length, bool copy);
};

struct TagState : BaseState
{
	TagState();

	BaseState* String(Context& ctx, const char* str, rapidjson::SizeType length, bool copy);
};

struct MultiState : BaseState
{
	MultiState();

	BaseState* StartArray(Context& ctx);
	BaseState* StartObject(Context& ctx);
	BaseState* EndArray(Context& ctx, rapidjson::SizeType);
};

// "...":[Numbers only]
class ArrayState : public BaseState
{
	feature_index array_hash;
	BaseState* return_state;

public:
	ArrayState();

	BaseState* StartArray(Context &ctx);
	BaseState* Float(Context& ctx, float f);
	BaseState* Uint(Context& ctx, unsigned f);
	BaseState* EndArray(Context& ctx, rapidjson::SizeType elementCount);
};

// only 0 is valid as DefaultState::Ignore injected that into the source stream
struct IgnoreState : BaseState
{
	IgnoreState();

	BaseState* Uint(Context& ctx, unsigned);
};

class DefaultState : public BaseState
{
private:
	BaseState* Ignore(Context& ctx, rapidjson::SizeType length);

	void InsertNamespace(Context& ctx);

public:
	DefaultState();

	BaseState* Key(Context& ctx, const char* str, rapidjson::SizeType length, bool copy);
	BaseState* String(Context& ctx, const char* str, rapidjson::SizeType length, bool copy);
	BaseState* Bool(Context& ctx, bool b);
	BaseState* StartObject(Context& ctx);
	BaseState* EndObject(Context& ctx, rapidjson::SizeType memberCount);
	BaseState* Float(Context& ctx, float f);
	BaseState* Uint(Context& ctx, unsigned f);
	BaseState* StartArray(Context& ctx);
};

struct Context
{
	vw* all;
	std::stringstream error;

	// last "<key>": encountered
	const char* key;
	rapidjson::SizeType key_length;

	BaseState* current_state;
	BaseState* previous_state;

	// the path of namespaces
	v_array<Namespace> namespace_path;

	v_array<example*>* examples;
	example* ex;
	rapidjson::InsituStringStream* stream;

	VW::example_factory_t example_factory;
	void* example_factory_context;

	// states
	DefaultState default_state;
	LabelState label_state;
	LabelObjectState label_object_state;
	LabelSinglePropertyState label_single_property_state;
	LabelIndexState label_index_state;
	TextState text_state;
	TagState tag_state;
	MultiState multi_state;
	IgnoreState ignore_state;
	ArrayState array_state;

	Context();
	~Context();
	void init(vw* all);

	void PushNamespace(const char* ns, BaseState* return_state);

	Namespace& CurrentNamespace();

	bool TransitionState(BaseState* next_state);
};

struct VWReaderHandler : public rapidjson::BaseReaderHandler<rapidjson::UTF8<>, VWReaderHandler>
{
	Context ctx;

	void init(vw* all, v_array<example*>* examples, rapidjson::InsituStringStream* stream, VW::example_factory_t example_factory, void* example_factory_context);

	// virtual dispatch to current state
	bool Bool(bool v);
	bool Int(int v);
	bool Uint(unsigned v);
	bool Int64(int64_t v);
	bool Uint64(uint64_t v);
	bool Double(double v);
	bool String(const Ch* str, rapidjson::SizeType len, bool copy);
	bool StartObject();
	bool Key(const Ch* str, rapidjson::SizeType len, bool copy);
	bool EndObject(rapidjson::SizeType count);
	bool StartArray();
	bool EndArray(rapidjson::SizeType count);

	bool Null();
	bool Default();

	// alternative to above if we want to re-use the VW float parser...
	bool RawNumber(const Ch* str, rapidjson::SizeType length, bool copy);

	std::stringstream& error();

	BaseState* current_state();
};

struct json_parser
{
	rapidjson::Reader reader;
	VWReaderHandler handler;
};
