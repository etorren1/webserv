#include "Client.hpp"
#include "Utils.hpp"
#include <errno.h>

void Client::clearStream( void ) {
	reader.seekg(0);
	reader.str(std::string());
	reader.clear();
	reader_size = 0;
}

void Client::savePartOfStream( size_t pos ) {
	char buf[reader_size - pos - 3];
	bzero(buf, reader_size - pos - 3);
	reader.seekg(pos + 4);
	reader.read(buf, reader_size - pos - 4);
	buf[reader_size - pos - 3] = 0;
	reader_size = reader.gcount();
	reader.str(std::string()); // clearing content in stream
	reader.clear();
	reader << buf;
}

void Client::checkMessageEnd( void ) {
	if (status & IS_BODY) {
		if (req.getTransferEnc() == "chunked") {
			if (reader_size > 4) {
				char buf[5];
				bzero(buf, 5);
				reader.seekg(reader_size - 5);
				reader.read(buf, 5);
				reader.seekg(0);
				size_t pos = find_2xCRLN(buf, 5);
				if (pos && buf[pos - 1] == '0')
					fullpart = true;
				else
					fullpart = false;
			}
		}
		else if (req.getContentLenght().size()) {
			size_t len = atoi(req.getContentLenght().c_str());
			if (reader_size >= len)
				fullpart = true;
			else
				fullpart = false;
		}
		else {
			fullpart = true;
		}
	}
	else {
		header = reader.str();
		size_t pos = header.find("\r\n\r\n");
		if (pos != std::string::npos) {
			if (pos + 4 != reader_size) // part of body request got into the header
				savePartOfStream(pos);
			else
				clearStream();
			header.erase(pos + 4);
			while (header.find("\r") != std::string::npos) {
				header.erase(header.find("\r"), 1); // из комбинации CRLF
				pos--;								// Удаляем символ возврата карретки
			}
			fullpart = true;
		}
		else
			fullpart = false;
	}
}

void Client::handleRequest( void ) {
	if (status & IS_BODY) {
		req.parseBody(reader, reader_size, envpVector);
		status |= REQ_DONE;
	}
	else {
		bool rd = req.parseText(header);
		if (rd == true) { // body exist
			status |= IS_BODY;
			if (reader_size) {
				checkMessageEnd();
				if (fullpart) {
					req.parseBody(reader, reader_size, envpVector);
					status |= REQ_DONE;
				}
			}
			else
				fullpart = false;
		}
		else
			status |= REQ_DONE;
	}
}

int Client::searchErrorPages() {
	size_t pos;
	std::string tmp;
	while ((pos = location.find_last_of("/")) != std::string::npos) {
		location = location.substr(0, pos);
		tmp = location + loc->get_error_page().second;
		res.setFileLoc(tmp);
		if (res.openFile())
			return 1;
	}
	return 0;
}

void Client::handleError( const int code ) {
	int file = 0;
	if (loc && loc->get_error_page().first == code)	{
		if ((file = searchErrorPages()) == 1) {
			req.setMIMEType(loc->get_error_page().second);
			res.setContentType(req.getContentType());
			res.make_response_header(req, code, resCode[code]);
		}
	}
	if (!file) {
		std::string mess = "none";
		try	{
			mess = resCode.at(code);
		}
		catch (const std::exception &e) {}
		res.make_response_html(code, mess); 
	}
	status = 0;
	statusCode = code;
	status |= ERROR;
	status |= REQ_DONE;
	if (file)
		status |= IS_FILE;
}

void Client::initResponse(char **envp) {
	if (status & AUTOIDX)
		res.make_response_autoidx(req, location, statusCode, resCode[statusCode]);
	else if (status & REDIRECT)
		res.make_response_html(statusCode, resCode[statusCode], location);
	else if (req.getMethod() == "PUT" || req.getMethod() == "DELETE") {
		if (req.getMethod() == "PUT") {
			std::ofstream file(location);
			if (!file.is_open()) {
				size_t sep = location.find_last_of("/");
				if (sep != std::string::npos) {
					rek_mkdir(location.substr(0, sep));
				}
				file.open(location);
			}
			if (file.is_open()) {
				file << reader.str();
				file.close();
			} else {
				throw codeException(406);
			}
			statusCode = 201;
		}
		if (req.getMethod() == "DELETE") {
			if (remove(location.c_str()) != 0) 
				codeException(403);
			statusCode = 204;
		}
		res.setFileLoc(location);
		clearStrStream(res.getStrStream());
		res.make_response_html(statusCode, resCode[statusCode], location);
	}
	else if (req.getMethod() == "GET") {
		res.setFileLoc(location);
		res.setContentType(req.getContentType());
		res.openFile();
		res.make_response_header(req, statusCode, resCode[statusCode]);
	}
	else if (req.getMethod() == "POST") {
		res.setContentType(req.getContentType());
		if (loc->is_cgi_index(location.substr(location.rfind("/") + 1))) {
			res.createSubprocess(req, envp);
			status |= IS_WRITE;
		}
		else {
			// size_t pos = location.rfind("/");
			// location = location.substr(0, pos);
			// parseLocation();
			// res.setFileLoc(location);
			// res.setContentType(req.getContentType());
			// res.openFile();
			// req.setMethod("GET");
			// res.make_response_header(req, statusCode, resCode[statusCode]);
			;
		}
	}
}

