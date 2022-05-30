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
	std::cout << RED << _header << RESET;
}

void Response::make_response_body(Request req, const size_t id, std::vector<pollfd> fds)
{

	//------------------------1------------------------
	// FOR PARTIAL RESPONSES

	int 			result;
	char 			*buffer = new char [2048];
	
	// std::ifstream							_input;
	_input.open(_fileLoc.c_str(), std::ios::binary|std::ios::in);
	if(!_input.is_open())
		throw(codeException(404));

	size_t file_size = getFileSize(_fileLoc.c_str());	

	size_t count = 0;
	while (!_input.eof())
	{
		_input.read (buffer, 2048);
		int read_bytes =  _input.gcount();
		if (read_bytes == -1)
		{
			std::cerr << "read = " << read_bytes << std::endl;
			throw (123 );
		}
		if (fds[id].revents & POLLOUT)
		usleep(1000);
			result = send(fds[id].fd, buffer, 2048, 0);		// Отправляем ответ клиенту с помощью функции send
		if (result == -1)
		{
			std::cerr << "wrote = " << result << std::endl;
			throw (123);
		}
		// std::cout << YELLOW << "wrote:" << result << "\nwritten: " << read_bytes << RESET << "\n";
		count += result;
	}
	// std::cout << GREEN << count << RESET << "\n";
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
}

void Server::make_response(Request req, const size_t id)
{
	
	std::stringstream response;
	size_t result;
	Response &res = client[fds[id].fd]->getResponse();
	// if (req.getReqURI() == "/favicon.ico")
	// {
	// 	res.setFileLoc("./site/image.png");
	// 	res.setContentType("image/png");
	// }
	// else
	// {
	// 	// res.setFileLoc("./site/colors/tables/yellow.html");
	// 	res.setFileLoc(location);
	// 	res.setContentType(req.getContentType());
	// }
	// res.setFileLoc("./site/video.mp4");
	// res.setContentType("video/mp4");
	// res.setFileLoc("./site/index.html");
	// res.setContentType("text/html");
	res.setFileLoc("./site/image.jpg");
	res.setContentType("image/jpg");
	try
	{
		res.make_response_header(req);
		result = send(fds[id].fd, res.getHeader().c_str(),	// Отправляем ответ клиенту с помощью функции send
						res.getHeader().length(), 0);
		res.make_response_body(req, id, fds);
	}
	catch (codeException &e)
	{
		generateErrorPage(e.getErrorCode(), id);
		return;
	}
	// catch (std::exception &e)
	// {
	// 	e.what();
	// 	return;
	// }
	res.clearResponseObj();
}

std::string	Response::getHeader() { return(_header); }
std::string	Response::getBody() { return(_body); }
std::string	Response::getContentType() { return(_contentType); }
std::string	Response::getStatusCode() { return(_statusCode); }
std::string	Response::getReasonPhrase() { return(_reasonPhrase); }
std::string	Response::getFileLoc() { return(_fileLoc); }
void		Response::setFileLoc(std::string loc) { _fileLoc = loc; };
void		Response::setContentType(std::string type) { _contentType = type; };