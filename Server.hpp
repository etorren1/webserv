#ifndef Server_hpp
#define Server_hpp

#include <iostream>
#include <netdb.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <poll.h>
#include <unistd.h>
#include <vector>
#include <list>
#include <set>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <ctime>
#include "Utils.hpp"
#include "Config/Config.hpp"
#include "Request.hpp"

#define	STOP	0b00
#define WORKING 0b10
#define RESTART 0b01
#define BUF_SIZE 2048

typedef struct s_cfg
{
    std::string     hostname;
    std::string     port;
	int				error_fd;
	int				access_fd;
    std::list<std::string> locations;

} t_cfg;

class Server {
	private:

		std::vector<struct pollfd>	fds;
		std::vector<std::string>	mess;
		std::vector<bool>			cnct;

		Http_block     				http;
		std::set<int>				srvSockets;
		std::map<std::string, Server_block *> srvs;

		struct s_cfg		conf;
		int					status;
		Request 			req;
		
		void 		connectClients( const int & fd );
		void 		clientRequest( void );
		int  		readRequest( const size_t id );
		void 		disconnectClients( const size_t id );
		void 		consoleCommands( void );
		// config file parser utilites
			std::string get_raw_param(std::string key, std::string & text);
			int    	get_block(const std::string& prompt,const std::string& content, std::string& dest, int last = 0);
			void    cut_comments( std::string & text );
			template <class T>
			void 	cfg_listen(std::string & text, T & block );
			template <class T>
			void    cfg_server_block( std::string & text, T & block );
			template <class T>
			void    cfg_error_log( std::string & text, T & block );
			template <class T>
			void    cfg_access_log( std::string & text, T & block );

		bool			isServerSocket( const int & fd );
		void			closeServer( int status );
		void    		writeLog( const std::string & path, const std::string & header, const std::string & text );
		void			errorShutdown( int code, const std::string & path, const std::string & error, const std::string & text = "");

		// for response:
		void			make_response(Request req, const size_t id);

		Server( const Server & src );
		Server operator=( const Server & src ); 

	public:

		Server( const int & config_fd );
		~Server();

		void		parseConfig( const int & fd );
		void	create( void );
		void	run( void );

};

#endif