#include "Client.hpp"

void		Client::checkConnection( const std::string & mess ) {
	if (mess.find_last_of("\n") != mess.size() - 1)
		breakconnect = true;
	breakconnect = false;
}

void		Client::handleRequest( void ) {
	req.parseText(message);
	status |= REQ_DONE;
	message.clear();
}

void		Client::makeResponse() {
	std::stringstream response;
	size_t result;
	if (req.getReqURI() == "/favicon.ico")
	{
		res.setFileLoc("./site/image.png");
		res.setContentType("image/png");
	}
	else
	{
		// 	res.setFileLoc(location);
		// 	res.setContentType(req.getContentType());
		// }
		res.setFileLoc("./site/video.mp4");
		res.setContentType("video/mp4");
	}
	// res.setFileLoc("./site/index.html");
	// res.setContentType("text/html");
	// res.setFileLoc("./site/image.jpg");
	// res.setContentType("image/jpg");
	int rd = 0;
	try
	{
		if (res._hederHasSent == 0)
		{
			res.make_response_header(req);
			result = send(socket, res.getHeader().c_str(), res.getHeader().length(), 0);	// Отправляем ответ клиенту с помощью функции send
			res._hederHasSent = 1;
		}
		if (res._hederHasSent == 1)
			rd = res.make_response_body(req, socket);
		if (rd)
			req.cleaner();
	}
	catch (codeException &e)
	{
		generateErrorPage(e.getErrorCode());
		return;
	}
	if (rd)
		res.clearResponseObj();
}


void	Client::generateErrorPage( const int error ) {
    std::string mess = "none";
    const int &code = error;
    std::map<int, std::string>::iterator it = resCode.begin();
    for (; it != this->resCode.end(); it++) {
        if (code == (*it).first) {
            mess = (*it).second;
        }
    }
    std::string responseBody = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"> \
                                <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"> \
                                <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> \
                                <title>Error page </title></head><body><div class=\"container\"><h2>" \
                                + itos(code) + "</h2><h3>" + mess + "</h3> \
                                <p><a href=\"#homepage\">Click here</a> to redirect to homepage.</p></div></body></html>";
    std::string header = "HTTP/1.1 " + itos(code) + " " + mess + "\n" + "Version: " + "HTTP/1.1" \
                         + "\n" + "Content-Type: " + "text/html" + "\n" + "Content-Length: " + itos(responseBody.length()) + "\n\n";
    std::string response = header + responseBody;
    size_t res = send(socket, response.c_str(), response.length(), 0);
}

void		Client::setHost( const std::string & nwhost ) { host = nwhost; }
void		Client::setMaxBodySize( const size_t n ) { max_body_size = n; }
void		Client::setMessage( const std::string & mess ) { message = mess; }
void		Client::setServer( Server_block * s ) { srv = s; }

bool 		Client::getBreakconnect() const { return breakconnect; }
Response &	Client::getResponse() { return res; }
Request &	Client::getRequest() { return req; }
size_t		Client::getMaxBodySize() const { return max_body_size; }
std::string	Client::getHost() const { return host; }
std::string Client::getMessage() const { return message; }
Server_block * Client::getServer( void ) { return srv;}

Client::Client(size_t nwsock) {
	breakconnect = false;
	socket = nwsock;
	status = 0;
	    //Для POST браузер сначала отправляет заголовок, сервер отвечает 100 continue, браузер 
    // отправляет данные, а сервер отвечает 200 ok (возвращаемые данные).
    this->resCode.insert(std::make_pair(100, "Continue"));
    this->resCode.insert(std::make_pair(101, "Switching Protocols"));
    this->resCode.insert(std::make_pair(200, "OK"));
    this->resCode.insert(std::make_pair(201, "Created"));
    this->resCode.insert(std::make_pair(202, "Accepted"));
    this->resCode.insert(std::make_pair(203, "Non-Authoritative Information"));
    this->resCode.insert(std::make_pair(204, "No Content"));
    this->resCode.insert(std::make_pair(304, "Not Modified"));
    this->resCode.insert(std::make_pair(400, "Bad Request"));
    this->resCode.insert(std::make_pair(401, "Unauthorized"));
    this->resCode.insert(std::make_pair(402, "Payment Required"));
    this->resCode.insert(std::make_pair(403, "Forbidden"));
    this->resCode.insert(std::make_pair(404, "Not Found"));
    this->resCode.insert(std::make_pair(405, "Method Not Allowed"));
    this->resCode.insert(std::make_pair(406, "Not Acceptable"));
    this->resCode.insert(std::make_pair(407, "Proxy Authentication Required"));
    this->resCode.insert(std::make_pair(408, "Request Timeout"));
    this->resCode.insert(std::make_pair(409, "Conflict"));
    this->resCode.insert(std::make_pair(500, "Internal Server Error"));
    this->resCode.insert(std::make_pair(501, "Not Implemented"));
    this->resCode.insert(std::make_pair(502, "Bad Gateway"));
    this->resCode.insert(std::make_pair(503, "Service Unavailable"));
    this->resCode.insert(std::make_pair(504, "Gateway Timeout"));
}

Client::~Client() {}
