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

#define REQ_DONE	0b00000001
#define RESP_DONE	0b00000010
#define IS_DIR		0b00000100
#define IS_FILE		0b00001000
#define AUTOIDX		0b00010000
#define ERROR		0b00100000
#define	HEAD_SENT	0b01000000

class Client
{
	private:
		Request						req;
		Response					res;
		Server_block				*srv;

		bool						breakconnect;
		size_t						socket;
		std::string					message;

		std::map<int, std::string>	resCode;
		int							statusCode;
		std::string					location;


	public:
		int							status;

		void 						checkConnection( const std::string & mess );
		void						handleRequest( void );
		void						handleError( const int code );
		int							parseLocation( void );
		void						cleaner( void );

		//for response:
		void						initResponse();
		void						makeResponse( char **envp );
		void						makeGetResponse( void );
		void						makePostResponse( char **envp );
		void						makeDeleteResponse( void );
		void						makeErrorResponse( void );
		void						makeAutoidxResponse( void );
		void						makeRedirectPesponse( int code, std::string loc );

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