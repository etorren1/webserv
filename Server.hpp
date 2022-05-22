#ifndef Server_hpp
#define Server_hpp

#include <iostream>
#include <netdb.h>
#include <poll.h>
#include <unistd.h>
#include <vector>
#include <list>
#include <fcntl.h>
#include "Utils.hpp"
#include <string>
#include <sstream>
#include "Request.hpp"

#define WORKING 0b10
#define RESTART 0b01
#define BUF_SIZE 2048

class Server {
	private:

		std::vector<struct pollfd>	userFds;

		struct pollfd		srvPoll;
		int					srvFd;
		int					srvPort;
		struct sockaddr_in	address;
		int					status;
		Request 			req;
		
		void 		connectUsers( void );
		void 		clientRequest( void );
		int  		readRequest( const size_t id );
		void 		consoleCommands( void );
		void		parseConfig( const std::string & config );

		Server( const Server & src );
		Server operator=( const Server & src ); 

	public:

		Server( const std::string & config);
		~Server();

		void	create( void );
		void	run( void );

};

#endif