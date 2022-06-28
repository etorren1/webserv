#include "Server.hpp"

std::vector<std::string>	split(std::string str, std::string delimiter, std::string trimer)
{
	size_t pos = 0;
	std::string token;
	std::vector<std::string> res;
	while ((pos = str.find(delimiter)) != std::string::npos) {
		token = str.substr(0, pos);
		token = trim(token, trimer);
		res.push_back(token);
		str.erase(0, pos + delimiter.length());
	}
	if (!str.empty())
		res.push_back(str);
	return (res);
}

std::string					trim(std::string str, std::string cut)
{
	if (!cut.size())
		return str;
	size_t beg = str.find_first_not_of(cut);
	size_t end = str.find_last_not_of(cut);
	return str.substr(beg, end - beg + 1);
}

std::string					itos( long long const & num )
{
	std::stringstream ss;
	ss << num;
	return(ss.str());
}

bool						existDir(const char * name) { // don't work with relative path
	struct stat s;
	if (stat(name,&s)) return false;
	return S_ISDIR(s.st_mode);
}

long long					getFileSize(const char *fileLoc) //http://www.c-cpp.ru/content/fstat
{
	FILE *file;
	struct stat buff;
	if (!(file = fopen(fileLoc, "r"))) {
		std::cout << RED << "Can't open file (" << fileLoc << "): has 404 exception " << RESET << "\n";
		throw codeException(404);
	}
	fstat (fileno (file), &buff);
	fclose (file);
	return (buff.st_size);
}

size_t 		find_CRLN( char* buf, size_t size, size_t indent ) {
    for (size_t i = 0; i < size - 1; i++)
        if (buf[i] == '\r' && buf[i + 1] == '\n')
            return (i + indent);
    return (0);
}

long		getStrStreamSize(std::stringstream &strm)
{
    std::streambuf* buf = strm.rdbuf();
	long size = buf->pubseekoff(0, strm.end);
	buf->pubseekpos(0);
	
	return size;
}

std::string	getCurTime()
{
	char buf[1000];
	time_t now = time(0);
	struct tm tm = *localtime(&now);
	strftime(buf, sizeof buf, "%a %d %b %Y %H:%M:%S %Z", &tm);
	// sprintf("Time is: [%s]\n", buf);
	return buf;
}

void		clearStrStream(std::stringstream & strstream)
{
	strstream.seekg(0);
	strstream.str(std::string());
	strstream.clear();
}

long		hexadecimalToDecimal(std::string hex_val)
{
    int len = hex_val.size();

    int base = 1;
 
    long dec_val = 0;
    for (int i = len - 1; i >= 0; i--) {
		if (hex_val[i] >= 'a' && hex_val[i] <= 'f')
			hex_val[i] -= 32;
        if (hex_val[i] >= '0' && hex_val[i] <= '9') {
            dec_val += (int(hex_val[i]) - 48) * base;
            base = base * 16;
        }
        else if (hex_val[i] >= 'A' && hex_val[i] <= 'F') {
            dec_val += (int(hex_val[i]) - 55) * base;
            base = base * 16;
        }
    }
    return dec_val;
}
