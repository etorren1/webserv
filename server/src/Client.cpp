#include "Client.hpp"
#include "Utils.hpp"
#include <errno.h>



void Client::checkMessageEnd(void)
{
	if (status & IS_BODY)
	{
		// std::cout << BLUE << "Transfer-Encoding: " << req.getTransferEnc() << RESET << "\n";
		// std::cout << BLUE << "Content-Length: " << req.getСontentLenght() << RESET << "\n";

		if (req.getTransferEnc() == "chunked")
		{
			if (message.find("0\r\n\r\n"))
				fullpart = true;
			else
				fullpart = false;
		}
		else if (req.getContentLenght().size())
		{
			size_t len = atoi(req.getContentLenght().c_str());
			// std::cout << "message.size = " << message.size() << " " << "len = " << len << "\n";
			if (message.size() == len)
				fullpart = true;
			else
				fullpart = false;
		}
		else
			std::cout << RED << "I cant work with this body Encoding" << RESET << "\n";
		// fullpart = true;
		// ЕСЛИ Transfer-Encoding ждем 0
		// ECЛИ Content-length ждем контент лен
	}
	else
	{
		size_t pos = message.rfind("\r\n\r\n");
		if (pos != std::string::npos)
		{
			while (message.find("\r") != std::string::npos)
			{
				message.erase(message.find("\r"), 1); // из комбинации CRLF
				pos--;								  // Удаляем символ возврата карретки
			}
			fullpart = true;
			tail = message.substr(pos + 4);
		}
		else
			fullpart = false;
	}
}

void Client::handleRequest(char **envp)
{
	if (status & IS_BODY)
	{
		std::cout << "PARSE BODY\n";
		req.parseBody(message);
		status |= REQ_DONE;
		// std::cout << GREEN << "REQ_DONE with body" << RESET << "\n";
	}
	else
	{
		bool rd = req.parseText(message);
		if (rd == true)
		{
			// std::cout << PURPLE << "IS_BODY" << RESET << "\n";
			message = tail;
			status |= IS_BODY;
			checkMessageEnd();
			if (fullpart)
				status |= REQ_DONE;
		}
		else
		{
			// std::cout << GREEN << "REQ_DONE without body" << RESET << "\n";
			status |= REQ_DONE;
		}
	}
}

int Client::searchErrorPages()
{
	size_t pos;
	std::string tmp;
	while ((pos = location.find_last_of("/")) != std::string::npos)
	{
		location = location.substr(0, pos);
		tmp = location + loc->get_error_page().second;
		res.setFileLoc(tmp);
		if (res.openFile())
		{
			// std::cout << CYAN << tmp << RESET << "\n";
			return 1;
		}
		// std::cout << PURPLE << tmp << RESET << "\n";
	}
	return 0;
}

void Client::handleError(const int code)
{
	int file = 0;
	if (loc && loc->get_error_page().first == code)
	{
		if ((file = searchErrorPages()) == 1)
		{
			req.setMIMEType(loc->get_error_page().second);
			res.setContentType(req.getContentType());
			res.make_response_header(req, code, resCode[code]);
		}
	}
	if (!file)
	{
		std::string mess = "none";
		try
		{
			mess = resCode.at(code);
		}
		catch (const std::exception &e)
		{
		}
		res.make_response_html(code, mess); //TODO: 
	}
	cleaner();
	statusCode = code;
	status |= ERROR;
	status |= REQ_DONE;
	if (file)
		status |= IS_FILE;
}

