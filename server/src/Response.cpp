#include "Response.hpp"

std::string Response::make_general_header ( Request & req, int statusCode )
{
	// std::string Server = "webserv";
	// _date = getTime();

	// std::string location;
	std::string contType = "Content-Type: " + _contentType + "\r\n";
	std::string contentLength = "Content-Length: " + _contentLength + "\r\n";
	std::string connection = "keep-alive"; //Connection: keep-alive
	// if (statusCode == 301)
	// {
	// 	location = "Location: http://" + req.getHost() + req.getReqURI() + "/\r\n";
	// 	// std::string Last-Modified: Sun, 22 May 2022 13:32:52 GMT
	// 	contType.clear();
	// 	contentLength.clear();
	// 	contentLength = "Content-Length: 0\r\n";
	// }

	return(
			"Version: " + req.getProtocolVer()  + "\r\n" + 
			// "Server: " + Server + "\r\n" +
			// location +
			contType +
			contentLength +
			"Connection: " + connection +// "\r\n" +
			// Transfer-Encoding:
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
	std::cout << GREEN << "\e[1msize = " << size << RESET << "\n";
	std::cout << GREEN << "\e[1mitos(size) = " << itos(size) << RESET << "\n";
	statusLine = req.getProtocolVer() + " " + _statusCode + " " + _reasonPhrase + "\r\n";
	generalHeader = make_general_header(req, code);
	addCookie(getCurTime());
	_header = statusLine + generalHeader + _cookie;
	_stream << _header;
	
	//std::cout << RED << _header << RESET;
}

void Response::addCookie(std::string cookie) {
	_cookie = "Set-Cookie: time=" + cookie + ";\r\n\r\n";
}

int Response::sendResponse_file(const size_t socket)
{
	char 			*buffer = new char [RES_BUF_SIZE];

	if(!_file.is_open()) {
		std::cout << RED << "File not open!: has 404 exception" << RESET << "\n";
		throw(codeException(404));
	}

	bzero(buffer, RES_BUF_SIZE);
	_file.read (buffer, RES_BUF_SIZE);
		_bytesRead = _file.gcount();
	
	_totalBytesRead += _bytesRead;

	_bytesSent = send(socket, buffer, _bytesRead, 0);		// Отправляем ответ клиенту с помощью функции send
	// if (_bytesSent == -1)
	// {
	// 	std::cerr << "wrote = " << _bytesSent << std::endl;
		// std::cout << strerror(errno);
		// std::cout << errno;
	// 	throw codeException(502);
	// }
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

int Response::sendResponse_stream(const size_t socket)
{
	char 			*buffer = new char [RES_BUF_SIZE];

	bzero(buffer, RES_BUF_SIZE);
	_stream.read(buffer, RES_BUF_SIZE);
	_bytesRead = _stream.gcount();
	
	_totalBytesRead += _bytesRead;

	_bytesSent = send(socket, buffer, _bytesRead, 0);		// Отправляем ответ клиенту с помощью функции send
	
	// if (wrRet == 100000) {
	// 	usleep (1000);
	// 	std::cout << RED << buffer << RESET << "\n"; 
	// 	wrRet = 0;
	// }
	// if (_bytesSent == -1)
	// {
	// 	std::cerr << "wrote = " << _bytesSent << std::endl;
		// std::cout << strerror(errno);
		// std::cout << errno;
	// 	throw codeException(502);
	// }
	if (_bytesSent < _bytesRead)
	{
		_totalBytesRead -= (_bytesRead - _bytesSent);
		_stream.seekg(_totalBytesRead);
	}
	delete[] buffer;
	if (_stream.eof())								//закрываем файл только после того как оправили все содержание файла
	{
		_stream.str(std::string()); // clear content in stream
		_stream.clear();
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
	// int i = 0;
	// while (arr[i]) {
	// 	usleep(100);
	// 	std::cerr << arr[i++] << "\n";
	// }
	// exit(1);
	return arr;
}

// void Response::addCgiVar( char ***envp,  Request & req, std::vector<std::string> & envpVector )
// {
// 	char **tmp;
// 	std::string tmpStr;
// 	size_t numOfLines = 0;
// 	size_t i = 0;
// 	size_t startIndx;
// 	std::vector<std::string>::iterator vBegin;
// 	std::vector<std::string>::iterator vEnd;
// 	std::map<std::string, std::string>::const_iterator mBegin;
// 	std::map<std::string, std::string>::const_iterator mEnd;

// 	vBegin = envpVector.begin();
// 	vEnd = envpVector.end();
// 	mBegin = req.getHeadears().begin();
// 	mEnd = req.getHeadears().end();

// 	std::string req_metod = ("REQUEST_METHOD=Post");			// REQUEST_METHOD=Post
// 	std::string serv_protocol = ("SERVER_PROTOCOL=HTTP/1.1");	//SERVER_PROTOCOL=HTTP/1.1
// 	std::string path_info = ("PATH_INFO=./");
// 	std::string content_length = ("HTTP_Transfer-Encoding=chunked");
// 	// std::string secret = ("HTTP_X_SECRET_HEADER_FOR_TEST=1");

// 	for (int i = 0; (*envp)[i] != NULL; ++i)
// 		numOfLines++;

// 	tmp = (char **)malloc(sizeof(char *) * (numOfLines + 4 + envpVector.size()) + req.getHeadears().size()); // 3 for new vars and additional 1 for NULL ptr

// 	while (i < numOfLines)							//переносим все изначальные envp в новый массив
// 	{
// 		tmp[i] = (*envp)[i];
// 		i++;
// 	}

// 	tmp[numOfLines] = strdup(req_metod.c_str());	//записываем необходимые для работы CGI переменные 
// 	tmp[numOfLines + 1] = strdup(serv_protocol.c_str());
// 	tmp[numOfLines + 2] = strdup(path_info.c_str());
// 	tmp[numOfLines + 3] = strdup(content_length.c_str());
// 	// tmp[numOfLines + 4] = strdup(secret.c_str());

// 	startIndx = numOfLines + 4;

// 	while (vBegin != vEnd)	//записываем переменные пришедшие из query string
// 	{
// 		tmp[startIndx] = strdup((*vBegin).c_str());
// 		startIndx++;
// 		vBegin++;
// 	}
// 	tmp[startIndx] = NULL;

// 	*envp = tmp;
// }

static void wait_subprocess(int) {
	int wstat;
	waitpid(P_ALL, &wstat, 0);
	if (wstat > 255)
		wstat /= 255;
	std::cout << "Child process exited with code " << wstat << std::endl;
	signal(SIGCHLD, SIG_DFL);
}

void 			Response::createSubprocess( Request & req, char **envp) {
	if (pipe(pipe1)) {
		std::cerr << RED << "Pipe1 error: has 500 exception" << RESET << "\n";
		throw(codeException(500));
	}
	if (pipe(pipe2)) {
		std::cerr << RED << "Pipe2 error: has 500 exception" << RESET << "\n";
		throw(codeException(500));
	}
	fcntl(pipe1[PIPE_OUT], F_SETFL, O_NONBLOCK);
	fcntl(pipe2[PIPE_IN], F_SETFL, O_NONBLOCK);
	if ((pid = fork()) < 0) {
		std::cerr << RED << "Fork error: has 500 exception" << RESET << "\n";
		throw(codeException(500));
	}
	if (pid == 0) { //child - process for CGI programm
		dup2(pipe1[PIPE_IN], 0);
		dup2(pipe2[PIPE_OUT], 1);
		close(pipe1[PIPE_OUT]);
		close(pipe1[PIPE_IN]);
		close(pipe2[PIPE_IN]);
		close(pipe2[PIPE_OUT]);
		if ((ex = execve(CGI_PATH, NULL, addCgiVar(req, envp))) < 0) {
			if (!envp)
				std::cerr << "ENVP DOES NOE EXIST\n";
			char buffer[100];
			std::cerr << RED << "Execve fault: has 500 exception" << RESET << "\n";
			strerror_r( errno, buffer, 256 ); // to remove
			std::cerr << "ERRNO: " << buffer << "\n"; // to remove
			throw(codeException(500));
			exit (20);
		}
		exit(ex);
	}
	else { // main process
		// close(pipe1[PIPE_OUT]); //Close unused pipe read end
		// close(pipe2[PIPE_IN]); //Close unused pipe write end
		signal(SIGCHLD, wait_subprocess);
		close(pipe1[PIPE_IN]);
		close(pipe2[PIPE_OUT]);
	}
}

int Response::extractCgiHeader( Request & req )
{
	std::string					headerAll;
	std::vector<std::string>	headerStrs;
	size_t pos, rd, bytesRead = 0;
	char	buf[BUF];
	while (true) {
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
	_bytesRead = 0;
	_bytesSent = 0;
	_totalBytesRead = 0;
}

int	&			Response::getPipeWrite( void ) { return pipe1[PIPE_OUT]; }
int	&			Response::getPipeRead( void ) { return pipe2[PIPE_IN]; }
int				Response::getContentLenght() { return(std::atoi(_contentLength.c_str())); }
std::string		Response::getHeader() { return(_header); }
std::string		Response::getContentType() { return(_contentType); }
std::string		Response::getReasonPhrase() { return(_reasonPhrase); }
std::string		Response::getFileLoc() { return(_fileLoc); }
std::string		Response::getCookie() { return(_cookie); }
std::ifstream &	Response::getFileStream() { return(_file); }
std::stringstream &	Response::getStrStream() { return(_stream); } 

void			Response::setFileLoc(std::string loc) { _fileLoc = loc; };
void			Response::setContentType(std::string type) { _contentType = type; };
