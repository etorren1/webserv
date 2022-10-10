#include "Server.hpp"

bool    Server::isServerSocket( const int & fd ) {
	if (srvSockets.find(fd) != srvSockets.end())
		return true;
	return false;
}

void     Server::checkBodySize( const size_t socket, size_t size ) {
    if (client[socket]->getMaxBodySize() == 0)
        return ;
    if (size > client[socket]->getMaxBodySize()) {
		debug_msg(1, RED, "Size more than client_max_body_size: has 413 exception ");
        throw codeException(413);
    }
}

int Server::checkTimeout( size_t socket ) {

	int timeout = http->get_connection_timeout();
	if (timeout && (timeChecker() - client[socket]->getlastActivity() > timeout))
		return 1;
	return 0;
}

Server_block * Server::searchServerName(std::string host)
{
	srvs_iterator begin = srvs.begin();
	srvs_iterator end = srvs.end();
	
	while(begin != end)	//обход серверов
	{
		std::vector <std::string> srvNames = begin->second->get_server_name();
		for (size_t i = 0; i < srvNames.size(); i++) //обход названий
			if (srvNames[i] == host)
				return(begin->second);
		begin++;
	}
	std::exception e;
	throw e;
}

Server_block * Server::getServerBlock( std::string host )
{
	Server_block * srv;
	size_t pos;

	if ((pos = host.find("localhost")) != std::string::npos)
		host = "127.0.0.1" + host.substr(9);

	try	{
		srv = searchServerName(host);
	}
	catch(const std::exception& e) {
		try {
			srv = srvs.at(host);
		}
		catch(const std::exception& e) {
			try {
				 pos = host.find(":");
				 host = "0.0.0.0" + host.substr(pos);
				 srv = srvs.at(host);
			}
			catch(const std::exception& e) {
				 return (NULL);
			}
		}
	}
	return srv; 
}
