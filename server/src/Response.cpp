#include "Response.hpp"

std::string Response::make_general_header (Request req)
{
	// std::string Server = "webserv";
	// _date = getTime();
	_contentLength = itos(getFileSize(_fileLoc.c_str()));

	// std::string Last-Modified: Sun, 22 May 2022 13:32:52 GMT
	std::string connection = "keep-alive"; //Connection: keep-alive
	return(
			"Version: " + req.getProtocolVer()  + "\r\n" + 
			// "Server: " + Server + "\r\n" +
			"Content-Type: " + _contentType + "\r\n" +
			"Content-Length: " + _contentLength + "\r\n" +
			"Connection: " + connection + "\r\n" +
			// Transfer-Encoding:
			+ "\r\n");
}

void Response::make_response_header(Request req) // https://datatracker.ietf.org/doc/html/rfc2616#section-6
{
	std::string statusLine;
	std::string generalHeader;

	_statusCode = "200";
	_reasonPhrase = "OK";

	statusLine = req.getProtocolVer() + " " + _statusCode + " " + _reasonPhrase + "\r\n";
	generalHeader = make_general_header(req);

	_header = statusLine + generalHeader;

	

	// std::cout << RED << _header << RESET;
}

int Response::make_response_body(Request req, const size_t socket)//2
{
	// int 			result;
	char 			*buffer = new char [RES_BUF_SIZE];

	if(!_file.is_open())
		throw(codeException(404));

	// size_t count = 0;

		_file.read (buffer, RES_BUF_SIZE);
		_bytesRead = _file.gcount();
		// if (_bytesRead < RES_BUF_SIZE) // ломает отправку файла
		// 	throw codeException(500);

		_totalBytesRead += _bytesRead;

		// if (_bytesRead == -1)
		// {
		// 	std::cerr << "read = " << _bytesRead << std::endl;
		// 	throw (123 );
		// }

		_bytesSent = send(socket, buffer, _bytesRead, 0);		// Отправляем ответ клиенту с помощью функции send

		if (_bytesSent == -1)
		{
			// throw (codeException(500));
			std::cerr << "wrote = " << _bytesSent << std::endl;
			std::cout << strerror(errno);
			// std::cout << errno;
			throw (123);
		}
		if (_bytesSent < _bytesRead)
		{
			_totalBytesRead -= (_bytesRead - _bytesSent);
			// if(_totalBytesRead && _totalBytesRead < file_size && _totalBytesRead > 0) //seekg sets the position of the next character to be extracted from the input stream.
				_file.seekg(_totalBytesRead);
		}
		// count += _bytesSent;
		// std::cout << "sent:" << _bytesSent << "\nread: " << _bytesRead << "\ntotal read:" << _totalBytesRead\
		// << "\ntotal send:" << count << "\n";
	if (_file.eof())								//закрываем файл только после того как оправили все содержание файла
	{
		_file.close();
		delete[] buffer;
		return (1);
	}

	delete[] buffer;
	return (0);
}

template <class T>
int Response::sendResponse(T input, const size_t socket)
{
	char 			*buffer = new char [RES_BUF_SIZE];

	if(_streamType == STREAM_IS_FILE && !input.is_open())
		throw(codeException(404));
	
	input.read (buffer, RES_BUF_SIZE);
		_bytesRead = input.gcount();
	
	_totalBytesRead += _bytesRead;

	_bytesSent = send(socket, buffer, _bytesRead, 0);		// Отправляем ответ клиенту с помощью функции send
	// if (_bytesSent == -1)
	// {
	// 	// throw (codeException(500));
	// 	std::cerr << "wrote = " << _bytesSent << std::endl;
	// 	std::cout << strerror(errno);
	// 	// std::cout << errno;
	// 	throw (123);
	// }
	if (_bytesSent < _bytesRead)
	{
		_totalBytesRead -= (_bytesRead - _bytesSent);
		input.seekg(_totalBytesRead);
	}

	if (input.eof())								//закрываем файл только после того как оправили все содержание файла
	{
		input.close();
		delete[] buffer;
		return (1);
	}

	delete[] buffer;
	return (0);
}

void Response::addCgiVar(char ***envp, Request req)
{
	char **tmp;
	size_t numOfLines = 0;
	size_t i = 0;

	std::string req_metod = ("REQUEST_METHOD=Post");			// REQUEST_METHOD=Post
	std::string serv_protocol = ("SERVER_PROTOCOL=HTTP/1.1");	//SERVER_PROTOCOL=HTTP/1.1
	std::string path_info = ("PATH_INFO=.");

	for (int i = 0; (*envp)[i] != NULL; ++i)
		numOfLines++;

	tmp = (char **)malloc(sizeof(char *) * numOfLines + 4); // 3 for new vars and additional 1 for NULL ptr

	while (i < numOfLines)
	{
		tmp[i] = (*envp)[i];
		i++;
	}
	tmp[i++] = (char *)req_metod.c_str();
	tmp[i++] = (char *)serv_protocol.c_str();
	tmp[i++] = (char *)path_info.c_str();
	tmp[i] = NULL;

	free (*envp);
	*envp = tmp;
}

void Response::clearResponseObj()
{
	_header.clear();
	_contentType.clear();
	_statusCode.clear();
	_reasonPhrase.clear();
	_connection.clear();
	_fileLoc.clear();
	_bytesRead = 0;
	_bytesSent = 0;
	_totalBytesRead = 0;
	_streamType = 0;
}

void Response::openFile()
{
	_file.open(_fileLoc.c_str(), std::ios::binary|std::ios::in); // open file
}

std::string		Response::getHeader() { return(_header); }
std::string		Response::getContentType() { return(_contentType); }
std::string		Response::getStatusCode() { return(_statusCode); }
std::string		Response::getReasonPhrase() { return(_reasonPhrase); }
std::string		Response::getFileLoc() { return(_fileLoc); }
std::ifstream &	Response::getInput() { return(_file); }

void			Response::setFileLoc(std::string loc) { _fileLoc = loc; };
void			Response::setContentType(std::string type) { _contentType = type; };
// void			Response::setInput(std::ifstream &input) { _file = input; };