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

#define REQ_DONE	0x001
#define RESP_DONE	0x002
#define IS_DIR		0x004
#define IS_FILE		0x008
#define AUTOIDX		0x010
#define ERROR		0x020
#define	HEAD_SENT	0x040
#define REDIRECT	0x080
#define IS_BODY		0x100
#define STRM_READY	0x200
#define IS_WRITE	0x400

#define TIMEOUT		30

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

			long				wrtRet;
			long				rdRet;

		std::map<int, std::string>			resCode;
		std::vector<std::string>			envpVector;
		int									statusCode;
		std::string							location;
		time_t								time;
		time_t								lastTime;

		//bonus:
		std::map<std::string, std::string>	cookies;

		// for POST:
		int					pipe1[2];
		int					pipe2[2];
		pid_t				pid;
		int					ex;
		size_t				totalSent;

	public:
		int			iter; //TEMPORARY - TO DELETE
		int			status;
		bool		cgiWriteFlag;

		void 						checkMessageEnd( void );
		void						savePartOfStream( size_t pos );
		void						clearStream( void );
		void						handleRequest( char **envp );
		void						handleError( const int code );
		int							parseLocation( );
		int							searchErrorPages( void );
		void						cleaner( void );

		//for response:
		void						initResponse( char **envp );
		void						makeResponse( char **envp );
		void						makeGetResponse( void );
		void						makePostResponse( char **envp );
		void						makeDeleteResponse( char ** envp );
		void						makePutResponse( char ** envp );
		void						makeErrorResponse( void );
		void						makeAutoidxResponse( void );
		int							makeRedirect( int code, std::string loc );
		int							checkTimeout( long );
		void						checkTimeout2( long );
		void						passThroughCgi();

		void						setStream( const std::stringstream & mess, const size_t size);
		void						setServer( Server_block * s );
		void						setClientTime(time_t);
		void						setLastTime(time_t);

		bool 						readComplete( void ) const;
		std::string					getHost( void ) const;
		size_t						getMaxBodySize( void ) const;
		Response &					getResponse( void );
		Request &					getRequest( void );
		Server_block * 				getServer( void );
		std::stringstream &			getStream( void );
		std::string &				getHeader( void );
		size_t						getStreamSize( void );
		Location_block * 			getLocationBlock( std::vector<std::string> vec ) const;
		int *						getPipe1();
		int *						getPipe2();
		time_t						getClientTime();
		time_t						getLastTime();

		void						autoindex( const std::string & path );
		void						extractCgiHeader( char * buff );

		Client( size_t nwsock );
		~Client();
};

#endif