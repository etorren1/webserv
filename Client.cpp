#include "Client.hpp"

void		Client::checkConnection( const std::string & mess ) {
	if (mess.find_last_of("\n") != mess.size() - 1)
		breakconnect = true;
	breakconnect = false;
}

bool 		Client::getBreakconnect() const { return breakconnect; }
Response &	Client::getResponse() { return res; };

Client::Client(size_t nwsock) {
	breakconnect = false;
	socket = nwsock;
}

Client::~Client() {}
