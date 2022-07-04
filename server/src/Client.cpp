#include "Client.hpp"
#include "Utils.hpp"
#include <errno.h>


void proc_exit(int) {
	int wstat;
	pid_t	pid;
	pid = waitpid(P_ALL, &wstat, 0);
	std::cout << "Child process exited " << wstat << std::endl;
	signal(SIGCHLD, SIG_DFL);
}


void Client::clearStream( void ) {
	reader.seekg(0);
	reader.str(std::string());
	reader.clear();
	reader_size = 0;
}

void Client::savePartOfStream( size_t pos ) {
	std::cout << RED << pos + 4 << " != " << reader_size << " tail detected" << RESET << "\n";
	char buf[reader_size - pos - 3];
	bzero(buf, reader_size - pos - 3);
	reader.seekg(pos + 4);
	reader.read(buf, reader_size - pos - 4);
	buf[reader_size - pos - 3] = 0;
	std::cout << "readed: "<< reader.gcount() << "\n";
	reader_size = reader.gcount();
	reader.str(std::string()); // clearing content in stream
	reader.clear();
	reader << buf;
	// std::cout << YELLOW << header.substr(pos + 4) << RESET << "\n";
	// std::cout << GREEN << buf << RESET << "\n";
}

void Client::checkMessageEnd( void ) {

	if (status & IS_BODY)
	{
		// std::cout << BLUE << "Transfer-Encoding: " << req.getTransferEnc() << RESET << "\n";
		// std::cout << BLUE << "Content-Length: " << req.getСontentLenght() << RESET << "\n";

		if (req.getTransferEnc() == "chunked")
		{
			char buf[6];
			bzero(buf, 5);
			reader.seekg(reader_size - 5);
			reader.read(buf, 5);
			reader.seekg(0);
			std::cout << "reader_size = " << reader_size << "\n";
			// std::cout << "start bytes print: " << "\n";
			// for (size_t i = 0; i < 5; i++)
			// {
			// 	printf("%d ", buf[i]);
			// }
			// std::cout << "\n";
			// std::cout << "end bytes print\n";

			if (buf[0] == '0' && buf[1] == '\r' && buf[2] == '\n'
				&& buf[3] == '\r' && buf[4] == '\n')
				fullpart = true;
			else
				fullpart = false;
		}
		else if (req.getContentLenght().size())
		{
			size_t len = atoi(req.getContentLenght().c_str());
			std::cout <<  CYAN << "reader_size = " << reader_size << " " << "len = " << len << RESET << "\n";
			if (reader_size >= len)
				fullpart = true;
			// else if (reader_size > len)
			// 	throw codeException(400);
			else
				fullpart = false;
		}
		else {
			std::cout << RED << "I cant work with this body Encoding" << RESET << "\n";
			fullpart = true;
			//throw codeException(400);
		}
		// ЕСЛИ Transfer-Encoding ждем 0
		// ECЛИ Content-length ждем контент лен
	}
	else
	{
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

void Client::handleRequest(char **envp)
{
	if (status & IS_BODY) {
		std::cout << CYAN << "\nPARSE BODY 1" << RESET << "\n";
		req.parseBody(reader, reader_size, envpVector);
		reader_size = getStrStreamSize(reader);
		status |= REQ_DONE;
		// std::cout << GREEN << "REQ_DONE with body" << RESET << "\n";
	}
	else
	{
		bool rd = req.parseText(header);
		if (rd == true) // exist body
		{
			// std::cout << PURPLE << "IS_BODY" << RESET << "\n";
			status |= IS_BODY;
			if (reader_size) {
				checkMessageEnd();
				if (fullpart) {
					std::cout << CYAN << "\nPARSE BODY 2" << RESET << "\n";
					req.parseBody(reader, reader_size, envpVector);
					reader_size = getStrStreamSize(reader);
					status |= REQ_DONE;
				}
			}
			else
				fullpart = false;
		}
		else {
			// std::cout << GREEN << "REQ_DONE without body" << RESET << "\n";
			status |= REQ_DONE;
		}
	}
}

int Client::searchErrorPages()
{
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
	if (!file)	{
		std::string mess = "none";
		try		{
			mess = resCode.at(code);
		}
		catch (const std::exception &e) {}
		res.make_response_html(code, mess); 
	}
	cleaner();
	statusCode = code;
	status |= ERROR;
	status |= REQ_DONE;
	if (file)
		status |= IS_FILE;
}

void Client::initResponse(char **envp)	{
	if (status & AUTOIDX)
		res.make_response_autoidx(req, location, statusCode, resCode[statusCode]);
	else if (status & REDIRECT)
		res.make_response_html(statusCode, resCode[statusCode], location); //TODO: 
		// res.make_response_header(req, statusCode, resCode[statusCode], 1);
	else if (req.getMethod() == "PUT")
		res.make_response_html(statusCode, resCode[statusCode], location); //TODO: 
	else {
		res.setFileLoc(location);
		res.setContentType(req.getContentType());
		res.openFile();
		res.make_response_header(req, statusCode, resCode[statusCode]);
	}
	if (req.getMethod() == "POST")
	{
		iter = 0;
		cgiDone = false;
		totalSent = 0;
		res.addCgiVar(&envp, req, envpVector);

		if (pipe(pipe2))
		{
			std::cout << RED << "HERE0" << RESET << "\n";
			throw(codeException(500));
		}
		if (pipe(pipe1))
		{
			std::cout << RED << "HERE1" << RESET << "\n";
			throw(codeException(500));
		}
		fcntl(pipe1[PIPE_OUT], F_SETFL, O_NONBLOCK);
		fcntl(pipe2[PIPE_IN], F_SETFL, O_NONBLOCK);

		size_t pos = location.rfind("/");
		std::string fileName = location.substr(pos + 1);
		std::cout << "fileName = " << fileName << "\n";
		if (loc->is_cgi_index(fileName))
		{
			clearStrStream(res.getStrStream()); //очищаем от записанного ранее хедера, который далеепридется переписать из-за cgi
			if ((pid = fork()) < 0)
			{
			std::cout << RED << "HERE2" << RESET << "\n";
				throw(codeException(500));
			}
			if (pid == 0) //child - process for CGI programm
			{
				// close(pipe1[PIPE_IN]); //Close unused pipe write end
				// close(pipe2[PIPE_OUT]); //Close unused pipe read end
				// dup2(pipe1[PIPE_OUT], 0);
				// dup2(pipe2[PIPE_IN], 1);
				// close(pipe1[PIPE_OUT]);
				// close(pipe2[PIPE_IN]);

				dup2(pipe1[PIPE_IN], 0);
				dup2(pipe2[PIPE_OUT], 1);
				
				close(pipe1[PIPE_OUT]);
				close(pipe1[PIPE_IN]);
				
				// close(pipe2[PIPE_IN]);
				// close(pipe2[PIPE_OUT]);

				if ((ex = execve(CGI_PATH, NULL, envp)) < 0)
				{
					if (!envp)
						std::cerr << "ENVP DOES NOE EXIST\n";
					char buffer[100];
					std::cerr << RED << "Execve fault: has 500 exception" << RESET << "\n";
					strerror_r( errno, buffer, 256 ); // to remove
					std::cerr << "ERRNO: " << buffer << "\n"; // to remove
					throw(codeException(500));
					exit (20);
				}
				// close(pipe1[PIPE_OUT]); //Closing remaining fds before closing child process
				// close(pipe2[PIPE_IN]); //Closing remaining fds before closing child process
				exit(ex);
			}
			else
			{
				// close(pipe1[PIPE_OUT]); //Close unused pipe read end
				// close(pipe2[PIPE_IN]); //Close unused pipe write end
				signal(SIGCHLD, proc_exit);
				close(pipe1[PIPE_IN]);
				close(pipe2[PIPE_OUT]);
			}
		}
		else
			std::cout << "NOT CGI\n";
	}
	// status |= REQ_DONE;
}

void Client::makeResponse(char **envp)
{ // envp не нужен уже
	if (status & ERROR)
		makeErrorResponse();
	else if (status & AUTOIDX)
		makeAutoidxResponse();
	else if (req.getMethod() == "GET")
		makeGetResponse();
	else if (req.getMethod() == "POST")
		makePostResponse(envp);
	else if (req.getMethod() == "DELETE")
		makeDeleteResponse(envp);
	else if (req.getMethod() == "PUT")
		makePutResponse(envp);
	// 	makePostResponse(envp);  //envp не нужен уже
}

void Client::makeAutoidxResponse()
{
	if (res.sendResponse_stream(socket))
		status |= RESP_DONE;
	if (status & RESP_DONE)
	{
		// std::cout << GREEN << "End AUTOINDEX response on " << socket << " socket" << RESET << "\n";
		cleaner();
	}
}

void Client::makeErrorResponse()
{
	if (status & HEAD_SENT)
	{
		if (status & IS_FILE)
		{
			if (res.sendResponse_file(socket))
				status |= RESP_DONE;
		}
		else
			status |= RESP_DONE;
	}
	else
	{
		if (res.sendResponse_stream(socket))
			status |= HEAD_SENT;
	}
	if (status & RESP_DONE)
	{
		// std::cout << GREEN << "End ERROR response on " << socket << " socket" << RESET << "\n";
		cleaner();
	}
}



void Client::extractCgiHeader( char * buff )
{
	clearStrStream(res.getStrStream());

	std::string					tmp, tmp2;
	std::vector<std::string>	headerAndBody;
	std::vector<std::string>	headerStrs;
	// int						code;

	tmp = buff;
	headerAndBody = split(tmp, "\r\n\r\n", "");
	headerStrs = split(headerAndBody.at(0), "\r\n", "");

	req.clearHeaders();
	req.parseMapHeaders(headerStrs, headerStrs.size());
	tmp.clear();
	tmp = req.getCgiStatusCode();
	tmp2 = tmp.substr(0, 3);
	req.setCgiStatusCode(tmp2);
	res.setStatusCode(tmp2);
	res.setContentType(req.getContType());

	// code = std::atoi(req.getCgiStatusCode().c_str());
	// res.make_response_header(req, code, resCode[code]);

	//отправка body оставшаяся в буффере после первого прочтения из пайпа
	if (headerAndBody.size() > 1)
		res.getStrStream().write(headerAndBody.at(1).c_str(), headerAndBody.at(1).length()); //записываем кусок body который попал в буффер вместе с хедером от cgi
	
	// res.sendResponse_stream(socket); //не тут должно быть 
}

void Client::cleaner()
{
	if (status & ERROR)
		std::cout << GREEN << "Complete working with error: \e[1m" << statusCode << " " << resCode[statusCode] << "\e[0m\e[32m on \e[1m" << socket << "\e[0m\e[32m socket" << RESET << "\n";
	else if (status & RESP_DONE)
		std::cout << GREEN << "Complete working with request: \e[1m" << req.getMethod() << " with code " << statusCode << "\e[0m\e[32m on \e[1m" << socket << "\e[0m\e[32m socket" << RESET << "\n";
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
}

void Client::setStream( const std::stringstream & mess, const size_t size ) {
	reader.str(std::string());
	reader.clear();
	reader << mess.rdbuf();
	reader_size = size;	
}
void Client::setServer(Server_block *s)
{
	srv = s;
	this->loc = getLocationBlock(req.getDirs());
	if (loc == NULL)
	{
		std::cout << RED << "Location not found: has 404 exception " << RESET << "\n";
		throw codeException(404);
	}
}

void Client::setClientTime(time_t t) { time = t; }
void Client::setLastTime(time_t t) { lastTime = t; }

bool			Client::readComplete() const { return fullpart; }
Response &		Client::getResponse() { return res; }
Request &		Client::getRequest() { return req; }
size_t			Client::getMaxBodySize() const { return loc->get_client_max_body_size(); }
std::string		Client::getHost() const { return req.getHost(); }
std::string &	Client::getHeader( void ) { return header; }
size_t			Client::getStreamSize( void ) { return reader_size; }
std::stringstream &	Client::getStream() { return reader; }
Server_block *	Client::getServer(void) { return srv; }
int *			Client::getPipe1() { return pipe1; }
int *			Client::getPipe2() { return pipe2; }
time_t			Client::getClientTime() { return time; }
time_t			Client::getLastTime() { return lastTime; }

Location_block *Client::getLocationBlock(std::vector<std::string> vec) const
{
	Location_block *lctn;
	size_t pos, i = 0;
	while (i < vec.size())
	{
		try
		{
			lctn = srv->lctn.at(vec[i]);
			return (lctn);
		}
		catch (const std::exception &e)
		{
			try
			{
				vec[i] += "/";
				lctn = srv->lctn.at(vec[i]);
				return (lctn);
			}
			catch (const std::exception &e)
			{
				i++;
			}
		}
	}
	return NULL;
}

Client::Client(size_t nwsock)
{
	time = timeChecker();
	lastTime = 0;
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
	res.setCookie("");
}

int Client::parseLocation()	{
	statusCode = 200;
	if (req.getMIMEType() == "none")
		status |= IS_DIR;
	else
		status |= IS_FILE;
	if (req.getMethod() != "DELETE" && loc->get_redirect().first && !(status & REDIRECT)) {
		if (makeRedirect(loc->get_redirect().first, loc->get_redirect().second)) {
			std::cout << CYAN << "REDIRECT" << RESET << "\n";
			location = req.getReqURI();
			// std::cout << "location after makeRedirect - " << location << "\n";
			return 0;
		}
	}
	size_t pos;
	std::string root = loc->get_root();
	std::string locn = loc->get_location();
	// if (locn[locn.size() - 1] != '/')
	// 	locn += "/";
	if (!loc->is_accepted_method(req.getMethod()))
	{
		std::cout << RED << "Method: " << req.getMethod() << " has 405 exception " << RESET << "\n";
		throw codeException(405);
	}
	size_t subpos;
	locn[locn.size() - 1] == '/' ? subpos = locn.size() - 1 : subpos = locn.size();
	// if (req.getReqURI().find("http") == std::string::npos) {
		location = root + locn + req.getReqURI().substr(subpos);
	// else 
		// location = req.getReqURI();
	// FOR INTRA TESTER
	std::cout << CYAN << "this is loc = " << location << "\n" << RESET;
	if (location.find("directory") != std::string::npos) {
		location.erase(location.find("directory"), 10);
		std::cout << RED << "\e[1m  ALERT! tester stick trim /directory/" << RESET << "\n";
	}
	std::cout << location << '\n';
	// DELETE IT IN FINAL VERSION!

	while ((pos = location.find("//")) != std::string::npos)
		location.erase(pos, 1);
	if (location.size() > 1 && location[0] == '/')
		location = location.substr(1);
	if (req.getMethod() == "PUT")
		return 1;
	if (status & IS_DIR)	{
		if (location.size() && location[location.size() - 1] != '/')	{
			// statusCode = 301; // COMMENT IT FOR TESTER
			// location = req.getReqURI() + "/"; // COMMENT IT FOR TESTER
			location += "/"; // DELETE COMMENT IT FOR TESTER
			status |= REDIRECT;
			// return 0; // COMMENT IT FOR TESTER
		} 	//else	{ // COMMENT ELSE { FOR TESTER
			std::vector<std::string> indexes = loc->get_index();
			int i = -1;
			if (!loc->get_autoindex())	{
				while (++i < indexes.size())	{
					std::string tmp = location + indexes[i];
					if (access(tmp.c_str(), 0) != -1)	{
						location = tmp;
						req.setMIMEType(indexes[i]);
						break;
					}
					std::cout << tmp << "\n";
				}
				if (i == indexes.size())	{
					std::cout << RED << "Not found index in directory: " << location << RESET << "\n";
					throw codeException(404);
				}
			}
			else	{
				if (access(location.c_str(), 0) == -1)		{
					std::cout << RED << "No such directory: " << location << RESET << "\n";
					throw codeException(404);
				}
				status |= AUTOIDX;
			}
	//	} // COMMENT IT FOR TESTER
	}
	else if (status & IS_FILE)	{ // FILE
		if (access(location.c_str(), 0) == -1)	{
			std::cout << RED << "File not found (IS_FILE): " << location << RESET << "\n";
			throw codeException(404);
		}
	}
	if (access(location.c_str(), 4) == -1)	{
		std::cout << RED << "Permisson denied: " << location << RESET << "\n";
		throw codeException(403);
	}
	std::cout << GREEN << "this is final location: " << location << " <-\n" << RESET;
	return (0);
}

int Client::makeRedirect(int code, std::string loc){
	status |= REDIRECT;
	// std::cout << "code - " << code << ", loc - " << loc << "\n";
	statusCode = code;
	req.splitLocation(loc);
	req.splitDirectories();
	return 1;
}

int Client::checkTimeout(long bytesRead) {
	// std::cout << GREEN << "bytesRead: " << bytesRead << " == reader_size: " << reader_size << "" << RESET << "\n";
	if (bytesRead == reader_size && (lastTime - time) > TIMEOUT)
			// std::cout << RED << "Timeout: " << time << " > " << TIMEOUT << " - client disconnected" << RESET << "\n";
			return 0;
    lastTime = timeChecker();
	// std::cout << RED << "Time: " << time << "- lastTime:  " << lastTime << " = " << (lastTime - time) << " < "<< TIMEOUT << RESET << "\n";
	return 1;
}

void Client::checkTimeout2(long bytesRead) {
	if (bytesRead == reader_size && time > TIMEOUT) {
		std::cout << RED << "Timeout: client disconnected" << RESET << "\n";
        throw codeException(408);
    }
    time = timeChecker();
	// return 1;
}