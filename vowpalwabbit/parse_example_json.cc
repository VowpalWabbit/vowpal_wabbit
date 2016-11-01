/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/

#include "vw.h"
#include "parse_example_json.h"

#include <rapidjson/error/en.h>
#include "cb.h"
#include "best_constant.h"
#include <boost/algorithm/string.hpp>

using namespace std;
using namespace rapidjson;

void Namespace::AddFeature(feature_value v, feature_index i)
{
	// filter out 0-values
	if (v == 0)
		return;

	ftrs->push_back(v, i);
	feature_count++;
}

void Namespace::AddFeature(vw* all, const char* str)
{
	ftrs->push_back(
		1.,
		VW::hash_feature(*all, str, namespace_hash));
	feature_count++;
}

Context::Context()
{
	namespace_path = v_init<Namespace>();
	current_state = &default_state;
}

void Context::init(vw* pall) 
{
	all = pall;
	key = " ";
	key_length = 1;
	previous_state = nullptr;
	label_object_state.init(pall);
}

Context::~Context()
{
	namespace_path.delete_v();
}

void Context::PushNamespace(const char* ns, BaseState* return_state)
{
	Namespace n;
	n.feature_group = ns[0];
	n.namespace_hash = VW::hash_space(*all, ns);
	n.ftrs = ex->feature_space + ns[0];
	n.feature_count = 0;
	n.return_state = return_state;

	namespace_path.push_back(n);
}

Namespace& Context::CurrentNamespace() { return *(namespace_path._end - 1); }

bool Context::TransitionState(BaseState* next_state)
{
	if (next_state == nullptr)
		return false;

	previous_state = current_state;
	current_state = next_state;

	return true;
}

BaseState::BaseState(const char* pname) : name(pname)
{ }

BaseState* BaseState::Null(Context& ctx)
{ 
	ctx.error << "Unexpected token: null"; 
	return nullptr; 
}

BaseState* BaseState::Bool(Context& ctx, bool b)
{ 
	ctx.error << "Unexpected token: bool (" << (b ? "true":"false") << ")"; 
	return nullptr; 
}

BaseState* BaseState::Float(Context& ctx, float v)
{ 
	ctx.error << "Unexpected token: float (" << v << ")"; 
	return nullptr; 
}

BaseState* BaseState::Uint(Context& ctx, unsigned v)
{
	ctx.error << "Unexpected token: uint (" << v << ")";
	return nullptr; 
}

BaseState* BaseState::String(Context& ctx, const char* str, SizeType len, bool)
{ 
	ctx.error << "Unexpected token: string('" << str << "' len: " << len <<")";
	return nullptr; 
}
	
BaseState* BaseState::StartObject(Context& ctx)
{ 
	ctx.error << "Unexpected token: {"; 
	return nullptr; 
}

BaseState* BaseState::Key(Context& ctx, const char* str, SizeType len, bool copy)
{ 
	ctx.error << "Unexpected token: key('" << str << "' len: " << len << ")";
	return nullptr;
}

BaseState* BaseState::EndObject(Context& ctx, SizeType)
{ 
	ctx.error << "Unexpected token: }"; 
	return nullptr; 
}

BaseState* BaseState::StartArray(Context& ctx)
{ 
	ctx.error << "Unexpected token: ["; 
	return nullptr; 
}

BaseState* BaseState::EndArray(Context& ctx, SizeType)
{ 
	ctx.error << "Unexpected token: ]";  
	return nullptr; 
}

// LabelObjectState
LabelObjectState::LabelObjectState() : BaseState("LabelObject")
{ 
}

void LabelObjectState::init(vw* all)
{
	found = found_cb = false;

	cb_label = {0.,0,0.,0.};
}

BaseState* LabelObjectState::StartObject(Context& ctx)
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

BaseState* LabelObjectState::Key(Context& ctx, const char* str, SizeType len, bool copy)
{
	ctx.key = str;
	ctx.key_length = len;
	return this;
}

BaseState* LabelObjectState::Float(Context& ctx, float v)
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

BaseState* LabelObjectState::Uint(Context& ctx, unsigned v) { return Float(ctx, (float)v); }

