#include "Client.hpp"

void Client::findIndex( void ) {
    size_t i = -1;
    std::vector<std::string> indexes = loc->get_index();
    while (++i < indexes.size()) {
        std::string tmp = location + indexes[i];
        if (access(tmp.c_str(), 0) != -1)	{
            location = tmp;
            req.setMIMEType(indexes[i]);
            break;
        }
    }
    if (i == indexes.size())	{
        debug_msg(1, RED, "Not found index in directory: ", location);
        throw codeException(404);
    }
}

int Client::parseLocation()	{
	statusCode = 200;
	if (req.getMIMEType() == "none" && !req.getContType().size())
		status |= IS_DIR;
	else
		status |= IS_FILE;
	if (req.getMethod() != "DELETE" && loc->get_redirect().first && !(status & REDIRECT)) {
		if (makeRedirect(loc->get_redirect().first, loc->get_redirect().second)) {
			location = req.getReqURI();
			return 0;
		}
	}
	size_t pos;
    std::string root;
    if (req.getMethod() == "POST")
        root = loc->get_cgi_root();
    else
	    root = loc->get_root();
	std::string locn = loc->get_location();
	if (!loc->is_accepted_method(req.getMethod()))
	{
		debug_msg(1, RED, "Method: ", req.getMethod(), " has 405 exception");
		throw codeException(405);
	}
	size_t subpos;
	locn[locn.size() - 1] == '/' ? subpos = locn.size() - 1 : subpos = locn.size();
	location = root + locn + req.getReqURI().substr(subpos);

	if (TESTER) {
		// FOR INTRA TESTER
		if (location.find("directory") != std::string::npos) {
			location.erase(location.find("directory"), 10);
		}
		// DELETE IT IN FINAL VERSION!
	}

	while ((pos = location.find("//")) != std::string::npos)
		location.erase(pos, 1);
	if (location.size() > 1 && location[0] == '/')
		location = location.substr(1);
	if (req.getMethod() == "PUT")
		return 1;
	if (status & IS_DIR)	{
		// if (location.size() && location[location.size() - 1] != '/')	{ // FINAL IF
		if (location.size() && location[location.size() - 1] != '/' && !TESTER)	{
			statusCode = 301;
			location = req.getReqURI() + "/"; 
			status |= REDIRECT;
			return 0;
		} 
		else	
		{
			if (TESTER && location.size() && location[location.size() - 1] != '/') // FOR TESTER
				location += "/"; // FOR TESTER
			if (!loc->get_autoindex())
                findIndex();
			else	{
				if (access(location.c_str(), 0) == -1)		{
					debug_msg(1, RED, "No such directory: ", location);
					throw codeException(404);
				}
				status |= AUTOIDX;
			}
		}
	}
	else if (status & IS_FILE)	{ // FILE
		if (access(location.c_str(), 0) == -1) {
			debug_msg(1, RED, "File not found (IS_FILE): ", location);
			throw codeException(404);
		}
	}
	if (access(location.c_str(), 4) == -1)	{
		debug_msg(1, RED, "Permisson denied: ", location);
		throw codeException(403);
	}
	debug_msg(3, GREEN, "this is final location: ", RESET, location);
	return (0);
}

int Client::makeRedirect(int code, std::string loc){
	status |= REDIRECT;
	statusCode = code;
	req.splitLocation(loc);
	req.splitDirectories();
	return 1;
}

Location_block *Client::getLocationBlock(std::vector<std::string> vec) const {
	Location_block *lctn;
	size_t i = 0;
	while (i < vec.size()) {
		try {
			lctn = srv->lctn.at(vec[i]);
			return (lctn);
		}
		catch (const std::exception &e) {
			try {
				vec[i] += "/";
				lctn = srv->lctn.at(vec[i]);
				return (lctn);
			}
			catch (const std::exception &e) {
				i++;
			}
		}
	}
	return NULL;
}

bool			Client::readComplete() {
	lastActivity = timeChecker();
	return fullpart;
}

void Client::checkMessageEnd( void ) {
	if (status & IS_BODY) {
		if (req.getTransferEnc() == "chunked") {
			if (reader_size > 4) {
				char buf[5];
				bzero(buf, 5);
				reader.seekg(reader_size - 5);
				reader.read(buf, 5);
				reader.seekg(0);
				size_t pos = find_2xCRLN(buf, 5);
				if (pos && buf[pos - 1] == '0')
					fullpart = true;
				else
					fullpart = false;
			}
		}
		else if (req.getContentLenght().size()) {
			size_t len = atoi(req.getContentLenght().c_str());
			if (reader_size >= len)
				fullpart = true;
			else
				fullpart = false;
		}
		else {
			debug_msg(1, RED, "Unknown body type or encoding: has 415 exception");
			codeException(415);
		}
	}
	else {
		header = reader.str();
		size_t pos = header.find("\r\n\r\n");
		if (pos != std::string::npos) {
			if (pos + 4 != reader_size) // part of body request got into the header
				savePartOfStream(pos);
			else
				clearStream();
			header.erase(pos + 4);
			while (header.find("\r") != std::string::npos) {
				header.erase(header.find("\r"), 1); // из комбинации CRLF
				pos--;								// Удаляем символ возврата карретки
			}
			fullpart = true;
		}
		else
			fullpart = false;
	}
}
