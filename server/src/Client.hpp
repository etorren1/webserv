#ifndef Client_hpp
#define Client_hpp

#include "Request.hpp"
#include "Response.hpp"
#include "config/Server_block.hpp"
#include <sys/socket.h>
#include <cstdlib>
// #include <filesystem> C++17 !!!
#include <fstream>
#include <fcntl.h>
#include <map>

#define REQ_DONE	0x01
#define RESP_DONE	0x02
#define IS_DIR		0x04
#define IS_FILE		0x08
#define AUTOIDX		0x10
#define ERROR		0x20
#define	HEAD_SENT	0x40
#define REDIRECT	0x80

class Client
{
	private:
		Request						req;
		Response					res;
		Server_block				*srv;
		Location_block				*loc;

		bool						breakconnect;
		size_t						socket;
		std::string					message;

		std::map<int, std::string>	resCode;
		int							statusCode;
		std::string					location;

		// for POST:
		int					pipe1[2];
		int					pipe2[2];
		pid_t				pid;
		int					ex;


	public:
		int							status;
		bool						cgiWriteFlag;

		void 						checkConnection( const std::string & mess );
		void						handleRequest( char **envp );
		void						handleError( const int code );
		int							parseLocation( std::string = "" );
		int							searchErrorPages( void );
		void						cleaner( void );

		//for response:
		void						initResponse( char **envp );
		void						makeResponse( char **envp );
		void						makeGetResponse( void );
		void						makePostResponse( char **envp );
		void						makeDeleteResponse( void );
		void						makeErrorResponse( void );
		void						makeAutoidxResponse( void );
		void						makeRedirect( int code, std::string loc );

		void						setMessage( const std::string & mess );
		void						setServer( Server_block * s );

		bool 						getBreakconnect( void ) const;
		std::string					getHost( void ) const;
		size_t						getMaxBodySize( void ) const;
		Response &					getResponse( void );
		Request &					getRequest( void );
		Server_block * 				getServer( void );
		std::string 				getMessage( void ) const;
		Location_block * 			getLocationBlock( std::vector<std::string> vec ) const;

		void						autoindex( const std::string & path );

		Client( size_t nwsock );
		~Client();
};

#endif