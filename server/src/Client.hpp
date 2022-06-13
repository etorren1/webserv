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

#define REQ_DONE	0x001
#define RESP_DONE	0x002
#define IS_DIR		0x004
#define IS_FILE		0x008
#define AUTOIDX		0x010
#define ERROR		0x020
#define	HEAD_SENT	0x040
#define REDIRECT	0x080
#define IS_BODY		0x100

class Client
{
	private:
		Request						req;
		Response					res;
		Server_block				*srv;
		Location_block				*loc;

		bool						fullpart;
		size_t						socket;
		std::string					message;
		std::string					tail;

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

		void 						checkMessageEnd( void );
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
		int							makeRedirect( int code, std::string loc );

		void						setMessage( const std::string & mess );
		void						setServer( Server_block * s );

		bool 						readComplete( void ) const;
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