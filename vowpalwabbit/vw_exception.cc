#include "vw_exception.h"

vw_exception::vw_exception()
{
}

vw_exception::vw_exception(const vw_exception& ex) : message(ex.message.str())
{
}

//vw_exception::vw_exception(const char* pmsg) 
//{
//	message << pmsg;
//}

//std::ostream& vw_exception::stream()
//{
//	return message;
//}

const char * vw_exception::what() const _NOEXCEPT
{
	return message.str().c_str();
}

// UPDATE MAKEFILE!!!

//template<typename T>
//vw_exception& operator<<(vw_exception& os, T msg)
//{
//	os.foo << msg;
//	return os;
//}


//vw_exception& operator<<(vw_exception& os, const char* msg)
//{
//	os.msg << msg;
//	return os;
//}