BaseState* LabelObjectState::EndObject(Context& ctx, SizeType)
{ 
	if (found_cb)
	{
		CB::label* ld = (CB::label*)&ctx.ex->l;
		ld->costs.push_back(cb_label);

		found_cb = false;
		cb_label = {0.,0,0.,0.};
	}
	else if (found)
	{
		count_label(ctx.ex->l.simple.label);

		found = false;
	}

	return return_state; 
}

LabelSinglePropertyState::LabelSinglePropertyState() : BaseState("LabelSingleProperty")
{ }

// forward _label
BaseState* LabelSinglePropertyState::Float(Context& ctx, float v)
{
	// skip "_label_"
	ctx.key += 7;
	ctx.key_length -= 7;

	if (ctx.label_object_state.Float(ctx, v) == nullptr)
		return nullptr;

	return ctx.previous_state;
}

BaseState* LabelSinglePropertyState::Uint(Context& ctx, unsigned v)
{
	// skip "_label_"
	ctx.key += 7;
	ctx.key_length -= 7;

	if (ctx.label_object_state.Uint(ctx, v) == nullptr)
		return nullptr;

	return ctx.previous_state;
}

// LabelIndexState
LabelIndexState::LabelIndexState() : BaseState("LabelIndex"), index(-1)
{ } 

BaseState* LabelIndexState::Uint(Context& ctx, unsigned int v)
{
	index = v;
	return ctx.previous_state;
}

// LabelState
LabelState::LabelState() : BaseState("Label")
{ }

BaseState* LabelState::StartObject(Context& ctx) { return ctx.label_object_state.StartObject(ctx); }

BaseState* LabelState::String(Context& ctx, const char* str, SizeType len, bool copy)
{ 
	// only to be used with copy=false
	assert(!copy);

	VW::parse_example_label(*ctx.all, *ctx.ex, str);
	return ctx.previous_state;
}

BaseState* LabelState::Float(Context& ctx, float v)
{
	// TODO: once we introduce label types, check here
	ctx.ex->l.simple.label = v;
	return ctx.previous_state;
}

BaseState* LabelState::Uint(Context& ctx, unsigned v)
{
	// TODO: once we introduce label types, check here
	ctx.ex->l.simple.label = (float)v;
	return ctx.previous_state;
}

// "_text":"a b c" 
TextState::TextState() : BaseState("text")
{ }

BaseState* TextState::String(Context& ctx, const char* str, SizeType length, bool copy)
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

// MultiState
MultiState::MultiState() : BaseState("Multi")
{ }

BaseState* MultiState::StartArray(Context& ctx)
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

BaseState* MultiState::StartObject(Context& ctx)
{
	// allocate new example
	ctx.ex = &(*ctx.example_factory)(ctx.example_factory_context);
	ctx.all->p->lp.default_label(&ctx.ex->l);
	ctx.examples->push_back(ctx.ex);

	// setup default namespace
	ctx.PushNamespace(" ", this);

	return &ctx.default_state;
}

BaseState* MultiState::EndArray(Context& ctx, SizeType)
{
	// return to shared example
	ctx.ex = (*ctx.examples)[0];

	return &ctx.default_state;
}

// ArrayState
ArrayState::ArrayState() : BaseState("Array"), return_state(nullptr)
{ }

BaseState* ArrayState::StartArray(Context &ctx)
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

BaseState* ArrayState::Float(Context& ctx, float f)
{
	ctx.CurrentNamespace().AddFeature(f, array_hash);
	array_hash++;

	return this;
}

BaseState* ArrayState::Uint(Context& ctx, unsigned f) { return Float(ctx, (float)f); }

BaseState* ArrayState::EndArray(Context& ctx, SizeType elementCount) { return return_state; }

// IgnoreState
IgnoreState::IgnoreState() : BaseState("Ignore")
{ }

BaseState* IgnoreState::Uint(Context& ctx, unsigned) { return ctx.previous_state; }

// DefaultState
BaseState* DefaultState::Ignore(Context& ctx, SizeType length)
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

	return &ctx.ignore_state;
}

void DefaultState::InsertNamespace(Context& ctx)
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

DefaultState::DefaultState() : BaseState("Default")
{ }

BaseState* DefaultState::Key(Context& ctx, const char* str, SizeType length, bool copy)
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

		return Ignore(ctx, length);
	}

	return this;
}

BaseState* DefaultState::String(Context& ctx, const char* str, SizeType length, bool copy)
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

