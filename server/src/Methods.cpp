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

	std::cout << BLUE << "ENTERED makePostResponse METOD" << "\n" << RESET;

	char				buff[BUF];
	long				wrtRet = 0;
	int					wr = 0;
	int					readRet = 0;
	int					code;
	char				buffer[200];
	long				bytesRead;

	if (cgiWriteFlag == false)	// флаг cgi записан == false
	{
		while (wrtRet < reader_size) {
			reader.read(buff, BUF);
			bytesRead = reader.gcount();
			wr = write(pipe1[PIPE_OUT], buff, bytesRead);
			usleep (1000);
			if (wr > 0)
				wrtRet += wr;
			if (wr < bytesRead)
				reader.seekg(wrtRet - 1);
			if (wr == 0) {
				std::cout << "wr == 0\n";
				int i = 0;
				while (i < bytesRead) {
					printf("%d ", buff[i++]);
				}
				printf("\n");
				exit(1);
			}
			std::cout << "Write: (" << wrtRet << ") wr: (" << wr << ") bytesRead : (" << bytesRead << ")\n"; // << CYAN << tmp << RESET << "\n";
		}
		exit (0);
		{
			readRet = read(pipe2[PIPE_IN], buff, BUF);
			std::cout << "Read: (" << readRet << ") \n";// << PURPLE << buff << RESET << "\n";

			// usleep(1000);

			if (wrtRet == -1) {
				std::cout << "WRTRET:" << wrtRet << "\n";
				strerror_r( errno, buffer, 256 ); // to remove
				std::cout << "ERRNO: " << buffer << "\n"; // to remove

				int qq = 0;
				// readRet = read(pipe2[PIPE_IN], buff, BUF);
				readRet = 1;
				// while (readRet > 0) {
				// 	readRet = read(pipe2[PIPE_IN], buff, BUF);
				// 	qq += readRet;
				// 	std::cout << "after read from = " << qq << "\n";
				// }
				// wrtRet = write(pipe1[PIPE_OUT], tmp.c_str(), tmp.length());
				std::cout << "WRTRET:" << wrtRet << "\n";
				strerror_r( errno, buffer, 256 ); // to remove
				std::cout << "ERRNO: " << buffer << "\n"; // to remove
				buff[readRet] = '\0';
				std::cout << "BUFFER: " << buff << std::endl;
				// exit(1);
			}
			else
			{
				// std::cout << "WRTRET NON ERROR:" << wrtRet << "  " << s.size() << "\n";
			}
			// s.erase(s.begin(), s.begin() + 10000);
			// tmp = s.substr(0, 10000);
		}
		// wrtRet = write(pipe1[PIPE_OUT], reader.str().c_str(), reader.str().length()); // pipe
		// std::cout << "WRTRET:" << wrtRet << "\n";
		// strerror_r( errno, buffer, 256 ); // to remove
		// std::cout << "ERRNO: " << buffer << "\n"; // to remove

		totalSent += wrtRet;
		if (totalSent == reader.str().length()) //SIGPIPE
		{
			close(pipe1[PIPE_OUT]);
			cgiWriteFlag = true;
		}
	}

	if (cgiWriteFlag == true)											//если все данные передались в cgi
	{
		std::cout << BLUE << "READING FROM PIPE1 started" << "\n" << RESET;
		readRet = read(pipe2[PIPE_IN], buff, BUF);						//читаем из cgi порцию даты, прочитанный кусок из cgi пишем клиенту в сокет
		std::cout << "READRETURN = " << readRet << "\n";
		if (!(status & HEAD_SENT))
		{
			extractCgiHeader(buff);
			status |= HEAD_SENT;
		}
		else
		{
			// std::cout << "readRet " << readRet << "\n";
			if (readRet == -1)
			{
			std::cout << RED << "HERE4" << RESET << "\n";
			std::cout << "ERRNO: " << buffer << "\n"; // to remove
				throw(codeException(500)); // ЗАШЕЛ СЮДА
			}
			if(status & STRM_READY)										//весь body уже записан в поток, отпавляем его частями клиенту, в нижние условия больше не заходим до конца response
			{
				if (res.sendResponse_stream(socket))
					status |= RESP_DONE;
			}
			else if (readRet != 0 || iter == 0)													//записываем из буффера часть данных в поток
			{
				buff[readRet] = '\0';
				std::cout << RED << "BUFF : " << buff << RESET << "\n";
				res.getStrStream().write(buff, readRet); //вот тут появляется мусор!!!
				std::cout << RED << "STREAM after BUFFER : " << res.getStrStream().str() << RESET << "\n";
				iter++;
			}
			else if (readRet == 0)										//0 запишется в readRet один раз и больше не будет меняться до конца response
			{
				close(pipe2[PIPE_IN]); // ikael changes
				// std::cout << BLUE << "READ STOPED" << "\n" << RESET;
				std::cout << "STRING: " << res.getStrStream().str() << "\n";
				std::stringstream tmp;									//нужен, чтобы хедер записать в начало основного потока
				status |= STRM_READY;									//все прочитали из cgi
				code = std::atoi(req.getCgiStatusCode().c_str());
				
				tmp << res.getStrStream().rdbuf();						//временyо перекладываем записанный в _strstream body в другой поток
				std::cout << "TMP: " << tmp.str() << "\n";
				clearStrStream(res.getStrStream());						//очищаем _stream
				res.make_response_header(req, code, resCode[code], getStrStreamSize(tmp));
				res.getStrStream() << tmp.str();						//!!! Возможно все не влезет в объет строки
				/*	The buffer of a std::stringstream object is a wrapper around a std::string object.
					As such, the maximum size is std::string::max_size().
					https://stackoverflow.com/questions/22025324/what-is-the-maximum-size-of-stdostringstream-buffer */
				std::cout << "_BODY: " << res.getStrStream().str();
			}
		}
	}
	if (status & RESP_DONE)
	{
		// close(pipe1[PIPE_IN]);
		waitpid(pid, &status, 0); // ???
		cleaner();
		close(pipe2[PIPE_OUT]);
		// std::cout << "COMPLEATING POST RESPONSE2\n"; 
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