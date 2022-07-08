#ifndef Client_hpp
#define Client_hpp

#include "Request.hpp"
#include "Response.hpp"
#include "config/Server_block.hpp"
#include <sys/socket.h>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <map>

#define REQ_DONE	0x0001
#define RESP_DONE	0x0002
#define IS_DIR		0x0004
#define IS_FILE		0x0008
#define AUTOIDX		0x0010
#define ERROR		0x0020
#define	HEAD_SENT	0x0040
#define REDIRECT	0x0080
#define IS_BODY		0x0100
#define STRM_READY	0x0200
#define IS_WRITE	0x0400
#define CGI_DONE	0x0800
#define IS_CGI		0x1000

#define TIMEOUT		10

class Client
{
	private:
		Request								req;
		Response							res;
		Server_block						*srv;
		Location_block						*loc;

		bool								fullpart;
		size_t								socket;
		size_t								reader_size;
		std::stringstream					reader;
		std::string							header;

			size_t				wrtRet;
			size_t				rdRet;
			size_t				countw;
			size_t				countr;

		std::map<int, std::string>			resCode;
		std::vector<std::string>			envpVector;
		int									statusCode;
		std::string							location;
		time_t								time;
		time_t								lastTime;

		//bonus:
		std::map<std::string, std::string>	cookies;

	public:

		int			status;

		void 						checkMessageEnd( void );
		void						savePartOfStream( size_t pos );
		void						clearStream( void );
		void						handleRequest( void );
		void						handleError( const int code );
		int							searchErrorPages( void );
		void						cleaner( void );

		// location
		int							parseLocation( void );
		void						findIndex( void );

		//for response:
		void						initResponse( char **envp );
		void						createOrDelete( void );
		void						makeResponse( void );
		void						makeGetResponse( void );
		void						makePostResponse( void );
		void						makeResponseWithoutBody();
		void						makeErrorResponse( void );
		int							makeRedirect( int code, std::string loc );
		int							checkTimeout( void );

		void						setStreamSize( const size_t size );
		void						setServer( Server_block * s );
		void						setClientTime(time_t);
		void						setLastTime(time_t);

		bool 						readComplete( void );
		std::string					getHost( void ) const;
		size_t						getMaxBodySize( void ) const;
		Response &					getResponse( void );
		Request &					getRequest( void );
		Server_block * 				getServer( void );
		std::stringstream &			getStream( void );
		std::string &				getHeader( void );
		size_t						getStreamSize( void );
		Location_block * 			getLocationBlock( std::vector<std::string> vec ) const;
		time_t						getClientTime();
		time_t						getLastTime();

		void						autoindex( const std::string & path );
		void						extractCgiHeader( char * buff );
		void						redirectPost( void );
		Client( size_t nwsock );
		~Client();
};

#endif