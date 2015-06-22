#include "vw_exception.h"

namespace VW {

	vw_exception::vw_exception(const char* pfile, int plineNumber, std::string& pmessage)
		: file(pfile), lineNumber(plineNumber), message(pmessage)
	{
	}

	vw_exception::vw_exception(const vw_exception& ex)
		: message(ex.message), file(ex.file), lineNumber(ex.lineNumber)
	{
	}

	const char* vw_exception::what() const _NOEXCEPT
	{
		return message.c_str();
	}
	
	const char* vw_exception::Filename() const
	{
		return file;
	}

	int vw_exception::LineNumber() const
	{
		return lineNumber;
	}
}