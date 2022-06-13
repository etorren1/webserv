#ifndef Server_hpp
#define Server_hpp

#include <iostream>
#include <netdb.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <poll.h>
#include <errno.h>
#include <unistd.h>
#include <vector>
#include <list>
#include <set>
#include <fcntl.h>
#include <string>
#include <sstream>
#include <ctime>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <signal.h>

#include "config/Config.hpp"
#include "Client.hpp"
#include "Utils.hpp"
#include "Request.hpp"
#include "Response.hpp"

#define	STOP	0b00
#define WORKING 0b10
#define RESTART 0b01
#define BUF_SIZE 1024
#define DEFAULT_PATH "webserv.conf"
#define srvs_iterator std::map<std::string, Server_block * >::iterator
#define lctn_iterator std::map<std::string, Location_block * >::iterator
#define client_iterator std::map<size_t, Client * >::iterator

class Server {
	private:

		std::vector<struct pollfd>	fds;
		std::set<int>				srvSockets;
		std::map<std::string, Server_block * > srvs;
		std::map<size_t, Client *>	client;
		Http_block     				*http;

		std::string					cfg_path;
		int							status;

		std::string					location;
		char						**envp;
		
		void 		connectClients( const int & fd );
		void 		mainHandler( void );
		int  		readRequest( const size_t id );
		void 		disconnectClients( const size_t id );
		void 		consoleCommands( void );
		int			createVirtualServer( const std::string & hostname, const std::string & port, Server_block * srv );
		int    		closeVirtualServer( Server_block * srv, int sock, const std::string & error, const std::string & text );
		void    	getHostAndPort( const std::string & listen, std::string & hostname, std::string & port );
		void		checkBodySize( const size_t socket, const std::string & text );
		// int		    readHeader( const size_t socket, std::string & text );
		Server_block * getServerBlock( std::string host ) const;
		void		clientRequest(const int socket);

		// config file parser utilites
			std::string get_raw_param(std::string key, std::string & text);
			int    	get_block(const std::string& prompt,const std::string& content, std::string& dest, int last = 0);
			void    cut_comments( std::string & text );
			template <class T> void		cfg_set_attributes( std::string & text, T * block );
			template <class T> void		cfg_server_name(std::string & text, T * block );
			template <class T> void		cfg_index(std::string & text, T * block );
			template <class T> void		cfg_accepted_methods(std::string & text, T * block );
			template <class T> void     cfg_root( std::string & text, T * block );
			template <class T> void     cfg_default_page( std::string & text, T * block );
			template <class T> void		cfg_listen(std::string & text, T * block );
			template <class T> void		cfg_error_log( std::string & text, T * block );
			template <class T> void		cfg_access_log( std::string & text, T * block );
			template <class T> void		cfg_sendfile( std::string & text, T * block );
			template <class T> void		cfg_autoindex( std::string & text, T * block );
			template <class T> void		cfg_return( std::string & text, T * block );
			template <class T> void     cfg_error_page( std::string & text, T * block );
			template <class T> void		cfg_client_max_body_size( std::string & text, T * block );
			template <class T> void		cfg_location_block( std::string & text, T * block );
			template <class T> void		cfg_server_block( std::string & text, T * block );

		bool			isServerSocket( const int & fd );
		void			closeServer( int status );
		void    		writeLog( const std::string & path, const std::string & header, const std::string & text = "");
		void			errorShutdown( int code, const std::string & path, const std::string & error, const std::string & text = "");

		// for response:
		void			make_response(Request req, const size_t id);

		Server();
		Server( const Server & src );
		Server operator=( const Server & src ); 

	public:

		Server( char **envp, std::string new_config_path = "" ) ;
		~Server();

		void	config( const int & fd );
		void	create();
		void	run( void );

};

#endif