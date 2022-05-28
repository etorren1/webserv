#include "Server.hpp"
#include <fstream>

std::string Response::parse_uri(std::string uri)
{
	std::fstream fs;
	fs.open (uri, std::fstream::in);
	if (fs)
	{
		fs.close();
		return(uri);
	}
	else
	{
		throw(codeException(404));
	}
}

std::string Response::find_requested_file_path(Request req)
{
	std::string full_path;
	std::string uri = req.getReqURI();

	// full_path = parse_uri(uri);
	try
	{
		// full_path = parse_uri(req.getReqURI());
		full_path = parse_uri("./site/index.html");
		return(full_path);
	}
	catch (codeException &e)
	{
		throw(codeException(404));
	}
}

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

	std::ifstream	input;
	size_t			result;

	input.open(_fileLoc.c_str(), std::ios::binary|std::ios::in);
	if(!input.is_open())
		throw(codeException(404));
	// get length of file:			https://m.cplusplus.com/reference/istream/istream/seekg/?kw=seekg
	input.seekg(0, std::ios::end); //seekg sets the position of the next character to be extracted from the input stream.
	size_t file_size = input.tellg(); //узнаем размер файла, tellg returns the position of the current character in the input stream.
	input.seekg(0, std::ios::beg);

	size_t range_begin = 0;
	if(range_begin && range_begin < file_size && range_begin > 0)
		input.seekg(range_begin); 

	std::string line;
	while (getline (input,line))
				_body.append(line + "\n");

	result = send(fds[id].fd, _body.c_str(),		// Отправляем ответ клиенту с помощью функции send
					_body.length(), 0);
	std::cout << YELLOW << result << RESET << "\n";

	input.close();

	//-------------------------------------------------

	// //------------------------2------------------------
	// std::fstream fs;
	// std::string line;
	// std::string result;
	// fs.open ("./site/image.jpg", std::fstream::in);
	// if (fs.is_open())
	// {
	// 	while (getline (fs,line))	//reading map
	// 			result.append(line + "\n");
	// 	fs.close();
	// }
	// std::cout << BLUE << result << "\n";
	// std::cout << result.length() << RESET <<"\n";
	// return(result);
	// //-------------------------------------------------
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
	if (req.getReqURI() == "/") {
		res.setFileLoc("./site/index.html");
		res.setContentType("text/html");
	}
	else {
		if (req.getHost() == "127.0.0.1:8080") {
			res.setFileLoc("./site/image.png");
			res.setContentType("image/png");
		}
	}
	// res.setFileLoc(res.find_requested_file_path(req));
	// res.setContentType(req.getContentType());
	try
	{
		res.make_response_header(req);
		result = send(fds[id].fd, res.getHeader().c_str(),		// Отправляем ответ клиенту с помощью функции send
						res.getHeader().length(), 0);

		res.make_response_body(req, id, fds);

		// response << res.getHeader() << res.getBody();	// Формируем весь ответ вместе с заголовками
		// std::cout << RED << response.str() << RESET;
	}
	catch (codeException &e)
	{
		generateErrorPage(e.getErrorCode(), id);
		return;
	}
	catch (std::exception &e)
	{
		e.what();
		return;
	}
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