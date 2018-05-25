#include "api_status.h"

namespace reinforcement_learning {

	int api_status::get_error_code() const {
		return _error_code;
	}

	const char* api_status::get_error_msg() const {
		return _error_msg.c_str();
	}

	api_status::api_status()
		: _error_code(0), _error_msg("")
	{}

	//static helper: update the status if needed (i.e. if it is not null)
	void api_status::try_update(api_status * status, const int new_code, const char* new_msg) {
		if ( status != nullptr ) {
			status->_error_code = new_code;
			status->_error_msg = new_msg;
		}
	}

  void api_status::try_clear(api_status* status) {
    if ( status != nullptr ) {
      status->_error_code = 0;
      status->_error_msg.clear();
    }
  }
}
