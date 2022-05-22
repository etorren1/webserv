
#include "Server.hpp"
#include <fstream>


static std::string parse_uri(std::string uri)
{
	try
	{
		std::fstream fs;
		fs.open ("test.txt", std::fstream::in);		// https://www.cplusplus.com/reference/fstream/fstream/open/
	}
	catch (std::exception& e)
	{
		std::cerr << "exception caught: " << e.what() << '\n';
	}
	if (fs.is_open())
		fs.close();
}
static std::string find_requested_file_path(Request req)
{
	std::string full_path;
	std::string uri = req.getReqURI();

	parse_uri(uri);


}

static std::string make_general_header (Request req)
{
	std::string Server = "webserv";
	// std::string Date =  Sun, 22 May 2022 18:42:40 GMT
	std::string contentType = "text/plain";
	std::string contentLength = "1000"; //= findContentLength();
	// std::string Last-Modified: Sun, 22 May 2022 13:32:52 GMT
	std::string connection = "keep-alive";
}

static std::string make_response_header(Request req) // https://datatracker.ietf.org/doc/html/rfc2616#section-6
{
	std::string statusLine;
	std::string header;

	std::string statusCode = "200";
	std::string reasonPhrase = "OK";

	statusLine = req.getProtocolVer() + " " + statusCode + " " + reasonPhrase + "\r\n";
	header = make_general_header(req);

	return (statusLine + "\n" + header);
}

static std::string make_response_body(Request req)
{
	std::ifstream file(find_requested_file_path(req));
	std::string tmp_line;
	std::string return_line;
	
	if (file.is_open())
	{
		while (getline (file,tmp_line))					// reading requested file
				return_line.append(tmp_line);
		file.close();
	}

	return(return_line);
}

void Server::response(Request req, const size_t id)
{
	std::stringstream response;
	size_t result;
	
	response	<< make_response_header(req)			// Формируем весь ответ вместе с заголовками
				<< make_response_body(req);
	
	result = send(fds[id].fd, response.str().c_str(),	// Отправляем ответ клиенту с помощью функции send
					response.str().length(), 0);
}


// void response()
// {
// 	std::stringstream response_body;
// 	std::stringstream response;
// 	size_t result;
// 	response_body << "<title>Test C++ HTTP Server</title>\n"
// 		<< "<h1>Test page</h1>\n"
// 		<< "<p>This is body of the test page...</p>\n"
// 		<< "<h2>Request headers</h2>\n"
// 		<< "<form action=\"/my-handling-form-page\" method=\"post\">\n" // my-handling-form-page - cgi scriipt
// 		<<	"<div>\n"
// 		<< "<label for=\"name\">Name:</label>\n"
// 		<< "<input type=\"text\" id=\"name\" name=\"user_name\">\n"
// 		<< "</div>\n"
// 		<< "<div>\n"
// 		<< "<label for=\"mail\">E-mail:</label>\n"
// 		<< "<input type=\"email\" id=\"mail\" name=\"user_email\">\n"
// 		<< "</div>\n"
// 		<< "<div>\n"
// 		<< "<label for=\"msg\">Message:</label>\n"
// 		<< "<textarea id=\"msg\" name=\"user_message\"></textarea>\n"
// 		<< "</div>\n"
// 		<< "<div class=\"button\">\n"
// 		<< "<button type=\"submit\">Send your message</button>\n"
// 		<< "</div>\n"
// 		<< "</form>\n"
// 		<< "<pre>" << buf << "</pre>\n"
// 		<< "<em><small>Test C++ Http Server</small></em>\n";
// 	// Формируем весь ответ вместе с заголовками
// 	response << "HTTP/1.1 200 OK\r\n"
// 		<< "Version: HTTP/1.1\r\n"
// 		<< "Content-Type: text/html; charset=utf-8\r\n"
// 		<< "Content-Length: " << response_body.str().length()
// 		<< "\r\n\r\n"
// 		<< response_body.str();
// 	// Отправляем ответ клиенту с помощью функции send
// 	result = send(userFds[id].fd, response.str().c_str(),
// 		response.str().length(), 0);
// }
