/*
Copyright (c) by respective owners including Yahoo!, Microsoft, and
individual contributors. All rights reserved.  Released under a BSD (revised)
license as described in the file LICENSE.
*/

#pragma once
#include <stdexcept>
#include <sstream>

namespace VW {

	class vw_exception : public std::exception
	{
	private:
		std::string message;

		// source file exception was thrown
		const char* file;

		// line number exception was thrown
		int lineNumber;

	public:
		vw_exception(const char* file, int lineNumber, std::string message);

		vw_exception(const vw_exception& ex);

		virtual const char* what() const _NOEXCEPT;

		const char* Filename() const;

		int LineNumber() const;
	};

// ease error handling and also log filename and line number
#define THROW(args) \
	{ \
		std::stringstream __msg; \
		__msg << args; \
		throw VW::vw_exception(__FILE__, __LINE__, __msg.str()); \
	}

}
