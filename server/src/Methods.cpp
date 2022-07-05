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

	// std::cout << BLUE << "ENTERED makePostResponse METOD" << RESET << "\n";

	char				buf[BUF];
	int					wr = 0;
	int					rd = 0;
	long				bytesRead = 0;

	// size_t lastRead = rdRet;
	// size_t lastWrite = wrtRet;

	if (reader_size == 0 && !(status & CGI_DONE)) {
		res.make_response_html(201, resCode[201]);
		status |= CGI_DONE;
	}
	if (!(status & CGI_DONE))	// флаг cgi записан == false
	{
		if (status & IS_WRITE) {
			bzero(buf, BUF);
			reader.read(buf, BUF);
			bytesRead = reader.gcount();
			wr = write(res.getPipeWrite(), buf, bytesRead);
			if (wr > 0) {
				wrtRet += wr;
				countw +=wr;
			}
			if (wr < bytesRead)
				reader.seekg(wrtRet);
			// if (wr == -1)
			// 	status &= ~IS_WRITE;
			if (countw >= BUF || wr == -1 || wrtRet == reader_size) {
				countw = 0;
				status &= ~IS_WRITE;
			}
			if (wrtRet >= reader_size)
				close(res.getPipeWrite());
			// checkTimeout2(wrtRet, lastWrite);
			// usleep (200);
			// std::cout << "Write: (" << wrtRet << ") wr: (" << wr << ") bytesRead: (" << bytesRead << ")\n"; // << CYAN << tmp << RESET << "\n";
		// 	std::string sada = buf;
		// 	std::cout << sada.substr(0, 5) << "\n";
		}
		else{
			bzero(buf, BUF);
			rd = read(res.getPipeRead(), buf, BUF);
			if (rd > 0) {
				rdRet += rd;
				countr += rd;
			}
			// if (rd == -1)
			// 	status |= IS_WRITE;
			if ((countr >= BUF || rd == -1)) {
				countr = 0;
				status |= IS_WRITE;
			}
			res.getStrStream() << buf;
			// checkTimeout2(rdRet, lastRead);
			// std::string sadae = buf;
			// std::cout << sadae.substr(0, 150) << "\n";
			// exit(1);
			// usleep (100);
			// std::cout << "Read: (" << rdRet << ") rd: (" << rd << ")\n"; // << CYAN << tmp << RESET << "\n";
			// if (rdRet > 99000000 && (rd == -1 || rd % 4048 == 0)) {
			// 	while (rdRet < wrtRet) {
			// 		rd = read(pipe2[PIPE_IN], buf, BUF);
			// 		if (rd > 0)
			// 			rdRet += rd;
			// 		std::cout << RED << "Read: (" << rdRet << ") rd: (" << rd << RESET << ")\n"; // << CYAN << tmp << RESET << "\n";
			// 		usleep(5000);
			// 	}
			// 	status |= IS_WRITE;
			// }
		}
		// if (reader.eof() && status & IS_WRITE) //SIGPIPE
		if (rdRet >= wrtRet && wrtRet == reader_size) //SIGPIPE
		{
			// AT THIS MOMENT NEED WRITE BODY FROM STREAM TO FILE !

			std::cout << "Write: (" << wrtRet << ") wr: (" << wr << ") bytesRead: (" << bytesRead << ")\n"; // << CYAN << tmp << RESET << "\n";
			std::cout << "Read: (" << rdRet << ") rd: (" << rd << ")\n"; // << CYAN << tmp << RESET << "\n";
			clearStream();
			
			close(res.getPipeRead());
			
			// char t[90];
			// res.getStrStream().read(t, 90);
			// res.getStrStream().seekg(0);
			// std::cout << YELLOW << t << RESET << "\n";
			statusCode = res.extractCgiHeader(req);
			res.wrRet = wrtRet;

			std::stringstream tmp;// need delete

			// res.getFileStream() << res.getStrStream().rdbuf();

			tmp << res.getStrStream().rdbuf(); // need delete
			clearStrStream(res.getStrStream());
			// res.make_response_header(req, statusCode, resCode[statusCode], getStrStreamSize(tmp));
			// if (wrtRet == 100000)
			// 	throw codeException(400);
				res.make_response_header(req, 201, resCode[201], getStrStreamSize(tmp));
			// else
			// 	res.make_response_header(req, 200, resCode[200], 0);
			res.getStrStream() << tmp.rdbuf(); // need delete
			status |= CGI_DONE;


		}
	}
	if (status & CGI_DONE)	//если все данные передались в cgi
	{
		if (res.sendResponse_stream(socket)) {
			std::cout << RED << "All sended" << RESET << "\n";
			status |= RESP_DONE;
		}
		// if (res.sendResponse_file(socket)) {

		// }
		// else
		// 	std::cout << RED << "not complete" << RESET << "\n";
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