#ifndef Client_hpp
#define Client_hpp

#include "Request.hpp"
#include "Response.hpp"

#define REQ_DONE	0x01

class Client
{
	private:
		// Request	req;
		Response res;

		bool		breakconnect;
		size_t		socket;
		std::string host;
		size_t		max_body_size;

	public:

		int			status;
		std::string	message;

		void 		checkConnection( const std::string & mess );

		void		setHost( const std::string & nwhost );
		void		setMaxBodySize( const size_t n );

		bool 		getBreakconnect() const;
		std::string	getHost() const;
		size_t		getMaxBodySize() const;
		Response &	getResponse();


		Client( size_t nwsock );
		~Client();
};

#endif