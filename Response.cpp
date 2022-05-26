
#include "Server.hpp"
#include <fstream>


static std::string parse_uri(std::string uri)
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

static std::string find_requested_file_path(Request req)
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

static std::string make_general_header (Request req, std::string response_body)
{
	std::string Server = "webserv";
	// std::string Date =  Sun, 22 May 2022 18:42:40 GMT
	std::string contentType = "image/jpg";
	std::cout << response_body.length() << RESET << "\n";
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

static std::string make_response_header(Request req, std::string response_body) // https://datatracker.ietf.org/doc/html/rfc2616#section-6
{
	std::cout << response_body.length() << RESET << "\n";
	std::string statusLine;
	std::string header;

	std::string statusCode = "200";
	std::string reasonPhrase = "OK";

	statusLine = req.getProtocolVer() + " " + statusCode + " " + reasonPhrase + "\r\n";
	header = make_general_header(req, response_body);

	return (statusLine + header);
}

static std::string make_response_body(Request req)
{

	//------------------------1------------------------
	// FOR PARTIAL RESPONSES

	std::ifstream input;
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
	std::string result;
	while (getline (input,line))	//reading map
				result.append(line + "\n");
	input.close();
	std::cout << YELLOW << "HERE"  << RESET << "\n";
	// std::cout << BLUE << result << RESET << "\n";
	std::cout << YELLOW << "HERE"  << RESET << "\n";
	std::cout << result.length() << RESET << "\n";
	return(result);
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

void Server::make_response(Request req, const size_t id)
{
	std::stringstream response;
	size_t result;

	try
	{
		std::string response_body = make_response_body(req);
		std::cout << response_body.length() << RESET << "\n";
		std::string response_header = make_response_header(req, response_body);			// Формируем весь ответ вместе с заголовками

		response	<< response_header			// Формируем весь ответ вместе с заголовками
					<< response_body;
		std::cout << RED << response.str() << RESET;


		result = send(fds[id].fd, response_header.c_str(),	// Отправляем ответ клиенту с помощью функции send
						response_header.length(), 0);
		result = send(fds[id].fd, response_body.c_str(),	// Отправляем ответ клиенту с помощью функции send
						response_body.length(), 0);
	}
	catch (fileException &e)
	{
		// std::cout << GREEN << "HERE" << RESET;
		generateErrorPage(404, id);
		return;
	}
	catch (std::exception &e)
	{
		generateErrorPage(404, id);
		return;
	}
}