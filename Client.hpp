#include "Request.hpp"
#include "Response.hpp"

class Client
{
private:
	std::vector<Request*> req;
	std::vector<Response*> res;
public:
	Client(/* args */);
	~Client();
};

Client::Client(/* args */)
{
}

Client::~Client()
{
}
