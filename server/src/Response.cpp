#include "Response.hpp"

std::string Response::make_general_header ( Request & req )
{
	std::string contType = "Content-Type: " + _contentType + "\r\n";
	std::string contentLength = "Content-Length: " + _contentLength + "\r\n";
	std::string connection = "keep-alive"; //Connection: keep-alive

	return(
			"Version: " + req.getProtocolVer()  + "\r\n" + 
			contType +
			contentLength +
			"Connection: " + connection +
			+ "\r\n");
}

void Response::make_response_header( Request & req, int code, std::string status, long size ) // https://datatracker.ietf.org/doc/html/rfc2616#section-6
{
	std::string statusLine;
	std::string generalHeader;

	_statusCode = itos(code);
	_reasonPhrase = status;
	if (!size)
		_contentLength = itos(getFileSize(_fileLoc.c_str()));
	else
		_contentLength = itos(size);
	statusLine = req.getProtocolVer() + " " + _statusCode + " " + _reasonPhrase + "\r\n";
	generalHeader = make_general_header(req);
	addCookie(getCurTime());
	_header = statusLine + generalHeader + _cookie;
	_stream << _header;
}

void Response::addCookie(std::string cookie) {
	_cookie = "Set-Cookie: time=" + cookie + "\r\n\r\n";
}

int Response::sendResponse_file(const size_t socket, time_t & lastActivity)
{
	char 			*buffer = new char [RES_BUF_SIZE];

	if(!_file.is_open()) {
		debug_msg(1, RED, "File not open!: has 404 exception");
		throw(codeException(404));
	}
	bzero(buffer, RES_BUF_SIZE);
	_file.read (buffer, RES_BUF_SIZE);
	_bytesRead = _file.gcount();

	_totalBytesRead += _bytesRead;
	_bytesSent = send(socket, buffer, _bytesRead, 0);		// Отправляем ответ клиенту с помощью функции send
	if (_bytesSent)
		lastActivity = timeChecker();
	if (_bytesSent < _bytesRead)
	{
		_totalBytesRead -= (_bytesRead - _bytesSent);
		_file.seekg(_totalBytesRead);
	}
	delete[] buffer;
	if (_file.eof())								//закрываем файл только после того как оправили все содержание файла
	{
		_file.close();
		_file.clear();
		return (1);
	}
	return (0);
}

int Response::sendResponse_stream(const size_t socket, time_t & lastActivity)
{
	char 			*buffer = new char [RES_BUF_SIZE];

	bzero(buffer, RES_BUF_SIZE);
	_stream.read(buffer, RES_BUF_SIZE);
	_bytesRead = _stream.gcount();

	if (!_logged)
		_logged = formHeaderLog(buffer, socket);
	_totalBytesRead += _bytesRead;
	_bytesSent = send(socket, buffer, _bytesRead, 0);		// Отправляем ответ клиенту с помощью функции send
	if (_bytesSent)
		lastActivity = timeChecker();
	if (_bytesSent < _bytesRead)
	{
		_totalBytesRead -= (_bytesRead - _bytesSent);
		_stream.seekg(_totalBytesRead);
	}
	delete[] buffer;
	if (_stream.eof())								//закрываем файл только после того как оправили все содержание файла
	{
		clearStrStream(_stream);
		return (1);
	}
	return (0);
}


char** Response::addCgiVar(Request & req, char **envp) {

	int size = 0;
	int iter = 0;
	std::string tmp;

	for (int i = 0; envp[i]; i++)
		size++;
	size += req.getHeadears().size();
	char **arr = new char * [size + 4];
	for (; envp[iter]; iter++)
		arr[iter] = strdup(envp[iter]);
	std::map<std::string, std::string>::const_iterator it = req.getHeadears().begin();
	for (; iter < size; iter++, it++) {
		tmp = "HTTP_" + (*it).first + "=" + (*it).second;
		arr[iter] = strdup(tmp.c_str());
	}
	tmp = "REQUEST_METHOD=" + req.getMethod();			// REQUEST_METHOD=Post
	arr[iter++] = strdup(tmp.c_str());
	tmp = "SERVER_PROTOCOL=" + req.getProtocolVer();	//SERVER_PROTOCOL=HTTP/1.1
	arr[iter++] = strdup(tmp.c_str());
	tmp = "PATH_INFO=./";						// ??? What is PATH_INFO ???
	arr[iter++] = strdup(tmp.c_str());
	arr[iter++] = 0;
	return arr;
}

char** Response::createArgv(std::vector<std::string> vars) {
	char **av = NULL;
	size_t size = vars.size();
	av = new char*[size + 1];
	for (size_t i = 0; i < size; i++)
		av[i] = strdup(vars[i].c_str());
	av[size] = NULL;
	return av;
}