void Client::makeResponse( void )
{
	lastTime = timeChecker();
	if (status & ERROR)
		makeErrorResponse();
	else if (status & AUTOIDX || req.getMethod() == "DELETE" || req.getMethod() == "PUT")
		makeResponseWithoutBody();
	else if (req.getMethod() == "GET")
		makeGetResponse();
	else if (req.getMethod() == "POST")
		makePostResponse();
}

void Client::makeResponseWithoutBody()
{
	if (res.sendResponse_stream(socket))
		status |= RESP_DONE;
	if (status & RESP_DONE)
	{
		cleaner();
	}
}

void Client::cleaner() {
	if (status & ERROR)
		debug_msg(1, GREEN,  "Complete working with error: \e[1m", itos(statusCode), " ", resCode[statusCode], "\e[0m\e[32m on \e[1m", itos(socket), "\e[0m\e[32m socket");
	else if (status & RESP_DONE)
		debug_msg(1, GREEN,  "Complete working with request: \e[1m", req.getMethod(), " with code ", itos(statusCode), "\e[0m\e[32m on \e[1m", itos(socket), "\e[0m\e[32m socket");
	clearStream();
	location.clear();
	header.clear();
	envpVector.clear();
	req.cleaner();
	res.cleaner();
	statusCode = 0;
	status = 0;
	loc = NULL;
	srv = NULL;
	time = timeChecker();
	lastTime = 0;
	wrtRet = 0;
	rdRet = 0;
	countr = 0;
	countw = 0;
}

void Client::setStreamSize( const size_t size ) {
	reader_size = size;
}

void Client::setServer(Server_block *s) {
	srv = s;
	this->loc = getLocationBlock(req.getDirs());
	if (loc == NULL) {
		debug_msg(1, RED, "Location not found: has 404 exception");
		throw codeException(404);
	}
	res.setLogPath(srv->get_access_log());
}

void Client::setClientTime(time_t t) { time = t; }
void Client::setLastTime(time_t t) { lastTime = t; }

Response &		Client::getResponse() { return res; }
Request &		Client::getRequest() { return req; }
size_t			Client::getMaxBodySize() const { return loc->get_client_max_body_size(); }
std::string		Client::getHost() const { return req.getHost(); }
std::string &	Client::getHeader( void ) { return header; }
size_t			Client::getStreamSize( void ) { return reader_size; }
std::stringstream &	Client::getStream() { return reader; }
Server_block *	Client::getServer(void) { return srv; }
time_t			Client::getClientTime() { return time; }
time_t			Client::getLastTime() { return lastTime; }

bool			Client::readComplete() {
	lastTime = timeChecker();
	return fullpart;
}

Location_block *Client::getLocationBlock(std::vector<std::string> vec) const {
	Location_block *lctn;
	size_t i = 0;
	while (i < vec.size()) {
		try {
			lctn = srv->lctn.at(vec[i]);
			return (lctn);
		}
		catch (const std::exception &e) {
			try {
				vec[i] += "/";
				lctn = srv->lctn.at(vec[i]);
				return (lctn);
			}
			catch (const std::exception &e) {
				i++;
			}
		}
	}
	return NULL;
}

