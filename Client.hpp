#ifndef Client_hpp
#define Client_hpp

#include "Request.hpp"
#include "Response.hpp"

class Client
{
	private:
		Request	req;
		Response res;

		bool		breakconnect;
		size_t		socket;

	public:

		std::string	message;

		void checkConnection( const std::string & mess );

		bool getBreakconnect() const;

		Client( size_t nwsock );
		~Client();
};

#endif