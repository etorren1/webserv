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

#define	STOP	0b00
#define WORKING 0b10
#define RESTART 0b01
#define ERR_LOG 0x1
#define BUF_SIZE 512

typedef struct s_cfg
{
    std::string     hostname;
    std::string     port;
	std::string		error_log;
    std::list<std::string> locations;

} t_cfg;

class Server {
	private:

		std::vector<struct pollfd>	fds;

		struct s_cfg		conf;
		struct pollfd		srvPoll;
		int					srvFd;
		struct sockaddr_in	address;
		int					status;
		int					flags;
		int					error_fd;
		
		void 		connectUsers( void );
		void 		clientRequest( void );
		int  		readRequest( const size_t id );
		void 		consoleCommands( void );
		void		parseConfig( const int & fd );
		// config file parser utilites
			std::string get_raw_param(std::string key, std::string & text);
			int    	get_block(const std::string& prompt,const std::string& content, std::string& dest, int last = 0);
			void    cut_comments( std::string & text );
			void 	cfg_listen(std::string & text, std::string & host, std::string & port );
			void    cfg_server_block( std::string & text, t_cfg *conf );
			void    cfg_error_log( std::string & text, std::string & error_log_atr );

		void			closeServer( int status );
		void			errorShutdown( int code, const std::string & error, const std::string & text = "");

		Server( const Server & src );
		Server operator=( const Server & src ); 

	public:

		Server( const int & config_fd );
		~Server();

		void	create( void );
		void	run( void );

};

#endif