void Client::initResponse(char **envp)
{
	if (status & AUTOIDX)
		res.make_response_autoidx(req, location, statusCode, resCode[statusCode]);
	else if (status & REDIRECT)
	{
		res.make_response_html(statusCode, resCode[statusCode], req.getHost() + req.getReqURI()); //TODO: 
		// res.make_response_header(req, statusCode, resCode[statusCode], 1);
	}
	else
	{
		res.setFileLoc(location);
		res.setContentType(req.getContentType());
		res.openFile();
		res.make_response_header(req, statusCode, resCode[statusCode]);
	}
	if (req.getMethod() == "POST")
	{

		iter = 0;
		cgiWriteFlag = false;
		totalSent = 0;
		res.addCgiVar(&envp, req);

		if (pipe(pipe2))
			throw(codeException(500));
		if (pipe(pipe1))
			throw(codeException(500));

		// if (cgi used)
		// {
			res.getStrStream().str(""); //очищаем от записанного ранее хедера, который далеепридется переписать из-за cgi
			res.getStrStream().clear();
			if ((pid = fork()) < 0)
				throw(codeException(500));
			if (pid == 0) //child - process for CGI programm
			{
				close(pipe1[PIPE_IN]); //Close unused pipe write end
				close(pipe2[PIPE_OUT]); //Close unused pipe read end
				dup2(pipe1[PIPE_OUT], 0);
				dup2(pipe2[PIPE_IN], 1);

				if ((ex = execve(CGI_PATH, NULL, envp)) < 0)
					throw(codeException(500));
				close(pipe1[PIPE_OUT]); //Closing remaining fds before closing child process
				close(pipe2[PIPE_IN]); //Closing remaining fds before closing child process
				exit(ex);
			}
			else
			{
				close(pipe1[PIPE_OUT]); //Close unused pipe read end
				close(pipe2[PIPE_IN]); //Close unused pipe write end
			}
		// }
	}
	status |= REQ_DONE;
}

void Client::makeResponse(char **envp)
{ // envp не нужен уже
	if (status & ERROR)
		makeErrorResponse();
	else if (status & AUTOIDX)
		makeAutoidxResponse();
	else if (req.getMethod() == "GET")
		makeGetResponse();
	else if (req.getMethod() == "POST")
		makePostResponse(envp);
	else if (req.getMethod() == "DELETE")
		makeDeleteResponse();
	// 	makePostResponse(envp);  //envp не нужен уже
}

void Client::makeAutoidxResponse()
{
	if (res.sendResponse_stream(socket))
		status |= RESP_DONE;
	if (status & RESP_DONE)
	{
		// std::cout << GREEN << "End AUTOINDEX response on " << socket << " socket" << RESET << "\n";
		cleaner();
	}
}

void Client::makeErrorResponse()
{
	if (status & HEAD_SENT)
	{
		if (status & IS_FILE)
		{
			if (res.sendResponse_file(socket))
				status |= RESP_DONE;
		}
		else
			status |= RESP_DONE;
	}
	else
	{
		if (res.sendResponse_stream(socket))
			status |= HEAD_SENT;
	}
	if (status & RESP_DONE)
	{
		// std::cout << GREEN << "End ERROR response on " << socket << " socket" << RESET << "\n";
		cleaner();
	}
}

void Client::makeGetResponse()
{
	if (status & HEAD_SENT)
	{
		if (res.sendResponse_file(socket))
		{
			status |= RESP_DONE;
			// std::cout << "GET sendfile() must be second\n";
		}
	}
	else
	{
		if (res.sendResponse_stream(socket))
		{
			status |= HEAD_SENT;
			// std::cout << "GET sendstream() must be first\n";
		}
	}
	if (status & RESP_DONE)
	{
		cleaner();
		// std::cout << "GET cleaner() must be ending\n";
		// std::cout << GREEN << "End GET response on " << socket << " socket" << RESET << "\n";
	}
}

void Client::extractCgiHeader( char * buff )
{
	res.getStrStream().str("");
	res.getStrStream().clear();

	std::string					tmp, tmp2;
	std::vector<std::string>	headerAndBody;
	std::vector<std::string>	headerStrs;
	// int							code;

	tmp = buff;
	headerAndBody = split(tmp, "\r\n\r\n", "");
	headerStrs = split(headerAndBody.at(0), "\r\n", "");

	req.parseMapHeaders(headerStrs, headerStrs.size());
	tmp.clear();
	tmp = req.getCgiStatusCode();
	tmp2 = tmp.substr(0, 3);
	req.setCgiStatusCode(tmp2);
	res.setStatusCode(tmp2);

	// code = std::atoi(req.getCgiStatusCode().c_str());
	// res.make_response_header(req, code, resCode[code]);

	//отправка body оставшаяся в буффере после первого прочтения из пайпа
	res.getStrStream().write(headerAndBody.at(1).c_str(), headerAndBody.at(1).length()); //записываем кусок body который попал в буффер вместе с хедером от cgi
	// res.sendResponse_stream(socket); //не тут должно быть 
}