BaseState* DefaultState::Bool(Context& ctx, bool b)
{
	if (b)
		ctx.CurrentNamespace().AddFeature(ctx.all, ctx.key);

	return this;
}

BaseState* DefaultState::StartObject(Context& ctx)
{
	ctx.PushNamespace(ctx.key, this);
	return this;
}

BaseState* DefaultState::EndObject(Context& ctx, SizeType memberCount)
{
	InsertNamespace(ctx);
	BaseState* return_state = ctx.namespace_path.pop().return_state;

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
		}

		// inject label
		ctx.label_object_state.EndObject(ctx, memberCount);
	}

	return return_state;
}

BaseState* DefaultState::Float(Context& ctx, float f)
{
	auto& ns = ctx.CurrentNamespace();
	ns.AddFeature(f, VW::hash_feature(*ctx.all, ctx.key, ns.namespace_hash));

	return this;
}

BaseState* DefaultState::Uint(Context& ctx, unsigned f) { return Float(ctx, (float)f); }

BaseState* DefaultState::StartArray(Context& ctx) {  return ctx.array_state.StartArray(ctx); }

// VWReaderHandler 
void VWReaderHandler::init(vw* all, v_array<example*>* examples, InsituStringStream* stream, VW::example_factory_t example_factory, void* example_factory_context)
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
bool VWReaderHandler::Bool(bool v) { return ctx.TransitionState(ctx.current_state->Bool(ctx, v)); }
bool VWReaderHandler::Int(int v) { return ctx.TransitionState(ctx.current_state->Float(ctx, (float)v)); }
bool VWReaderHandler::Uint(unsigned v) { return ctx.TransitionState(ctx.current_state->Uint(ctx, v));}
bool VWReaderHandler::Int64(int64_t v) { return ctx.TransitionState(ctx.current_state->Float(ctx, (float)v)); }
bool VWReaderHandler::Uint64(uint64_t v) { return ctx.TransitionState(ctx.current_state->Float(ctx, (float)v)); }
bool VWReaderHandler::Double(double v) { return ctx.TransitionState(ctx.current_state->Float(ctx, (float)v)); }
bool VWReaderHandler::String(const Ch* str, SizeType len, bool copy) { return ctx.TransitionState(ctx.current_state->String(ctx, str, len, copy)); }
bool VWReaderHandler::StartObject() { return ctx.TransitionState(ctx.current_state->StartObject(ctx)); }
bool VWReaderHandler::Key(const Ch* str, SizeType len, bool copy) { return ctx.TransitionState(ctx.current_state->Key(ctx, str, len, copy)); }
bool VWReaderHandler::EndObject(SizeType count) { return ctx.TransitionState(ctx.current_state->EndObject(ctx, count)); }
bool VWReaderHandler::StartArray() { return ctx.TransitionState(ctx.current_state->StartArray(ctx)); }
bool VWReaderHandler::EndArray(SizeType count) { return ctx.TransitionState(ctx.current_state->EndArray(ctx, count)); }

bool VWReaderHandler::Null() { return true; }
bool VWReaderHandler::Default() { return false; }

// alternative to above if we want to re-use the VW float parser...
bool VWReaderHandler::RawNumber(const Ch* str, SizeType length, bool copy) { return false; }

std::stringstream& VWReaderHandler::error() { return ctx.error; }

BaseState* VWReaderHandler::current_state() { return ctx.current_state;  }

namespace VW
{
	void read_line_json(vw& all, v_array<example*>& examples, char* line, example_factory_t example_factory, void* ex_factory_context)
	{
		// destructive parsing
		InsituStringStream ss(line);

		VWReaderHandler& handler = all.p->jsonp->handler;
	        handler.init(&all, &examples, &ss, example_factory, ex_factory_context);
		
		ParseResult result = all.p->jsonp->reader.Parse<kParseInsituFlag, InsituStringStream, VWReaderHandler>(ss, handler);
		if (!result.IsError())
			return;
		
		BaseState* current_state = handler.current_state();

		THROW("JSON parser error at " << result.Offset() << ": " << GetParseError_En(result.Code()) << ". " 
			  "Handler: " << handler.error().str() <<
			  "State: " << (current_state ? current_state->name : "null"));
	}
}
