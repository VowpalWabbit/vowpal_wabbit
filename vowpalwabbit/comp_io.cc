#include "comp_io.h"
#include <boost/iostreams/filter/gzip.hpp>
#include <fstream>

using namespace std;
using namespace boost::iostreams;

// comp_io_buf::comp_io_input::comp_io_input(std::istream& input) : in(&buf)
comp_io_buf::comp_io_input::comp_io_input(std::istream* pinput, bool own) : input(nullptr), in(&buf)
{
	buf.push(gzip_decompressor());
	buf.push(*pinput);

	if (own)
		input = pinput;
}

comp_io_buf::comp_io_input::~comp_io_input()
{
	if (input)
		delete input;
}

comp_io_buf::comp_io_output::comp_io_output(std::ostream* poutput) : out(&buf), output(poutput)
{
	buf.push(gzip_decompressor());
	buf.push(*poutput);
}

int comp_io_buf::open_file(const char* name, bool stdin_off, int flag)
{ int ret = -1;
	switch (flag)
	{
	case READ:
	{
		unique_ptr<comp_io_input> f;
		if (*name != '\0')
			f = make_unique<comp_io_input>(new ifstream(name, ios_base::in | ios_base::binary));
		else if (!stdin_off)
			f = make_unique<comp_io_input>(&cin, /* own */ false);

		if (f)
		{
			if (f->in.bad())
				return -1;

			gz_input.push_back(std::move(f));
			gz_spec.push_back({ name, stdin_off });
			ret = (int)gz_input.size() - 1;
			files.push_back(ret);
		}
	}

	break;

	case WRITE:
	{
		unique_ptr<comp_io_output> f(new comp_io_output(new ofstream(name, ios_base::out | ios_base::binary)));
		if (f->out.bad())
			return -1;

		gz_output.push_back(std::move(f));
		gz_spec.push_back({ name, stdin_off });
		ret = (int)gz_output.size() - 1;
		files.push_back(ret);

	}
	break;

	default:
		std::cerr << "Unknown file operation. Something other than READ/WRITE specified" << std::endl;
	}
	return ret;
}

void comp_io_buf::reset_file(int f)
{
	comp_spec& spec = gz_spec[f];
	if (f < gz_input.size())
	{
		if (spec.name.length() > 0)
			gz_input[f] = make_unique<comp_io_input>(new ifstream(spec.name, ios_base::in | ios_base::binary));
		else if (!spec.stdin_off)
			gz_input[f] = make_unique<comp_io_input>(&cin, /* own */ false);
	}
	else
		gz_output[f] = make_unique<comp_io_output>(new ofstream(spec.name, ios_base::out | ios_base::binary));

	space.end() = space.begin();
	head = space.begin();
}

ssize_t comp_io_buf::read_file(int f, void* buf, size_t nbytes)
{
	auto& in = gz_input[f]->in;
	if (!in.read((char*)buf, nbytes))
		return -1;

	return in.gcount();
}

size_t comp_io_buf::num_files() { return gz_input.size() + gz_output.size(); }

ssize_t comp_io_buf::write_file(int f, const void* buf, size_t nbytes)
{
	auto& out = gz_output[f]->out;

	if (!out.write((const char*)buf, nbytes))
		return -1;

	return nbytes;
}

bool comp_io_buf::compressed() { return true; }

void comp_io_buf::flush()
{
	if (write_file(0, space.begin(), head - space.end()) != (int)((head - space.end())))
		std::cerr << "error, failed to write to cache\n";
	head = space.begin();
}

bool comp_io_buf::close_file()
{
	if (gz_input.size() == 0 && gz_output.size() == 0)
		return false;

	while (gz_input.size() > 0)
	{
		gz_input.back().release();
		gz_input.pop_back();

		if (files.size() > 0)
			files.pop();
	}

	while (gz_output.size() > 0)
	{
		gz_output.back().release();
		gz_output.pop_back();

		if (files.size() > 0)
			files.pop();
	}

	return true;
}