void Client::makePostResponse(char **envp)
{
	std::cout << BLUE << "ENTERED makePostResponse METOD" << "\n" << RESET;
	iter++;
	std::cout << "iter = " << iter << "\n";
	std::cout << "cgiWriteFlag = " << cgiWriteFlag << "\n";
	char				buff[2048];
	int					wrtRet;
	int					readRet;
	int					code;

	if (cgiWriteFlag == false)		// флаг cgi записан == false 
	{
		std::cout << "BODY: " << message << "\n";

		wrtRet = write(pipe1[PIPE_IN], message.c_str(), message.length());
		totalSent += wrtRet;
		if (totalSent == message.length()) //SIGPIPE
		{
			close(pipe1[PIPE_IN]);
			cgiWriteFlag = true;
		}
	}

	if (cgiWriteFlag == true)		//если все данные передались в cgi
	{
		std::cout << BLUE << "READING FROM PIPE1 started" << "\n" << RESET;
		//читаем из cgi порцию даты, прочитанный кусок из cgi пишем клиенту в сокет
		readRet = read(pipe2[PIPE_OUT], buff, 2048);  // ret -1

		if (!(status & HEAD_SENT))
		{
			extractCgiHeader(buff);
			status |= HEAD_SENT;
		}
		else
		{
			// std::cout << "readRet " << readRet << "\n";
			if (readRet == -1)
				throw(codeException(500));
			if(status & RESP_DONE)			//весь body уже записан в поток, отпавляем его частями клиенту
				std::cout << "sendResponse ret" << res.sendResponse_stream(socket) << "\n";
			if (readRet == 0)				//0 запишется в readRet один раз и больше не будет меняться до конца response
			{
				// std::cout << BLUE << "READ STOPED" << "\n" << RESET;
				status |= RESP_DONE;		//все прочитали из cgi
				code = std::atoi(req.getCgiStatusCode().c_str());
				res.make_response_header(req, code, resCode[code], getStrStreamSize(res.getStrStream()));

			}
			else //записываем из буффера часть данных в поток
			{
				// res.make_response_header(req, 200, "OK", 500); //заменить!!!
				// res.sendResponse_stream(socket);
				buff[readRet] = '\0';
				res.getStrStream().write(buff, readRet);
			}
		}
	}

	// if (???)
	// 	status |= RESP_DONE;
	if (status & RESP_DONE)
	{
		// close(pipe1[PIPE_IN]);
		waitpid(pid, &status, 0); // ???
		cleaner();
		close(pipe2[PIPE_OUT]);
		std::cout << "COMPLEATING POST RESPONSE2\n"; 
	}
}

void Client::makeDeleteResponse()
{
}

void Client::cleaner()
{
	if (status & ERROR)
		std::cout << GREEN << "Complete working with error: \e[1m" << statusCode << " " << resCode[statusCode] << "\e[0m\e[32m on \e[1m" << socket << "\e[0m\e[32m socket" << RESET << "\n";
	else
		std::cout << GREEN << "Complete working with request: \e[1m" << req.getMethod() << "\e[0m\e[32m on \e[1m" << socket << "\e[0m\e[32m socket" << RESET << "\n";
	message.clear();
	tail.clear();
	location.clear();
	req.cleaner();
	res.cleaner();
	statusCode = 0;
	status = 0;
	loc = NULL;
	srv = NULL;
}

void Client::setMessage(const std::string &mess) { message = mess; }
void Client::setServer(Server_block *s)
{
	srv = s;
	this->loc = getLocationBlock(req.getDirs());
	if (loc == NULL)
	{
		std::cout << RED << "Location not found: has 404 exception " << RESET << "\n";
		throw codeException(404);
	}
}

