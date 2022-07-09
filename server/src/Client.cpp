#include "Client.hpp"
#include "Utils.hpp"
#include <errno.h>

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
			res.setContentType(req.getResponceContType());
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
	else if (req.getMethod() == "PUT" || req.getMethod() == "DELETE")
		createOrDelete();
	else if (req.getMethod() == "GET") {
		res.setFileLoc(location);
		res.setContentType(req.getResponceContType());
		res.openFile();
		res.make_response_header(req, statusCode, resCode[statusCode]);
	}
	else if (req.getMethod() == "POST") {
		if (loc->is_cgi_index(location.substr(location.rfind("/") + 1))) {
			res.setFileLoc(location);
			res.setContentType(req.getResponceContType());
			res.createSubprocess(req, location, envp);
			status |= IS_WRITE | IS_CGI;
		}
		else
			redirectPost();
	}
}

void Client::makeResponse( void )
{
	lastActivity = timeChecker();
	if (status & ERROR)
		makeErrorResponse();
	else if (status & AUTOIDX || req.getMethod() == "DELETE" || req.getMethod() == "PUT")
		makeResponseWithoutBody();
	else if (req.getMethod() == "GET")
		makeGetResponse();
	else if (req.getMethod() == "POST") {
		if (status & IS_CGI)
			makePostResponse();
		else
			makeGetResponse();
	}
}

void Client::redirectPost( void ) {
	debug_msg(3, CYAN, "\e[1mNOT CGI: redirected");
	size_t pos = req.getReferer().find(req.getRawHost());
	location = req.getReferer().substr(pos + req.getRawHost().size()); //remove http://host:port
	statusCode = 301;
	status |= REDIRECT;
	res.make_response_html(statusCode, resCode[statusCode], location);
}

void Client::createOrDelete() {
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

Response &		Client::getResponse() { return res; }
Request &		Client::getRequest() { return req; }
size_t			Client::getMaxBodySize() const { return loc->get_client_max_body_size(); }
std::string		Client::getHost() const { return req.getHost(); }
std::string &	Client::getHeader( void ) { return header; }
size_t			Client::getStreamSize( void ) { return reader_size; }
std::stringstream &	Client::getStream() { return reader; }
Server_block *	Client::getServer(void) { return srv; }
time_t			Client::getlastActivity() { return lastActivity; }

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
	lastActivity = timeChecker();
	wrtRet = 0;
	rdRet = 0;
	countr = 0;
	countw = 0;
}

Client::Client(size_t nwsock) {
	lastActivity = timeChecker();
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
