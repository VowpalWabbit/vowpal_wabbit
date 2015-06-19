#include <stdexcept>
#include <sstream>

class vw_exception : public std::exception
{
	std::stringstream message;

public:
	vw_exception();
	vw_exception(const vw_exception&);
	
	template<typename T>
	vw_exception(T msg)
	{
		message << msg;
	}

	virtual const char * what() const _NOEXCEPT;
	
	template<typename T>
	friend vw_exception& operator<<(vw_exception& os, T msg)
	{
		os.message << msg;
		return os;
	}
};

// for use with file and line numbers
#define THROW(args) { vw_exception __err; __err << args; throw __err; }
