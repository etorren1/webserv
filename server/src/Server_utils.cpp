#include "Server.hpp"

bool    Server::isServerSocket( const int & fd ) {
	if (srvSockets.find(fd) != srvSockets.end())
		return true;
	return false;
}

static void    rek_mkdir( std::string path)
{
	int sep = path.find_last_of("/");
	std::string create = path;
	if (sep != std::string::npos) {
		rek_mkdir(path.substr(0, sep));
		path.erase(0, sep);
	}
	mkdir(create.c_str(), 0777);
}

void    Server::writeLog( const std::string & path, const std::string & header, const std::string & text ) {
    if (path != "off") {
        int fd;
        char buf[BUF_SIZE];
        fd = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777); // | O_TRUNC | O_APPEND
        if (fd < 0) {
            int sep = path.find_last_of("/");
            if (sep != std::string::npos) {
                rek_mkdir(path.substr(0, sep));
            }
            fd = open(path.c_str(), O_RDWR | O_CREAT | O_TRUNC, 0777); // | O_TRUNC | O_APPEND
            if (fd < 0) {
                std::cerr << RED << "Error: can not open or create log file" << RESET << "\n";
                return ;
            }
        }
        if (fd) {
            std::time_t result = std::time(NULL);
            std::string time = std::asctime(std::localtime(&result));
            time = "[" + time.erase(time.size() - 1) + "] ";
            write(fd, time.c_str(), time.size());
            write(fd, header.c_str(), header.size());
            write(fd, "\n", 1);
            write(fd, text.c_str(), text.size());
            write(fd, "\n\n", 2);
            close (fd);
        }
    }
}

void     Server::checkBodySize( const size_t socket, size_t size ) {
    if (client[socket]->getMaxBodySize() == 0)
        return ;
    if (size > client[socket]->getMaxBodySize()) {
        std::cout << RED << "Size more than client_max_body_size: has 400 exception " << RESET << "\n";
        throw codeException(400);
    }
}

Server_block * Server::searchServerName(std::string host)
{
	Server_block * srv;
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
