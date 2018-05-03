#pragma once

#include <string>


namespace decision_service {

	namespace error_code {
		//success code
		const int success = 0;

		//error code
		const int invalid_argument = 1;
		const int background_queue_overflow = 2;
		const int eventhub_http_generic = 3;
		const int eventhub_http_bad_status_code = 4;
	}


	class api_status {
	public:
		int get_error_code() const;
		void set_error_code(int);
		const std::string& get_error_msg() const;
		void set_error_msg(const std::string&);

		api_status();
		void clear();
		static void try_update(api_status* status, int new_code, const std::string& new_msg);

	private:
		int _error_code;
		std::string _error_msg;
	};
	
}