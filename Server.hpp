#ifndef Server_hpp
#define Server_hpp

#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <vector>
#include <fcntl.h>
#include "Utils.hpp"
#include <string>
#include <sstream>

#define WORKING 0b10
#define RESTART 0b01
#define BUF_SIZE 512

class Server {
	private:

		std::vector<struct pollfd>	userFds;

		struct pollfd		srvPoll;
		int					srvFd;
		int					srvPort;
		struct sockaddr_in	address;
		int					status;
		
		void 		connectUsers( void );
		void 		clientRequest( void );
		int  		readRequest( size_t const id );
		void 		consoleCommands( void );

		Server( Server const & src );
		Server operator=( Server const & src ); 

	public:

		Server( std::string const & port);
		~Server();

		void	create( void );
		void	run( void );

};

#endif