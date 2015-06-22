#pragma once
#include <stdexcept>
#include <sstream>
#include "v_array.h"

namespace VW {

	class vw_exception : public std::exception
	{
	private:
		std::stringstream message;

		// source file exception was thrown
		const char* file;

		// line number exception was thrown
		int lineNumber;

	public:
		vw_exception(const char* pfile, int plineNumber) 
			: file(pfile), lineNumber(plineNumber) 
		{ }

		vw_exception(const vw_exception& ex)
			: message(ex.message.str()), file(ex.file), lineNumber(ex.lineNumber)
		{ }

		virtual const char * what() const _NOEXCEPT
		{ return message.str().c_str(); }

		const char* Filename() const 
		{ return file; }

		int LineNumber() const 
		{ return lineNumber; }

		std::ostream& stream()
		{ return message;}
	};

// ease error handling and also log filename and line number
#define THROW(args) \
	{ \
	VW::vw_exception __err(__FILE__, __LINE__); \
	__err.stream() << args; \
	throw __err; \
	}

}