bool			Client::readComplete() const { return fullpart; }
Response &		Client::getResponse() { return res; }
Request &		Client::getRequest() { return req; }
size_t			Client::getMaxBodySize() const { return loc->get_client_max_body_size(); }
std::string		Client::getHost() const { return req.getHost(); }
std::string		Client::getMessage() const { return message; }
Server_block *	Client::getServer(void) { return srv; }
int *			Client::getPipe1() { return pipe1; };
int *			Client::getPipe2() { return pipe2; };

Location_block *Client::getLocationBlock(std::vector<std::string> vec) const
{
	Location_block *lctn;
	size_t pos, i = 0;
	while (i < vec.size())
	{
		try
		{
			lctn = srv->lctn.at(vec[i]);
			return (lctn);
		}
		catch (const std::exception &e)
		{
			try
			{
				vec[i] += "/";
				lctn = srv->lctn.at(vec[i]);
				return (lctn);
			}
			catch (const std::exception &e)
			{
				i++;
			}
		}
	}
	return NULL;
}

Client::Client(size_t nwsock)
{
	fullpart = false;
	location.clear();
	message.clear();
	socket = nwsock;
	status = 0;
	statusCode = 0;
	srv = NULL;
	loc = NULL;
	//Для POST браузер сначала отправляет заголовок, сервер отвечает 100 continue, браузер
	// отправляет данные, а сервер отвечает 200 ok (возвращаемые данные).
	this->resCode.insert(std::make_pair(100, "Continue"));
	this->resCode.insert(std::make_pair(101, "Switching Protocols"));
	this->resCode.insert(std::make_pair(200, "OK"));
	this->resCode.insert(std::make_pair(201, "Created"));
	this->resCode.insert(std::make_pair(202, "Accepted"));
	this->resCode.insert(std::make_pair(203, "Non-Authoritative Information"));
	this->resCode.insert(std::make_pair(204, "No Content"));
	this->resCode.insert(std::make_pair(300, "Multiple Choices"));
	this->resCode.insert(std::make_pair(301, "Moved Permanently"));
	this->resCode.insert(std::make_pair(302, "Found"));
	this->resCode.insert(std::make_pair(303, "See Other"));
	this->resCode.insert(std::make_pair(304, "Not Modified"));
	this->resCode.insert(std::make_pair(305, "Use Proxy"));
	this->resCode.insert(std::make_pair(307, "Temporary Redirect"));
	this->resCode.insert(std::make_pair(400, "Bad Request"));
	this->resCode.insert(std::make_pair(401, "Unauthorized"));
	this->resCode.insert(std::make_pair(402, "Payment Required"));
	this->resCode.insert(std::make_pair(403, "Forbidden"));
	this->resCode.insert(std::make_pair(404, "Not Found"));
	this->resCode.insert(std::make_pair(405, "Method Not Allowed"));
	this->resCode.insert(std::make_pair(406, "Not Acceptable"));
	this->resCode.insert(std::make_pair(407, "Proxy Authentication Required"));
	this->resCode.insert(std::make_pair(408, "Request Timeout"));
	this->resCode.insert(std::make_pair(409, "Conflict"));
	this->resCode.insert(std::make_pair(410, "Gone"));
	this->resCode.insert(std::make_pair(411, "Length Required"));
	this->resCode.insert(std::make_pair(412, "Precondition Failed"));
	this->resCode.insert(std::make_pair(413, "Request Entity Too Large"));
	this->resCode.insert(std::make_pair(414, "Request-URI Too Long"));
	this->resCode.insert(std::make_pair(415, "Unsupported Media Type"));
	this->resCode.insert(std::make_pair(500, "Internal Server Error"));
	this->resCode.insert(std::make_pair(501, "Not Implemented"));
	this->resCode.insert(std::make_pair(502, "Bad Gateway"));
	this->resCode.insert(std::make_pair(503, "Service Unavailable"));
	this->resCode.insert(std::make_pair(504, "Gateway Timeout"));
}

