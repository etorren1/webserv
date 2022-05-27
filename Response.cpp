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
		throw(fileException());
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
	catch (fileException &e)
	{
		throw(fileException());
	}
}

std::string Response::make_general_header (Request req, std::string response_body)
{
	std::string Server = "webserv";
	// std::string Date =  Sun, 22 May 2022 18:42:40 GMT
	std::string contentType = "image/jpg";
	std::string contentLength = itos (response_body.length()); //= findContentLength();
	std::cout << YELLOW << contentLength << "\n";	
	// std::string Last-Modified: Sun, 22 May 2022 13:32:52 GMT
	std::string connection = "keep-alive";
	return(
			"Version: " + req.getProtocolVer()  + "\r\n" + 
			// "Server: " + Server + "\r\n" +
			"Content-Type: " + contentType + "\r\n" +
			"Content-Length: " + contentLength + "\r\n" +
			// "Connection: " + connection + "\r\n" +
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
}

void Response::make_response_body(Request req)
{

	//------------------------1------------------------
	// FOR PARTIAL RESPONSES

	std::ifstream input;
	// std::string dir_name = find_requested_file_path(Request req);
	std::string dir_name = "./site/image.jpg";

	input.open(dir_name.c_str(), std::ios::binary|std::ios::in);
	if(!input.is_open())
		// return false;
		throw(std::exception());
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

// void return_error_page(std::string errorNum, const size_t id, std::vector<struct pollfd> fds)
// {
// 	std::string responseBody = "<!DOCTYPE html>\\
// 	<html lang=\"en\">\\
// 	<head>\\
// 	    <meta charset=\"UTF-8\">\\
// 	    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\\
// 	    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\\
// 	    <title>Responsive 404 page </title>\\
// 	</head>\\
// 	<body>\\
// 	    <div class=\"container\">\\
// 	        <h2>404</h2>\\
// 	        <h3>Oops, nothing here...</h3>\\
// 	        <p>Please Check the URL</p>\\
// 	        <p>Otherwise, <a href=\"#homepage\">Click here</a> to redirect to homepage.</p>\\
// 	    </div>\\
// 	</body>\\
// 	</html";
// 	std::string contentLength = itos (responseBody.length()); //body length
// 	std::string header = "HTTP/1.1 404 Not Found\\
// 	Version: HTTP/1.1\\
// 	Content-Type: text/html\\
// 	Content-Length: \\";
// 	header.append(contentLength + "\n\n");
// 	std::string fullResponseMsg = header.append(responseBody);

// 	std::cout << GREEN << "HERE" << RESET;
// 	size_t result = send(fds[id].fd, fullResponseMsg.c_str(),	// Отправляем ответ клиенту с помощью функции send
// 		fullResponseMsg.length(), 0);
// }

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

	try
	{
		res.make_response_body(req);
		res.make_response_header(req);					// Формируем весь ответ вместе с заголовками
		response << res.getHeader() << res.getBody();	// Формируем весь ответ вместе с заголовками
					
		std::cout << RED << response.str() << RESET;

		result = send(fds[id].fd, response.str().c_str(),		// Отправляем ответ клиенту с помощью функции send
						response.str().length(), 0);
	}
	catch (fileException &e)
	{
		generateErrorPage(404, id);
		return;
	}
	catch (std::exception &e)
	{
		generateErrorPage(404, id);
		return;
	}
	res.clearResponseObj();
}

std::string	Response::getHeader() { return(_header);}
std::string	Response::getBody() { return(_body);}
std::string	Response::getContentType() { return(_contentType);}
std::string	Response::getStatusCode() { return(_statusCode);}
std::string	Response::getReasonPhrase() { return(_reasonPhrase);}