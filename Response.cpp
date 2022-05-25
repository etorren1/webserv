
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
		full_path = parse_uri("./site/image.png");
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
	std::string contentType = "image/png";
	std::string contentLength = itos (std::strlen(response_body.c_str())); //= findContentLength();
	// std::string Last-Modified: Sun, 22 May 2022 13:32:52 GMT
	std::string connection = "keep-alive";
	return(
			"Version: " + req.getProtocolVer()  + "\r\n" + 
			// "Server: " + Server + "\r\n" +
			"Content-Type: " + contentType + "\r\n" +
			"Content-Length: " + contentLength + "\r\n" +
			// "Connection: " + connection + "\r\n" +
			+ "\r\n");
}

static std::string make_response_header(Request req, std::string response_body) // https://datatracker.ietf.org/doc/html/rfc2616#section-6
{
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
	// std::ifstream file(find_requested_file_path(req));
	// std::string tmp_line;
	// std::string return_line;
	
	// if (file.is_open())
	// {
	// 	while (getline (file,tmp_line))					// reading requested file
	// 			return_line.append(tmp_line + "\n");
	// 	file.close();
	// }
	std::fstream fs;
	std::string line;
	std::string result;
	fs.open ("./site/image.png", std::fstream::in);
	if (fs.is_open())
	{
		while (getline (fs,line))	//reading map
				result.append(line + "\n");
		fs.close();
	}
	std::cout << BLUE << result << "\n";
	std::cout << result.length() << RESET <<"\n";

	// return(return_line);
	return(result);
}

void return_error_page(std::string errorNum, const size_t id, std::vector<struct pollfd> fds)
{
	std::string responseBody = "<!DOCTYPE html>\\
	<html lang=\"en\">\\
	<head>\\
	    <meta charset=\"UTF-8\">\\
	    <meta http-equiv=\"X-UA-Compatible\" content=\"IE=edge\">\\
	    <meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">\\
	    <title>Responsive 404 page </title>\\
	</head>\\
	<body>\\
	    <div class=\"container\">\\
	        <h2>404</h2>\\
	        <h3>Oops, nothing here...</h3>\\
	        <p>Please Check the URL</p>\\
	        <p>Otherwise, <a href=\"#homepage\">Click here</a> to redirect to homepage.</p>\\
	    </div>\\
	</body>\\
	</html";
	std::string contentLength = itos (std::strlen(responseBody.c_str())); //body length
	std::string header = "HTTP/1.1 404 Not Found\\
	Version: HTTP/1.1\\
	Content-Type: text/html\\
	Content-Length: \\";
	header.append(contentLength + "\n\n");
	std::string fullResponseMsg = header.append(responseBody);

	std::cout << GREEN << "HERE" << RESET;
	size_t result = send(fds[id].fd, fullResponseMsg.c_str(),	// Отправляем ответ клиенту с помощью функции send
		fullResponseMsg.length(), 0);
}

void Server::make_response(Request req, const size_t id)
{
	std::stringstream response;
	size_t result;

	try
	{
		std::string response_body = make_response_body(req);
		std::string response_header = make_response_header(req, response_body);			// Формируем весь ответ вместе с заголовками

		response	<< response_header			// Формируем весь ответ вместе с заголовками
					<< response_body;

		// std::string response_header =
		// 	int fd = open("./site/image.png", O_RDWR);
		// 	char buf[2048];
		// 	int rd;
		// 	int bytes = 0;
		// 	std::string text;
		// 	while ((rd = read(fd, buf, 2048)) > 0)
		// 	{
		// 		buf[rd] = 0;
		// 		text += buf;
		// 		bytes += rd;
		// 	}

		std::cout << RED << response.str() << RESET;
		result = send(fds[id].fd, response_header.c_str(),	// Отправляем ответ клиенту с помощью функции send
						response_header.length(), 0);
		result = send(fds[id].fd, response_body.c_str(),	// Отправляем ответ клиенту с помощью функции send
						response_body.length(), 0);
	}
	catch (fileException &e)
	{
		// std::cout << GREEN << "HERE" << RESET;
		return_error_page("404", id, this->fds);
		return;
	}
	catch (std::exception &e)
	{
		return;
	}
}