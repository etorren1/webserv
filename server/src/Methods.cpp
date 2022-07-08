#include "Client.hpp"
#include "Utils.hpp"
#include <errno.h>

void Client::makeGetResponse( void )
{
	if (status & HEAD_SENT) {
		if (res.sendResponse_file(socket))
			status |= RESP_DONE;
	}
	else {
		if (res.sendResponse_stream(socket)) {
			if (status & REDIRECT)
				status |= RESP_DONE;
			else
				status |= HEAD_SENT;
		}
	}
	if (status & RESP_DONE)
		cleaner();
}

void Client::makePostResponse( void )
{
	char				buf[BUF];
	int					wr = 0;
	int					rd = 0;
	long				bytesRead = 0;

	if (reader_size == 0 && !(status & CGI_DONE)) {
		res.make_response_html(201, resCode[201]);
		status |= CGI_DONE;
	}
	if (!(status & CGI_DONE))
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
			if (countw >= BUF || wr == -1 || wrtRet == reader_size) {
				countw = 0;
				status &= ~IS_WRITE;
			}
			if (wrtRet >= reader_size)
				close(res.getPipeWrite());
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
			if ((countr >= BUF || rd == -1)) {
				countr = 0;
				status |= IS_WRITE;
			}
			res.getStrStream() << buf;
		}
		if (rdRet >= wrtRet && wrtRet == reader_size) //SIGPIPE
		{
			clearStream();
			
			close(res.getPipeRead());
			statusCode = res.extractCgiHeader(req);
			std::stringstream tmp;
			tmp << res.getStrStream().rdbuf();
			clearStrStream(res.getStrStream());
			if (TESTER) // tester stuff
				statusCode = 201;
			res.make_response_header(req, statusCode, resCode[statusCode], getStrStreamSize(tmp));
			res.getStrStream() << tmp.rdbuf();
			status |= CGI_DONE;
		}
	}
	else if (status & CGI_DONE)	//если все данные передались в cgi
	{
		debug_msg(3, RED,  "CGI_DONE FLAGE REACHED\n");
		if (res.sendResponse_stream(socket)) {
			status |= RESP_DONE;
		}
	}

	if (status & RESP_DONE)
		cleaner();
}

void Client::makeResponseWithoutBody() {
	if (res.sendResponse_stream(socket))
		status |= RESP_DONE;
	if (status & RESP_DONE)	{
		cleaner();
	}
}

void Client::makeErrorResponse() {
	if (status & HEAD_SENT) {
		if (status & IS_FILE) {
			if (res.sendResponse_file(socket))
				status |= RESP_DONE;
		}
		else
			status |= RESP_DONE;
	}
	else {
		if (res.sendResponse_stream(socket))
			status |= HEAD_SENT;
	}
	if (status & RESP_DONE) {
		cleaner();
	}
}

