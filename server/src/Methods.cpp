#include "Client.hpp"
#include "Utils.hpp"
#include <errno.h>

void Client::makeGetResponse( void )
{
	if (status & HEAD_SENT) {
		if (res.sendResponse_file(socket, lastActivity))
			status |= RESP_DONE;
	}
	else {
		if (res.sendResponse_stream(socket, lastActivity)) {
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
		if (status & IS_WRITE && wrtRet != reader_size) {
			bzero(buf, BUF);
			reader.read(buf, BUF);
			bytesRead = reader.gcount();
			wr = write(res.getPipeWrite(), buf, bytesRead);
			if (wr > 0) {
				lastActivity = timeChecker();
				wrtRet += wr;
				countw +=wr;
			}
			if (wr < bytesRead)
				reader.seekg(wrtRet);
			if (countw >= BUF || wr <= 0 || wrtRet == reader_size) {
				countw = 0;
				status &= ~IS_WRITE;
			}
			// if (wrtRet >= getStrStreamSize(reader))
			if (wrtRet >= reader_size)
				close(res.getPipeWrite());
			std::cout << "wrtRet = " << wrtRet << " reader_size = " << reader_size << " wr = "<< wr << " bytesRead = " << bytesRead << "\n";
			usleep(100);
		}
		else{
			bzero(buf, BUF);
			rd = read(res.getPipeRead(), buf, BUF);
			if (rd > 0) {
				lastActivity = timeChecker();
				rdRet += rd;
				countr += rd;
			}
			if ((countr >= BUF || rd <= 0)) {
				countr = 0;
				status |= IS_WRITE;
			}
			res.getStrStream() << buf;
			std::cout << "rdRet = " << rdRet << " rd = "<< rd << "\n";
			usleep(100);
		}
		if (res.waitChild() && wrtRet == reader_size && rd <= 0) {
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
		if (res.sendResponse_stream(socket, lastActivity)) {
			status |= RESP_DONE;
		}
	}
	if (status & RESP_DONE)
		cleaner();
}

void Client::makeResponseWithoutBody() {
	if (res.sendResponse_stream(socket, lastActivity))
		status |= RESP_DONE;
	if (status & RESP_DONE)	{
		cleaner();
	}
}

void Client::makeErrorResponse() {
	if (status & HEAD_SENT) {
		if (status & IS_FILE) {
			if (res.sendResponse_file(socket, lastActivity))
				status |= RESP_DONE;
		}
		else
			status |= RESP_DONE;
	}
	else {
		if (res.sendResponse_stream(socket, lastActivity))
			status |= HEAD_SENT;
	}
	if (status & RESP_DONE) {
		cleaner();
	}
}

