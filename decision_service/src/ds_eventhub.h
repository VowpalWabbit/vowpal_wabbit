#pragma once

#include <cpprest/http_client.h>


namespace decision_service {

	class eventhub {
	public:
		void send(const std::string&);

		eventhub(const std::string&, const std::string&, const std::string&, const std::string&);
		eventhub(const std::string& url);//for tests

	private:
		pplx::task<void> post(const std::string&);
		std::string& authorization();

	private:
		web::http::client::http_client _client;
		
		const std::string _eventhub_host;          //e.g. "ingest-x2bw4dlnkv63q.servicebus.windows.net"
		const std::string _shared_access_key_name; //e.g. "RootManageSharedAccessKey"
		const std::string _shared_access_key;      //e.g. "2MNeQafvOsmV9jgbUOtx4I1KBAtQEWXqU2e6Om8M/n4="
		const std::string _eventhub_name;          //e.g. "interaction"

		std::string _authorization;
		long long _authorization_valid_until;//in seconds
	};
}
