#include "Server.hpp"
#include "Response.hpp"


std::string Response::make_general_header (Request req, std::string response_body)
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
	generalHeader = make_general_header(req, _body);

	_header = statusLine + generalHeader;

	_input.open(_fileLoc.c_str(), std::ios::binary|std::ios::in); // open file

	std::cout << RED << _header << RESET;
	
}

int count = 0;

void Response::make_response_body(Request req, const size_t socket, std::vector<pollfd> fds)
{
	char 			*buffer = new char [2048];
	size_t			file_size;
	
	std::fill(&buffer[0], &buffer[2048], 0);
	file_size = getFileSize(_fileLoc.c_str());

	// if (_hasSent != 1) {
	// 	_input.open(_fileLoc.c_str(), std::ios::binary|std::ios::in);
	// 	size_t file_size = getFileSize(_fileLoc.c_str());	
	// }

	if(!_input.is_open())
		throw(codeException(404));

	// size_t count = 0;
	// while (!_input.eof())
	// {
	_input.read (buffer, 2048);
	_bytesRead = _input.gcount(); //The number of successfully extracted characters can be queried using gcount() - https://en.cppreference.com/w/cpp/io/basic_istream/read
	if (_bytesRead < 2048)
		throw codeException(500);
	_totalBytesRead += _bytesRead;
	if (_bytesRead == -1)
	{
		std::cerr << "read = " << _bytesRead << std::endl;
		throw (123 );
	}

	_bytesSent = send(socket, buffer, _bytesRead, 0);	// Отправляем ответ клиенту с помощью функции send
	if (_bytesSent == -1)
	{
		std::cerr << "wrote = " << _bytesSent << std::endl;
		throw (123);
	}
	if (_bytesSent < _bytesRead)
	{
		std::cout << "HERE";
		_bytesRead -= _bytesRead - _bytesSent;
		if(_bytesRead && _bytesRead < file_size && _bytesRead > 0) //seekg sets the position of the next character to be extracted from the input stream.
			_input.seekg(_bytesRead);
	}

	count += _bytesSent;
	std::cout << YELLOW << "sent:" << _bytesSent << "\nwritten: " << _bytesRead << RESET << "\n";
	// }
	std::cout << GREEN << count << RESET << "\n";
	if (_input.eof())								//закрываем файл только после того как оправили все содержание файла
	{
			// std::cout << RED << "blabla" << RESET << "\n";	
		_input.close();
		_sendingFinished = 1;
	}
	// std::cout << BLUE<< "HERE" << RESET << "\n";

	delete[] buffer;
}

void Response::clearResponseObj()
{
	_header.clear();
	_body.clear();
	_contentType.clear();
	_statusCode.clear();
	_reasonPhrase.clear();
	_connection.clear();
	_fileLoc.clear();
	_sendingFinished = 0;
	_range_begin = 0;
	_bytesRead = 0;
	_bytesSent = 0;
	_totalBytesRead = 0;
}

void Server::make_response(Request req, const size_t socket)
{
	std::stringstream response;
	size_t result;
	Response &res = client[socket]->getResponse();
	// if (req.getReqURI() == "/favicon.ico")
	// {
	// 	res.setFileLoc("./site/image.png");
	// 	res.setContentType("image/png");
	// }
	// else
	// {
	// 	res.setFileLoc(location);
	// 	res.setContentType(req.getContentType());
	// }
	res.setFileLoc("./site/video.mp4");
	res.setContentType("video/mp4");
	// res.setFileLoc("./site/index.html");
	// res.setContentType("text/html");
	// res.setFileLoc("./site/image.jpg");
	// res.setContentType("image/jpg");
	try
	{
		if (res._hasSent == 0)
		{
			res.make_response_header(req);
			result = send(socket, res.getHeader().c_str(),	// Отправляем ответ клиенту с помощью функции send
							res.getHeader().length(), 0);	
			res._hasSent = 1;
		}
		// std::cout << "location: " << location << "\n";
		if (res._hasSent == 1)
			res.make_response_body(req, socket, fds);
	}
	catch (codeException &e)
	{
		generateErrorPage(e.getErrorCode(), socket);
		return;
	}
	// catch (std::exception &e)
	// {
	// 	e.what();
	// 	return;
	// }
	// res.clearResponseObj();
}

std::string	Response::getHeader() { return(_header); }
std::string	Response::getBody() { return(_body); }
std::string	Response::getContentType() { return(_contentType); }
std::string	Response::getStatusCode() { return(_statusCode); }
std::string	Response::getReasonPhrase() { return(_reasonPhrase); }
std::string	Response::getFileLoc() { return(_fileLoc); }
void		Response::setFileLoc(std::string loc) { _fileLoc = loc; };
void		Response::setContentType(std::string type) { _contentType = type; };