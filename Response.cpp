
#include "Server.hpp"


find_uri()
{
	std::string full_path;

}

get_request_data()
{
	find_uri();

}

std::string make_general_header (Request req)
{

}

std::string make_response_header(Request req) // https://datatracker.ietf.org/doc/html/rfc2616#section-6
{
	std::string statusLine;
	std::string header;

	std::string statusCode = "200";
	std::string reasonPhrase = "OK";

	statusLine = req._protocolVersion + "  " + statusCode + "  " + reasonPhrase + "\r\n";

	return (statusLine + "\n" + header);
}

std::string make_response_body()
{

}

void response(Request req)
{
	std::stringstream response;
	size_t result;
	// Формируем весь ответ вместе с заголовками
	response	<< make_response_header(req)
				<< make_response_body(req);
	// Отправляем ответ клиенту с помощью функции send
	result = send(userFds[id].fd, response.str().c_str(),
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