void 			Response::createSubprocess( Request & req, std::string & path, char **envp) {
	if (TESTER) // for pass tester
		path = "cgi-bin/cgi_tester";
	debug_msg(3, CYAN, "\e[1mCGI ENABLED");
	if (pipe(pipe1)) {
		debug_msg(1, RED, "Pipe1 error: has 500 exception");
		throw(codeException(500));
	}
	if (pipe(pipe2)) {
		debug_msg(1, RED, "Pipe2 error: has 500 exception");
		throw(codeException(500));
	}
	fcntl(pipe1[PIPE_OUT], F_SETFL, O_NONBLOCK);
	fcntl(pipe2[PIPE_IN], F_SETFL, O_NONBLOCK);
	if ((pid = fork()) < 0) {
		debug_msg(1, RED, "Fork error: has 500 exception");
		throw(codeException(500));
	}
	if (pid == 0) { //child - process for CGI programm
		dup2(pipe1[PIPE_IN], 0);
		dup2(pipe2[PIPE_OUT], 1);
		close(pipe1[PIPE_OUT]);
		close(pipe1[PIPE_IN]);
		close(pipe2[PIPE_IN]);
		close(pipe2[PIPE_OUT]);
		std::vector<std::string> vec;
		vec.push_back(path);
		if ((ex = execve(path.c_str(), createArgv(vec), addCgiVar(req, envp))) < 0) {
			std::cerr << "here\n";
			std::cerr << "ex = " << ex << "\n";
			debug_msg(1, RED, "Execve fault: has 500 exception");
			throw(codeException(500));
		}
		exit(ex);
	}
	else { // main process
		close(pipe1[PIPE_IN]);
		close(pipe2[PIPE_OUT]);
	}
}

int Response::waitChild( void ) {
	int wstat = -1;
	pid_t pstat;
	// std::cout << "waitpid\n";
	pstat = waitpid(pid, &wstat, WNOHANG); //WNOHANG
	// std::cout << "pstat = " << pstat << "\n";
	if (pstat > 0) {
		if (wstat > 255)
			wstat /= 255;
		debug_msg(3, "Child process exited with code ", itos(wstat));
		return (1);
	}
	else if (pstat < 0) {
		kill(pid, SIGKILL);
		debug_msg(3, RED, "Child process killed: pid = -1: has 500 exception");
		throw codeException(500);
	}
	return (0);
}

int Response::extractCgiHeader( Request & req )
{
	std::string					headerAll;
	std::vector<std::string>	headerStrs;
	size_t pos, rd, bytesRead = 0;
	char	buf[BUF];
	while (!_stream.eof()) {
		bzero(buf, BUF);
		_stream.read(buf, BUF);
		headerAll += buf;
		rd = _stream.gcount();
		pos = find_2xCRLN(buf, BUF, bytesRead);
		bytesRead += rd;
		if (pos) {
			headerAll = headerAll.substr(0, pos);
			break;
		}
	}
	if (_stream.eof()) {
		clearStrStream(_stream);
		_stream << &buf[pos + 4];
	}
	else
		_stream.seekg(pos + 4);
	headerStrs = split(headerAll, "\r\n");
	req.clearHeaders();
	req.parseMapHeaders(headerStrs, headerStrs.size());
	_contentType = req.getContType();
	return atoi(req.getCgiStatusCode().substr(0, 3).c_str());
}

bool Response::openFile()
{
	_file.open(_fileLoc.c_str(), std::ios::binary|std::ios::in); // open file
	if (!_file.is_open())
		return false;
	return true;
}

void Response::cleaner()
{
	_file.close();
	_file.clear();
	_header.clear();
	_contentType.clear();
	_contentLength.clear();
	_statusCode.clear();
	_reasonPhrase.clear();
	_connection.clear();
	_fileLoc.clear();
	_logPath.clear();
	_bytesRead = 0;
	_bytesSent = 0;
	_totalBytesRead = 0;
	_logged = false;
}

int	&			Response::getPipeWrite( void ) { return pipe1[PIPE_OUT]; }
int	&			Response::getPipeRead( void ) { return pipe2[PIPE_IN]; }
int				Response::getContentLenght() { return(std::atoi(_contentLength.c_str())); }
std::string		Response::getHeader() { return(_header); }
std::string		Response::getResponceContType() { return(_contentType); }
std::string		Response::getReasonPhrase() { return(_reasonPhrase); }
std::string		Response::getFileLoc() { return(_fileLoc); }
std::string		Response::getCookie() { return(_cookie); }
std::ifstream &	Response::getFileStream() { return(_file); }
std::stringstream &	Response::getStrStream() { return(_stream); } 

void			Response::setFileLoc(std::string loc) { _fileLoc = loc; };
void			Response::setContentType(std::string type) { _contentType = type; };
void			Response::setLogPath(std::string path) { _logPath = path; }
