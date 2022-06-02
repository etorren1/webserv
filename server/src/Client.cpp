#include "Client.hpp"

void		Client::checkConnection( const std::string & mess ) {
	if (mess.find_last_of("\n") != mess.size() - 1)
		breakconnect = true;
	breakconnect = false;
}

void		Client::handleRequest( void ) {
	req.parseText(message);
	if (req.getReqURI() != "/favicon.ico")
		parseLocation();
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
		if (rd) {
			req.cleaner();
			status |= RESP_DONE;
		}
	}
	catch (codeException &e)
	{
		generateErrorPage(e.getErrorCode());
		return;
	}
	// if (rd)
	// 	res.clearResponseObj(); //ломает передачу данных
}


void	Client::generateErrorPage( const int error ) {
    std::string mess = "none";
    const int &code = error;
    std::map<int, std::string>::iterator it = resCode.begin();
	try {
		mess = resCode.at(code);
	} catch(const std::exception& e) {}
	
    std::string responseBody = "<!DOCTYPE html><html lang=\"en\"><head><meta charset=\"UTF-8\"> \
                                <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\"> \
                                <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\"> \
                                <title>" + itos(code) + " " + mess + "</title></head><body  align=\"center\"><div class=\"container\"><h1>" \
                                + itos(code) + " " + mess + "</h1><hr></hr> \
                                <p>webserver</p></div></body></html>";
    std::string header = "HTTP/1.1 " + itos(code) + " " + mess + "\n" + "Version: " + "HTTP/1.1" \
                         + "\n" + "Content-Type: " + "text/html" + "\n" + "Content-Length: " + itos(responseBody.length()) + "\n\n";
    std::string response = header + responseBody;
    size_t res = send(socket, response.c_str(), response.length(), 0);
}

void		Client::setMessage( const std::string & mess ) { message = mess; }
void		Client::setServer( Server_block * s ) { srv = s; }

bool 		Client::getBreakconnect() const { return breakconnect; }
Response &	Client::getResponse() { return res; }
Request &	Client::getRequest() { return req; }
size_t		Client::getMaxBodySize() const { return srv->get_client_max_body_size(); }
std::string	Client::getHost() const { return srv->get_listen(); }
std::string Client::getMessage() const { return message; }
Server_block * Client::getServer( void ) { return srv;}

Location_block * Client::getLocationBlock( std::vector<std::string> vec ) const {
    Location_block * lctn;
    size_t pos, i = 0;
    while ( i < vec.size() ) {
		try {
			lctn = srv->lctn.at(vec[i]);
			return (lctn);
		} catch ( const std::exception &e ) {
			i++;
		}
	}
    return NULL; 
}

Client::Client(size_t nwsock) {
	breakconnect = false;
	socket = nwsock;
	status = 0;
	srv = NULL;
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


void Client::parseLocation() {
    // std::cout << YELLOW << "req.getMIMEType() - " << req.getMIMEType() << "\n" << RESET;
    if (req.getMIMEType().empty() || req.getMIMEType() == "none" || req.isFile() == false)
        reqType = 0; // dir
    else
        reqType = 1; //file
	Location_block *loc = getLocationBlock(req.getDirs());
	if (loc == NULL) {
		generateErrorPage(404);
		return ;
	}
	std::string root = loc->get_root();
	// std::cout << GREEN << "this is root - " << root << "\n" << RESET;
    std::vector<std::string> vec = req.getDirs();
    std::vector<std::string> indexPages = loc->get_index();
	std::string defPage, rqst;
	// std::cout << GREEN << "\n	loc->get_path() - " << loc->get_path() << "\n" << RESET;
    std::string request = req.getDirNamesWithoutRoot(loc->get_path());
	// std::cout << GREEN << "this is request - " << request << "\n" << RESET;
	if (request.front() == '/')
		rqst = request.substr(1);
	else rqst = request;
    location = root + rqst;
	// std::cout << GREEN << "this is location - " << location << "\n" << RESET;
	if (reqType == 0) {
		if (existDir(location.c_str())) {
            int ret = open(location.c_str(), O_RDONLY);
			if (!access(location.c_str(), 4)) {
        		// std::cout << "if path - dir\n";
			    // std::cout << "location before .back(/) " << location << "\n";
				if (location.back() != '/')
            	    location.push_back('/');
            	// std::cout << "location after .back(/) " << location << "\n";
				try	{
					std::vector<std::string>::iterator it = indexPages.begin();
					for (; it < indexPages.end(); it++) {
						std::string path = location + *it;
						FILE *file;
        		        file = fopen(path.c_str(), "r");
        		        // std::cout << RED << "path -" << path << " \n" << RESET;
        		        if (file != NULL) {
							defPage = *it;
        		            // std::cout << "defPage " << defPage << " found\n";
        		        }
					}
				}
				catch(const std::exception& e)	{
					generateErrorPage(404);
					std::cerr << e.what() << '\n';
				}
			} else {
                std::cout << "Permission denied\n";
                generateErrorPage(403);
                return ;
            }
            location += defPage;
            // std::cout << "location after += defPage " << location << "\n";
  			FILE *file;
            file = fopen(location.c_str(), "r");
            if (file != NULL) {
                std::cout << "File " << location << " found\n";
            } else {
                generateErrorPage(404);
            }
		} else {
            // std::cout << "Dir " << location << " doesn't exist\n";
            generateErrorPage(404);
            return ;
        }
	} else {
        // std::cout << "if " << location << " is file\n";
        FILE *file;
        // std::cout << "location = " << location << "\n";
        file = fopen(location.c_str(), "r");
        // std::cout << "if location can't open = " << location << "\n";
        if (file != NULL) {
            std::cout << "File " << location << " found\n";
        } else {
            // std::cout << "file == NULL\n";
            generateErrorPage(404);
            return;
        }
    }
}