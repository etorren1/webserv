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
	// std::cout << PURPLE << "\e[1mnum in itos = " << num << RESET << "\n";
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
        debug_msg(1, RED,  "Can't open file: ", fileLoc);
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

size_t 		find_2xCRLN( char* buf, size_t size, size_t indent ) {
	if (size < 4)
		return (0);
    for (size_t i = 0; i < size - 3; i++)
        if (buf[i] == '\r' && buf[i + 1] == '\n'
			&& buf[i + 2] == '\r' && buf[i + 3] == '\n')
            return (i + indent);
    return (0);
}

std::string 	getstr(char *c, size_t size) {
    std::string str;
    for (size_t i = 0; i < size; i++)
        str += c[i];
    return str;
}

size_t		getStrStreamSize(std::stringstream &strm)
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

void	rek_mkdir( std::string path)
{
    size_t sep = path.find_last_of("/");
    std::string create = path;
    if (sep != std::string::npos) {
        rek_mkdir(path.substr(0, sep));
        path.erase(0, sep);
    }
    mkdir(create.c_str(), 0777);
}

time_t timeChecker() { 
    time_t result = time(0);
    return (intmax_t)result;
}

void	writeLog( const std::string & path, const std::string & header, const std::string & text ) {
    if (path != "off") {
		struct stat st;
		stat(path.c_str(), &st);
		size_t size = st.st_size;
		int flags = 0, fd;
		size > 1000000 ? flags = O_RDWR | O_CREAT | O_TRUNC : flags = O_RDWR | O_CREAT | O_APPEND;
        fd = open(path.c_str(), flags, 0777);
        if (fd < 0) {
            size_t sep = path.find_last_of("/");
            if (sep != std::string::npos) {
                rek_mkdir(path.substr(0, sep));
            }
            fd = open(path.c_str(), flags, 0777);
            if (fd < 0) {
                debug_msg(1, RED, "Error: can not open or create log file");
                return ;
            }
        }
        if (fd) {
            std::time_t result = std::time(NULL);
            std::string time = std::asctime(std::localtime(&result));
            time = "[" + time.erase(time.size() - 1) + "] ";
            write(fd, time.c_str(), time.size());
            write(fd, header.c_str(), header.size());
            write(fd, "\n", 1);
            write(fd, text.c_str(), text.size());
            write(fd, "\n", 1);
            close (fd);
        }
    }
}

void debug_msg(int lvl, std::string m1, std::string m2, std::string m3, std::string m4,\
	                     std::string m5, std::string m6, std::string m7, std::string m8) {
    if (DEBUGLVL >= lvl) {
        usleep(300);
        std::cerr << m1 << m2 << m3 << m4 << m5 << m6 << m7 << m8 << RESET << "\n";
    }
}