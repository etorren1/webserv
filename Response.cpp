
#include "Server.hpp"
#include <fstream>


static std::string parse_uri(std::string uri)
{
	std::fstream fs;
	fs.exceptions ( std::fstream::failbit );		//https://stackoverflow.com/questions/9670396/exception-handling-and-opening-a-file
	try
	{
		fs.open (uri, std::fstream::in);		// https://www.cplusplus.com/reference/fstream/fstream/open/
	}
	catch (std::exception const& e)
	{
		std::cerr << "exception caught: " << e.what() << '\n';
		std::cout << GREEN << "here" << RESET <<'\n';
	}
	fs.close();
	return(uri);
}

static std::string find_requested_file_path(Request req)
{
	std::string full_path;
	std::string uri = req.getReqURI();

	// return(parse_uri(uri));

	return(parse_uri("./site/index.ht"));
}

static std::string make_general_header (Request req, std::string response_body)
{
	std::string Server = "webserv";
	// std::string Date =  Sun, 22 May 2022 18:42:40 GMT
	std::string contentType = "text/html";
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
	std::ifstream file(find_requested_file_path(req));
	std::string tmp_line;
	std::string return_line;
	
	if (file.is_open())
	{
		while (getline (file,tmp_line))					// reading requested file
				return_line.append(tmp_line + "\n");
		file.close();
	}
	return(return_line);
}

void Server::make_response(Request req, const size_t id)
{
	std::stringstream response;
	size_t result;

	std::string response_body = make_response_body(req);
	std::string response_header = make_response_header(req, response_body);			// Формируем весь ответ вместе с заголовками
	
	response	<< response_header			// Формируем весь ответ вместе с заголовками
				<< response_body;
	
	std::cout << RED << response.str() << RESET;
	result = send(fds[id].fd, response.str().c_str(),	// Отправляем ответ клиенту с помощью функции send
					response.str().length(), 0);
}