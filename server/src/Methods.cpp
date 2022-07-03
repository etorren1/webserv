#include "Client.hpp"
#include "Utils.hpp"
#include <errno.h>

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
			if (status & REDIRECT) {
				std::cout << location << "<-\n";
				std::cout << RED << "\e[1mDONE" <<RESET << "\n";
				status |= RESP_DONE;
			}
			else
				status |= HEAD_SENT;
			// std::cout << "GET sendstream() must be first\n";
		}
	}
	if (status & RESP_DONE)
	{
		// std::cout << GREEN << "End GET response on " << socket << " socket" << RESET << "\n";
		cleaner();
		// std::cout << "GET cleaner() must be ending\n";
	}
}

void Client::makePostResponse(char **envp)
{

	std::cout << BLUE << "ENTERED makePostResponse METOD" << RESET << "\n";

	char				buf[BUF];
	int					wr = 0;
	int					rd = 0;
	int					code;
	char				buffer[200];
	long				bytesRead = 0;

	if (cgiWriteFlag == false)	// флаг cgi записан == false
	{
		if (status & IS_WRITE) {
			bzero(buf, BUF);
			reader.read(buf, BUF);
			bytesRead = reader.gcount();
			wr = write(pipe1[PIPE_OUT], buf, bytesRead);
			usleep (1000);
			if (wr > 0)
				wrtRet += wr;
			if (wr < bytesRead)
				reader.seekg(wrtRet);
			if (wr == -1)
				status &= ~IS_WRITE;
			std::cout << "Write: (" << wrtRet << ") wr: (" << wr << ") bytesRead: (" << bytesRead << ")\n"; // << CYAN << tmp << RESET << "\n";
		// 	std::string sada = buf;
		// 	std::cout << sada.substr(0, 5) << "\n";
		}
		else {
			bzero(buf, BUF);
			rd = read(pipe2[PIPE_IN], buf, BUF);
			usleep (1000);
			rdRet += rd;
			if (rd == -1)
				status |= IS_WRITE;
			res.getStrStream() << buf;
			// std::string sadae = buf;
			// std::cout << sadae.substr(0, 150) << "\n";
			// exit(1);
			// std::cout << "Read: (" << rdRet << ") rd: (" << rd << ")\n"; // << CYAN << tmp << RESET << "\n";
		}
		if (reader.eof() && status & IS_WRITE) //SIGPIPE
		{
			clearStream();
			close(pipe1[PIPE_OUT]);
			close(pipe2[PIPE_IN]);
			statusCode = res.extractCgiHeader(req);

			std::stringstream tmp;
			tmp << res.getStrStream().rdbuf();
			clearStrStream(res.getStrStream());
			res.make_response_header(req, statusCode, resCode[statusCode], -1);
			// res.getStrStream() << tmp;
			cgiWriteFlag = true;
		}
	}

	if (cgiWriteFlag == true)											//если все данные передались в cgi
	{
		if (res.sendResponse_stream(socket))
			status |= RESP_DONE;
		else
			std::cout << RED << "not complete" << RESET << "\n";
	}
	if (status & RESP_DONE)
	{
		cleaner();
		std::cout << BLUE <<  "\e[1mCOMPLEATING POST RESPONSE! CONGRATULATIONS\n"; 
	}
}

void Client:: makeDeleteResponse(char **envp)	{
	std::cout << RED << "DELETE\n" << RESET;
	if (remove(location.c_str()) != 0) 
		codeException(403);
	else {
		statusCode = 204;
		// initResponse(envp);
		res.setFileLoc(location);
		clearStrStream(res.getStrStream());
		res.make_response_html(204, resCode[204]);

		// res.make_response_header(req, 204, resCode[204], res.getContentLenght());
		if (res.sendResponse_stream(socket))  {
			status |= RESP_DONE;
			cleaner();
		}
	}
}

void Client:: makePutResponse(char **envp)	{
	std::cout << RED << "PUT\n" << RESET;
	std::ofstream file(location);
	std::cout << GREEN << location << "\n" << RESET;
	if (!file.is_open()) {
		int sep = location.find_last_of("/");
		if (sep != std::string::npos) {
			rek_mkdir(location.substr(0, sep));
		}
		file.open(location);
	}
	if (file.is_open()) {
		std::cout << GREEN << "if file is_open - " << req.getReqURI() << ", location - " << location << "\n" << RESET;
		file << reader.str();
		file.close();
	} else {
		std::cout << RED << "File is not open: " << location << ", code - 406" << RESET << "\n";
		throw codeException(406);
	}
	statusCode = 201;
	res.setFileLoc(location);
	clearStrStream(res.getStrStream());
	res.make_response_html(201, resCode[201]);
	if (res.sendResponse_stream(socket))  {
		status |= RESP_DONE;
		cleaner();
	}
	// exit(1);
}