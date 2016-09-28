/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD
license as described in the file LICENSE.
*/

// TODO: check ifdefs...
//#define RAPIDJSON_SIMD
//#define RAPIDJSON_SSE42

#include "parse_example_json.h"
#include <rapidjson/reader.h>
#include <stack>
#include <vector>
#include "vw.h"
#include "v_array.h"

using namespace rapidjson;
using namespace std;

struct VWReaderHandler : public BaseReaderHandler<UTF8<>, VWReaderHandler>
{
	struct Namespace
	{
		char feature_group;
		feature_index namespace_hash;
		features* features;
		size_t feature_count;
	};

	vw* all;
	example* ex;
	InsituStringStream* stream;

	// last "<key>": encountered
	const char* key;
	SizeType key_length;

	// the path of namespaces
	v_array<Namespace> namespace_path;

	const char* error;

	feature_index(VWReaderHandler::*hash_method)();

	// hash method state
	v_array<char> string_buffer;
	feature_index array_hash;

	bool (VWReaderHandler::*string_method)(const Ch* str, SizeType length);

	bool ignoreNextUint;

	VWReaderHandler(vw* pall, example* pex, InsituStringStream* pstream) 
		: all(pall), ex(pex), error(nullptr), key(" "), key_length(1), stream(pstream), ignoreNextUint(false)
	{
		hash_method = &VWReaderHandler::StringHash;
		string_buffer = v_init<char>();
		namespace_path = v_init<Namespace>();
	}

	Namespace& CurrentNamespace()
	{
		return *(namespace_path._end - 1);
	}

	void PushNamespace(const Ch* ns)
	{
		Namespace n;
		n.feature_group = ns[0];
		n.namespace_hash = VW::hash_space(*all, ns);
		n.features = ex->feature_space + ns[0];
		n.feature_count = 0;

		namespace_path.push_back(n);
	}

	feature_index StringHash()
	{
		return VW::hash_feature(*all, key, CurrentNamespace().namespace_hash);
	}

	feature_index ArrayHash()
	{
		auto hash = array_hash;
		array_hash++;

		return hash;
	}

	bool StringLabel(const Ch* str, SizeType length)
	{
		VW::parse_example_label(*all, *ex, str);
		return true;
	}

	bool StringText(const Ch* str, SizeType length)
	{
		const Ch* start = str;
		const Ch* end = str + length;
		for (Ch* p = (Ch*)str;p != end;p++)
		{
			if (*p == ' ' || *p == '\t')
			{
				*p = '\0';
				StringFeature(start);

				start = p + 1;
			}
		}

		if (start < end)
			StringFeature(start);

		return true;
	}

	bool StringDefault(const Ch* str, SizeType length)
	{
		// string escape
		const Ch* end = str + length;
		for (Ch* p = (Ch*)str;p != end;p++)
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
		string_buffer.erase();
		size_t total_length = key_length + length;

		if (string_buffer.size() < total_length)
			string_buffer.resize(total_length + 1);

		memcpy(string_buffer.begin(), key, key_length);
		memcpy(string_buffer.begin() + key_length, str, length);
		string_buffer[total_length] = '\0';

		StringFeature(string_buffer.begin());

		return true;
	}

	bool Ignore(SizeType length)
	{
		// fast ignore
		// skip key + \0 + "
		Ch* head = stream->src_ + length + 2;

		if (*head != ':')
			return false;
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
		Ch* value = stream->src_ + length + 3;
		*value = '0';
		value++;
		memset(value, ' ', head - value - 1);

		ignoreNextUint = true;

		return true;
	}

	bool Key(const Ch* str, SizeType length, bool copy)
	{
		// only to be used with 
		assert(!copy);

		key = str;
		key_length = length;

		if (length > 0 && str[0] == '_')
		{
			if (!strncmp(key, "_label", max(6, key_length)))
				string_method = &VWReaderHandler::StringLabel;
			else if (!strncmp(key, "_text", max(5, key_length)))
				string_method = &VWReaderHandler::StringText;
			else
				return Ignore(length);
		}
		else
			string_method = &VWReaderHandler::StringDefault;

		return true;
	}

	bool StartObject()
	{
		PushNamespace(key);

		return true;
	}

	bool EndObject(SizeType memberCount)
	{
		if (CurrentNamespace().feature_count > 0)
		{
			auto feature_group = CurrentNamespace().feature_group;
			// avoid duplicate insertion
			for (unsigned char ns : ex->indices)
				if (ns == feature_group)
					return true;

			ex->indices.push_back(feature_group);
		}

		// return to default namespace
		namespace_path.pop();

		return true;
	}

	bool StartArray()
	{
		key = "";
		key_length = 0;
		array_hash = CurrentNamespace().namespace_hash;
		hash_method = &VWReaderHandler::ArrayHash;

		return true;
	}

	bool EndArray(SizeType elementCount)
	{
		hash_method = &VWReaderHandler::StringHash;

		return true;
	}

	bool AddFeature(float f)
	{
		auto& ns = CurrentNamespace();
		ns.features->push_back(f, (this->*hash_method)());
		ns.feature_count++;

		return true;
	}

	bool Bool(bool b)
	{
		return AddFeature(1.);
	}

	bool Int(int i)
	{
		return AddFeature(i);
	}

	bool Uint(unsigned i)
	{
		// set by Ignore()
		if (ignoreNextUint)
		{
			ignoreNextUint = false;
			return true;
		}

		return AddFeature(i);
	}

	bool Int64(int64_t i)
	{
		return AddFeature(i);
	}

	bool Uint64(uint64_t i)
	{
		return AddFeature(i);
	}

	bool Double(double i)
	{
		return AddFeature(i);
	}

	// alternative to above if we want to re-use the VW float parser...
	bool RawNumber(const Ch* str, SizeType length, bool copy)
	{
		return false;
	}

	void StringFeature(const Ch* str)
	{
		auto& ns = CurrentNamespace();
		ns.features->push_back(
			1.,
			VW::hash_feature(*all, str, ns.namespace_hash));
		ns.feature_count++;
	}

	bool String(const Ch* str, SizeType length, bool copy)
	{ 
		// dispatch
		(this->*string_method)(str, length);

		return true;
	}
};

namespace VW
{
	void read_line_json(vw& all, example* ex, char* line)
	{
		// TODO: keep handler in *all to cache string buffer
		// reader can be re-used
		Reader reader;

		// destructive parsing
		InsituStringStream ss(line);

		VWReaderHandler handler(&all, ex, &ss);
		reader.Parse<kParseInsituFlag, InsituStringStream, VWReaderHandler>(ss, handler);
	}
}