Client::Client(size_t nwsock) {
	time = timeChecker();
	lastTime = timeChecker();
	fullpart = false;
	location.clear();
	header.clear();
	reader_size = 0;
	socket = nwsock;
	status = 0;
	statusCode = 0;
	srv = NULL;
	loc = NULL;
	envpVector.clear();
	wrtRet = 0;
	rdRet = 0;
	//Для POST браузер сначала отправляет заголовок, сервер отвечает 100 continue, браузер
	// отправляет данные, а сервер отвечает 200 ok (возвращаемые данные).
	this->resCode.insert(std::make_pair(100, "Continue"));
	this->resCode.insert(std::make_pair(101, "Switching Protocols"));
	this->resCode.insert(std::make_pair(200, "OK"));
	this->resCode.insert(std::make_pair(201, "Created"));
	this->resCode.insert(std::make_pair(202, "Accepted"));
	this->resCode.insert(std::make_pair(203, "Non-Authoritative Information"));
	this->resCode.insert(std::make_pair(204, "No Content"));
	this->resCode.insert(std::make_pair(300, "Multiple Choices"));
	this->resCode.insert(std::make_pair(301, "Moved Permanently"));
	this->resCode.insert(std::make_pair(302, "Found"));
	this->resCode.insert(std::make_pair(303, "See Other"));
	this->resCode.insert(std::make_pair(304, "Not Modified"));
	this->resCode.insert(std::make_pair(305, "Use Proxy"));
	this->resCode.insert(std::make_pair(307, "Temporary Redirect"));
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
	this->resCode.insert(std::make_pair(410, "Gone"));
	this->resCode.insert(std::make_pair(411, "Length Required"));
	this->resCode.insert(std::make_pair(412, "Precondition Failed"));
	this->resCode.insert(std::make_pair(413, "Request Entity Too Large"));
	this->resCode.insert(std::make_pair(414, "Request-URI Too Long"));
	this->resCode.insert(std::make_pair(415, "Unsupported Media Type"));
	this->resCode.insert(std::make_pair(500, "Internal Server Error"));
	this->resCode.insert(std::make_pair(501, "Not Implemented"));
	this->resCode.insert(std::make_pair(502, "Bad Gateway"));
	this->resCode.insert(std::make_pair(503, "Service Unavailable"));
	this->resCode.insert(std::make_pair(504, "Gateway Timeout"));
}

Client::~Client() {
}

int Client::parseLocation()	{
	statusCode = 200;
	if (req.getMIMEType() == "none" && !req.getContType().size())
		status |= IS_DIR;
	else
		status |= IS_FILE;
	if (req.getMethod() != "DELETE" && loc->get_redirect().first && !(status & REDIRECT)) {
		if (makeRedirect(loc->get_redirect().first, loc->get_redirect().second)) {
			location = req.getReqURI();
			return 0;
		}
	}
	size_t pos;
	std::string root = loc->get_root();
	std::string locn = loc->get_location();
	if (!loc->is_accepted_method(req.getMethod()))
	{
		debug_msg(1, RED, "Method: ", req.getMethod(), " has 405 exception");
		throw codeException(405);
	}
	size_t subpos;
	locn[locn.size() - 1] == '/' ? subpos = locn.size() - 1 : subpos = locn.size();
	location = root + locn + req.getReqURI().substr(subpos);

	if (TESTER) {
		// FOR INTRA TESTER
		if (location.find("directory") != std::string::npos) {
			location.erase(location.find("directory"), 10);
		}
		// DELETE IT IN FINAL VERSION!
	}

	while ((pos = location.find("//")) != std::string::npos)
		location.erase(pos, 1);
	if (location.size() > 1 && location[0] == '/')
		location = location.substr(1);
	if (req.getMethod() == "PUT")
		return 1;
	if (status & IS_DIR)	{
		// if (location.size() && location[location.size() - 1] != '/')	{ // FINAL IF
		if (location.size() && location[location.size() - 1] != '/' && !TESTER)	{
			statusCode = 301;
			location = req.getReqURI() + "/"; 
			status |= REDIRECT;
			return 0;
		} 
		else	
		{
			if (TESTER && location.size() && location[location.size() - 1] != '/') // FOR TESTER
				location += "/"; // FOR TESTER
			std::vector<std::string> indexes = loc->get_index();
			size_t i = -1;
			if (!loc->get_autoindex())	{
				while (++i < indexes.size()) {
					// std::cout << "loc - " << location << " index - " << indexes[i] << "\n";
					std::string tmp = location + indexes[i];
					if (access(tmp.c_str(), 0) != -1)	{
						location = tmp;
						req.setMIMEType(indexes[i]);
						break;
					}
					// std::cout << "i = " << i << " " << tmp << "\n";
				}
				if (i == indexes.size())	{
					debug_msg(1, RED, "Not found index in directory: ", location);
					throw codeException(404);
				}
			}
			else	{
				if (access(location.c_str(), 0) == -1)		{
					debug_msg(1, RED, "No such directory: ", location);
					throw codeException(404);
				}
				status |= AUTOIDX;
			}
		}
	}
	else if (status & IS_FILE)	{ // FILE
		if (access(location.c_str(), 0) == -1)	{
			debug_msg(1, RED, "File not found (IS_FILE): ", location);
			throw codeException(404);
		}
	}
	if (access(location.c_str(), 4) == -1)	{
		debug_msg(1, RED, "Permisson denied: ", location);
		throw codeException(403);
	}
	// std::cout << GREEN << "this is final location: " << location << " <-\n" << RESET;
	return (0);
}

int Client::makeRedirect(int code, std::string loc){
	status |= REDIRECT;
	statusCode = code;
	req.splitLocation(loc);
	req.splitDirectories();
	return 1;
}

int Client::checkTimeout( void ) {
    time = timeChecker();
	if ((time - lastTime) > TIMEOUT)
		return 1;
	return 0;
}
