#include "Client.hpp"
#include "Utils.hpp"
#include <errno.h>

void Client::makePostResponse(char **envp)
{

	std::cout << BLUE << "ENTERED makePostResponse METOD" << "\n" << RESET;

	char				buff[BUF];
	int					wrtRet = 0;
	int					readRet = 0;
	int					code;
	char				buffer[200];

	if (cgiWriteFlag == false)	// флаг cgi записан == false
	{
		std::string s = reader.str();
		std::string tmp(s.substr(0, 2000));
		while (tmp.length()) {
			wrtRet = write(pipe1[PIPE_OUT], tmp.c_str(), tmp.length());

			if (wrtRet == -1) {
				std::cout << "WRTRET:" << wrtRet << "\n";
				strerror_r( errno, buffer, 256 ); // to remove
				std::cout << "ERRNO: " << buffer << "\n"; // to remove

				int qq = 0;
				// readRet = read(pipe2[PIPE_IN], buff, BUF);
				readRet = 1;
				while (readRet > 0) {
					readRet = read(pipe2[PIPE_IN], buff, BUF);
					qq += readRet;
					std::cout << "after read from = " << qq << "\n";
				}
				wrtRet = write(pipe1[PIPE_OUT], tmp.c_str(), tmp.length());
				std::cout << "WRTRET:" << wrtRet << "\n";
				strerror_r( errno, buffer, 256 ); // to remove
				std::cout << "ERRNO: " << buffer << "\n"; // to remove
				buff[readRet] = '\0';
				std::cout << "BUFFER: " << buff << std::endl;
			}
			else
			{
				std::cout << "WRTRET NON ERROR:" << wrtRet << "\n";
			}
			s.erase(s.begin(), s.begin() + 2000);
			tmp = s.substr(0, 2000);
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