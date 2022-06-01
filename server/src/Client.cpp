#include "Client.hpp"

void		Client::checkConnection( const std::string & mess ) {
	if (mess.find_last_of("\n") != mess.size() - 1)
		breakconnect = true;
	breakconnect = false;
}

void		Client::setHost( const std::string & nwhost ) { host = nwhost; }
void		Client::setMaxBodySize( const size_t n ) { max_body_size = n; }

bool 		Client::getBreakconnect() const { return breakconnect; }
Response &	Client::getResponse() { return res; };
size_t		Client::getMaxBodySize() const { return max_body_size; }
std::string	Client::getHost() const { return host; }

Client::Client(size_t nwsock) {
	breakconnect = false;
	socket = nwsock;
	status = 0;
}

Client::~Client() {}
