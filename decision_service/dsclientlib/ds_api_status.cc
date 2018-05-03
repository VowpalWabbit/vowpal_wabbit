
#include "ds_api_status.h"


namespace decision_service {

	int api_status::get_error_code() const {
		return _error_code;
	}

	void api_status::set_error_code(int new_code) {
		_error_code = new_code;
	}

	const std::string& api_status::get_error_msg() const {
		return _error_msg;
	}

	void api_status::set_error_msg(const std::string& new_msg) {
		_error_msg = new_msg;
	}

	api_status::api_status()
		: _error_code(0), _error_msg("")
	{}

	void api_status::clear()
	{
		_error_code = 0;
		_error_msg.clear();
	}

	//static helper: update the status if needed (i.e. if it is not null)
	void api_status::try_update(api_status * status, int new_code, const std::string & new_msg) {
		if (status) {
			status->set_error_code(new_code);
			status->set_error_msg(new_msg);
		}
	}

}