Client::~Client() {}

int Client::parseLocation()
{
	statusCode = 200;
	if (req.getMIMEType() == "none")
		status |= IS_DIR;
	else
		status |= IS_FILE;
	if (loc->get_redirect().first && !(status & REDIRECT)) {
		if (makeRedirect(loc->get_redirect().first, loc->get_redirect().second)) {
			std::cout << CYAN << "REDIRECT" << RESET << "\n";
			return 0;
		}
	}
	size_t pos;
	std::string root = loc->get_root();
	std::string locn = loc->get_location();
	// if (locn[locn.size() - 1] != '/')
	// 	locn += "/";
	if (loc->get_accepted_methods().size())	{
		std::string method = "";
		for (size_t i = 0; i != loc->get_accepted_methods().size(); i++)
			if (req.getMethod() == loc->get_accepted_methods()[i])
				method = loc->get_accepted_methods()[i];
		if (!method.size())
		{
			std::cout << RED << "Method: " << req.getMethod() << " has 405 exception " << RESET << "\n";
			throw codeException(405);
		}
	}
	size_t subpos;
	locn[locn.size() - 1] == '/' ? subpos = locn.size() - 1 : subpos = locn.size();
	location = root + locn + req.getReqURI().substr(subpos);

	// FOR INTRA TESTER
	// std::cout << location << "\n";
	// if (location.find("directory") != std::string::npos) {
	// 	location.erase(location.find("directory"), 10);
	// 	std::cout << RED << "\e[1m  ALERT! tester stick trim /directory/" << RESET << "\n";
	// }
	// std::cout << location << '\n';
	// DELETE IT IN FINAL VERSION!

	while ((pos = location.find("//")) != std::string::npos)
		location.erase(pos, 1);
	if (location.size() > 1 && location[0] == '/')
		location = location.substr(1);
	if (status & IS_DIR)
	{
		if (location.size() && location[location.size() - 1] != '/')
		{
			statusCode = 301; // COMMENT IT FOR TESTER
			location.push_back('/');
			if (access(location.c_str(), 0) == -1)
			{
				std::cout << RED << "File not found (IS_DIR): " << location << RESET << "\n";
				throw codeException(404);
			}
			status |= REDIRECT;
			return 0;
		}
		else
		{ // COMMENT IT FOR TESTER
			std::vector<std::string> indexes = loc->get_index();
			int i = -1;
			if (!loc->get_autoindex())
			{
				while (++i < indexes.size())
				{
					std::string tmp = location + indexes[i];
					if (access(tmp.c_str(), 0) != -1)
					{
						location = tmp;
						req.setMIMEType(indexes[i]);
						break;
					}
				}
				if (i == indexes.size())
				{
					std::cout << RED << "Not found index in directory: " << location << RESET << "\n";
					throw codeException(404);
				}
			}
			else
			{
				if (access(location.c_str(), 0) == -1)
				{
					std::cout << RED << "No such directory: " << location << RESET << "\n";
					throw codeException(404);
				}
				status |= AUTOIDX;
			}
		} // COMMENT IT FOR TESTER
	}
	else if (status & IS_FILE)
	{ // FILE
		if (access(location.c_str(), 0) == -1)
		{
			std::cout << RED << "File not found (IS_FILE): " << location << RESET << "\n";
			throw codeException(404);
		}
	}
	if (access(location.c_str(), 4) == -1)
	{
		std::cout << RED << "Permisson denied: " << location << RESET << "\n";
		throw codeException(403);
	}
	// std::cout << GREEN << "this is final location: " << location << " <-\n" << RESET;
	return (0);
}

int Client::makeRedirect(int code, std::string loc)
{
	status |= REDIRECT;
	std::cout << "code - " << code << ", loc - " << loc << "\n";
	statusCode = code;
	// if (loc.find("http") != std::string::npos || loc.find("localhost") != std::string::npos)
	req.splitLocation(loc);
	req.splitDirectories();
	return 1;
}