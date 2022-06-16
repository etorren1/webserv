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
	if (!(file = fopen(fileLoc, "r")))
		throw codeException(404);
	fstat (fileno (file), &buff);
	fclose (file);
	return (buff.st_size);
}

void						closeAllFds(Client &client)
{
	(client.getPipe1());
}
// std::string	getTime()
// {
// 	char buf[1000];
// 	time_t now = time(0);
// 	struct tm tm = *gmtime(&now);
// 	strftime(buf, sizeof buf, "%a, %d %b %Y %H:%M:%S %Z", &tm);
// 	sprintf("Time is: [%s]\n", buf);
// 	return 0;
// };
