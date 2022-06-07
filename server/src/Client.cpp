#include "Client.hpp"

#define PATH_INFO "cgi_tester"
#define PIPE_IN 1	//we write
#define PIPE_OUT 0	//we read

void	Client::checkConnection( const std::string & mess ) {
	if (mess.find_last_of("\n") != mess.size() - 1)
		breakconnect = true;
	breakconnect = false;
}

void	Client::handleRequest( void ) {
	req.parseText(message);
	parseLocation();
	initResponse();
}

void	Client::handleError( const int code ) {
	std::string mess = "none";
	try {
		mess = resCode.at(code);
	} catch(const std::exception& e) {}
	cleaner();
	res.make_response_error(code, mess);
	statusCode = code;
	status |= ERROR;
	status |= REQ_DONE;
}

void	Client::initResponse ()
{
	if (status & AUTOIDX)
		autoindex(location);
	else {
		res.setFileLoc(location);
		res.setContentType(req.getContentType());
		res.openFile();
		res.make_response_header(req, statusCode, resCode[statusCode]);
	}
	status |= REQ_DONE;
}

void	Client::makeResponse(char **envp) {
		try
		{
			if (status & ERROR)
				makeErrorResponse();
			else if (status & AUTOIDX)
				std::cout << "AUTOINDEX\n";
			else if (req.getMethod() == "GET")
				makeGetResponse();
			else if (req.getMethod() == "POST")
				makePostResponse(envp);
		}
		catch (codeException &e)
		{
			status |= ERROR;
			statusCode = e.getErrorCode();
			std::cout << RED << "Error " <<  statusCode << RESET << "\n";
			initResponse();
		}
}

void	Client::makeErrorResponse() {
	if (res.sendResponse_stream(socket))
		status |= RESP_DONE;
	if (status & RESP_DONE)
		cleaner();
}

void	Client::makeGetResponse()
{
	if (status & HEAD_SENT) {
		if (res.sendResponse_file(socket))
			status |= RESP_DONE;
	}
	else {
		if (res.sendResponse_stream(socket))
			status |= HEAD_SENT;
	}
	if (status & RESP_DONE)
		cleaner();
}

void Client::makePostResponse(char **envp)
{
	pid_t	pid;
	int		pipe1[2];
	int		pipe2[2];
	int		ex;
	int		status;
	std::stringstream reqBody;

	res.addCgiVar(&envp, req);

	reqBody << req.getBody();

	// res._file.open(res.getFileLoc().c_str(), std::ios::binary|std::ios::in);
	// if(!res._file.is_open())
		// throw(codeException(404));

	if (pipe(pipe1) && pipe(pipe2))
		throw(codeException(500));
	if ((pid = fork()) < 0)
		throw(codeException(500));
	if (pid == 0) //child - prosses for CGI programm
	{
		close(pipe1[PIPE_IN]); //Close unused pipe write end
		close(pipe2[PIPE_OUT]); //Close unused pipe read end
		dup2(pipe1[PIPE_OUT], 0);
		dup2(pipe2[PIPE_IN], 1);
		if ((ex = execve(PATH_INFO, NULL, envp)) < 0)
			throw(codeException(500));
		exit(ex);
	}
	else //parent - current programm prosses
	{
		close(pipe1[PIPE_OUT]); //Close unused pipe read end
		close(pipe2[PIPE_IN]); //Close unused pipe write end
		write(pipe1[PIPE_IN], req.getBody().c_str(), req.getBody().length());
		waitpid(pid, &status, 0);
		// res.get
		// res.sendResponse_stream()
	}
}

void	Client::cleaner() {
	message.clear();
	location.clear();
	req.cleaner();
	res._hederHasSent = 0;
	res.cleaner();
	statusCode = 0;
	status = 0;
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
			try {
				vec[i] += "/";
				lctn = srv->lctn.at(vec[i]);
				return (lctn);
			} catch ( const std::exception &e ) {
				i++;
			}
		}
	}
    return NULL; 
}

Client::Client( size_t nwsock ) {
	breakconnect = false;
	location.clear();
	message.clear();
	socket = nwsock;
	status = 0;
	statusCode = 0;
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

Client::~Client() {}

int Client::parseLocation() {
	if (req.getMIMEType() == "none") {
		// std::cout << "IS_DIR\n";
        status |= IS_DIR;
	} else
		status |= IS_FILE;
	Location_block *loc = getLocationBlock(req.getDirs());
	if (loc == NULL)
		throw codeException(404);
	if (loc->get_redirect().first)
		makeRedirectPesponse(loc->get_redirect().first, loc->get_redirect().second);
	size_t pos;
	std::string root = loc->get_root();
	std::string	locn = loc->get_location();
	std::string method = "none";
	std::string tmp;
	std::cout << RED << locn << RESET << "\n";
	for (size_t i = 0; i != loc->get_accepted_methods().size(); i++) {
		if (req.getMethod() == loc->get_accepted_methods()[i]) {
			method = loc->get_accepted_methods()[i];
			break;
		}
	}
	if (method == "none") {
		statusCode = 405;
		std::cout << "statusCode - " << statusCode << "\n";
		throw codeException(405);
	}
	if (loc->get_client_max_body_size() < req.getReqSize()) {
		statusCode = 413;
		throw codeException(413);
	}
	if (locn.size() > 1 && (pos = root.find(locn)) != std::string::npos)
		root = root.substr(0, pos);
		std::cout << YELLOW << root << RESET << "\n";
	location = root + locn + req.getReqURI().substr(locn.size());
	while ((pos = location.find("//")) != std::string::npos)
		location.erase(pos, 1);
	if (location[0] == '/')
		location = location.substr(1);
	std::cout << GREEN << "this is location - " << location << " <-\n" << RESET;
	if (status & IS_DIR) {
			if (location.size() && location[location.size()-1] != '/') {
				statusCode = 301;
				location.push_back('/');
			}
			std::vector<std::string>indexes = loc->get_index();
			int i = -1;
			if (!loc->get_autoindex()) {
				while (++i < indexes.size()) {
					std::string tmp = location + indexes[i];
					if (access(tmp.c_str(), 0) != -1) {
						location = tmp;
						req.setMIMEType(indexes[i]);
						break;
					}
				}
			}
			else {
				if (access(location.c_str(), 0) == -1) {
					std::cout << location << " - access(location.c_str(), 0) == -1 IS_DIR\n";
					throw codeException(404);
				}
				status |= AUTOIDX;
			}
			if (i == indexes.size()) {
				throw codeException(404);
			}
	} else if (status & IS_FILE) {
		if (access(location.c_str(), 0) == -1) {
			std::cout << location << " - access(location.c_str(), 0) == -1 IS_FILE\n";
			throw codeException(404);
		}
    }
	if (access(location.c_str(), 4) == -1)
		throw codeException(403);
	if (statusCode != 301)
		statusCode = 200;
	return (0);
}

void Client::makeRedirectPesponse(int code, std::string loc) {

}