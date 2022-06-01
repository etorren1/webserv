#ifndef Client_hpp
#define Client_hpp

#include "Request.hpp"
#include "Response.hpp"
#include "config/Server_block.hpp"
#include <sys/socket.h>
#include <map>

#define REQ_DONE	0x01
#define RESP_DONE	0x02

class Client
{
	private:
		Request	req;
		Response res;
		Server_block * srv;

		bool		breakconnect;
		size_t		socket;
		std::string host;
		size_t		max_body_size;
		std::string	message;

		std::map<int, std::string>	resCode;


	public:

		int			status;

		void		generateErrorPage( const int error );
		void 		checkConnection( const std::string & mess );
		void		handleRequest( void );
		void		makeResponse( void );

		void		setHost( const std::string & nwhost );
		void		setMaxBodySize( const size_t n );
		void		setMessage( const std::string & mess );
		void		setServer( Server_block * s );

		bool 		getBreakconnect( void ) const;
		std::string	getHost( void ) const;
		size_t		getMaxBodySize( void ) const;
		Response &	getResponse( void );
		Request &	getRequest( void );
		Server_block * getServer( void );
		std::string getMessage( void ) const;


		Client( size_t nwsock );
		~Client();
};

#endif