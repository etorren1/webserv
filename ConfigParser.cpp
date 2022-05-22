#include "Server.hpp"
#include <sys/stat.h> // for make directory

int    Server::get_block(const std::string& prompt,const std::string& content, std::string& dest, int last)
{
    int pos = content.find(prompt);
    if (prompt.size() > content.size() || pos == std::string::npos)
        return (0);
    int brackets = 1;
    int end = pos;
    while (content[end] != '{') {
        if (end++ == content.length())
            errorShutdown(255, "error: configuration file: not closed brackets.", content);
    }
    end++;
    while (brackets) {
        if (content[end] == '{')
            brackets++;
        else if (content[end] == '}')
            brackets--;
        if (end++ == content.length())
            errorShutdown(255, "error: configuration file: not closed brackets.", content);
    }
    dest = content.substr(pos, end - pos);
    return last + end;
}

std::string Server::get_raw_param(std::string key, std::string & text)
{
    int pos = text.find(key);
    if (pos == std::string::npos)
        return "";
    int end = text.find("\n", pos) - 1;
    std::string res = text.substr(pos, end - pos);
    if (trim(res, " \n\t\r;{}").size() == key.size())
        return "";
    res.erase(0, key.size());
    return trim(res, " \n\t\r;{}");
}

void Server::cfg_listen(std::string & text, std::string & host, std::string & port )
{
    std::string raw;
    raw = get_raw_param("listen", text);
    if (!raw.size())
        errorShutdown(255, "error: configuration file: requaired parameter: listen.");
    int sep = raw.find(":");
    if (sep == std::string::npos) {
        port = raw;
        host = "0.0.0.0";
    }
    else {
        host = raw.substr(0, sep);
        if (host == "*" || !host.size())
            host = "0.0.0.0";
        port = raw.substr(sep + 1, raw.size() - sep - 1);
    }
    if (!port.size())
        errorShutdown(255, "error: configuration file: no such port.");
}

void    Server::cfg_server_block( std::string & text, t_cfg *conf )
{
    std::string tmp;

    int last = 0;
    while ((last = get_block("location", &text[last], tmp, last)) > 0) {
        conf->locations.push_back(tmp);
    }
    cfg_access_log(text);
    cfg_listen(text, conf->hostname, conf->port);
}

void    Server::cut_comments( std::string & text )
{
    int rd;
    while ((rd = text.find("#")) != std::string::npos)
    {
        int end = text.find("\n", rd);
        text.replace(rd, text.size() - end, &text[end]);
        text.erase(text.size() - (end - rd));
    }
}

void    rek_mkdir( std::string path)
{
    int sep = path.find_last_of("/");
    std::string create = path;
    if (sep != std::string::npos) {
        rek_mkdir(path.substr(0, sep));
        path.erase(0, sep);
    }
    mkdir(create.c_str(), 0777);
}

void    Server::cfg_error_log( std::string & text )
{
    std::string raw;
    raw = get_raw_param("error_log", text);
    if (!raw.size()) {
        return ;
    }
    conf.error_fd = open(raw.c_str(), O_RDWR | O_CREAT | O_TRUNC , 0777);
    if (conf.error_fd < 0) {
        int sep = raw.find_last_of("/");
        if (sep != std::string::npos) {
            rek_mkdir(raw.substr(0, sep));
        }
        conf.error_fd = open(raw.c_str(), O_RDWR | O_CREAT | O_TRUNC , 0777);
        if (conf.error_fd < 0) {
            std::cerr << RED << "Error: can not open or create error_log file" << RESET << "\n";
            return ;
        }
    }
    flags |= ERR_LOG;
}

void    Server::cfg_access_log( std::string & text )
{
    std::string raw;
    raw = get_raw_param("access_log", text);
    if (!raw.size()) {
        return ;
    }
    conf.access_fd = open(raw.c_str(), O_RDWR | O_CREAT | O_TRUNC , 0777);
    if (conf.access_fd < 0) {
        int sep = raw.find_last_of("/");
        if (sep != std::string::npos) {
            rek_mkdir(raw.substr(0, sep));
        }
        conf.access_fd = open(raw.c_str(), O_RDWR | O_CREAT | O_TRUNC , 0777);
        if (conf.access_fd < 0) {
            std::cerr << RED << "Error: can not open or create access_log file" << RESET << "\n";
            return ;
        }
    }
    flags |= ACS_LOG;
}

void    Server::parseConfig( const int & fd ) {
    char buf[512];
    int rd;
    std::string text;
    while ((rd = read(fd, buf, BUF_SIZE)) > 0) {
        buf[rd] = 0;
        text += buf;
    } // read config file

    cut_comments(text);
    cfg_error_log(text); // get error_log path and create file if it not exist
    if (get_block("server", text, text) == -1)
        errorShutdown(255, "error: configuration file: not closed brackets.", text);
    cfg_server_block(text, &conf);

}