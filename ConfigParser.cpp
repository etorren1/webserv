#include "Server.hpp"

typedef struct s_cfg
{
    std::string     hostname;
    std::string     port;
    std::list<std::string> locations;

} t_cfg;

int    get_block(const std::string& prompt,const std::string& content, std::string& dest, int last = 0)
{
    int pos = content.find(prompt);
    if (prompt.size() > content.size() || pos == std::string::npos)
        return (-1);
    int brackets = 1;
    int end = pos;
    while (content[end] != '{') {
        if (end++ == content.length()) {
            std::cout << RED << "ALERT! Not closed brackets!" << RESET << "\n";
            return (-1);
        }
    }
    end++;
    while (brackets) {
        if (content[end] == '{')
            brackets++;
        else if (content[end] == '}')
            brackets--;
        if (end++ == content.length()) {
            std::cout << RED << "ALERT! Not closed brackets!" << RESET << "\n";
            return (-1);
        }
    }
    dest = content.substr(pos, end - pos);
    return last + end;
}

std::string get_raw_param(std::string key, std::string & text)
{
    int pos = text.find(key);
    if (pos == std::string::npos)
        return "";
    int end = text.find("\n", pos) - 1;
    std::string res = text.substr(pos, end - pos);
    if (trim(res, " \n\t\r;").size() == key.size())
        return "";
    res.erase(0, key.size());
    return trim(res, " ;\n\r\t");
}

void listen(std::string & text, std::string & host, std::string & port )
{
    std::string raw;
    raw = get_raw_param("listen", text);
    if (!raw.size()) {
        std::cout << RED << "ALERT! host or port no such" << RESET << "\n";
        exit (255);
    }
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
    if (!port.size()) {
        std::cout << RED << "ALERT! no such port" << RESET << "\n";
        exit (255);
    }
}

void    parse_server( std::string & text, t_cfg *conf )
{
    std::string tmp;

    int last = 0;
    while ((last = get_block("location", &text[last], tmp, last)) > 0) {
        conf->locations.push_back(tmp);
    }
    listen(text, conf->hostname, conf->port);

    std::cout << conf->hostname << ":" << conf->port << "\n";
}

void    cut_comments( std::string & text )
{
    int rd;
    while ((rd = text.find("#")) != std::string::npos)
    {
        int end = text.find("\n", rd);
        text.replace(rd, text.size() - end, &text[end]);
        text.erase(text.size() - (end - rd));
    }
}

void    Server::parseConfig( const std::string & config ) {
    int fd = open(config.c_str(), O_RDONLY);
    char buf[512];
    int rd;
    std::string text;
    while ((rd = read(fd, buf, BUF_SIZE)) > 0) {
        buf[rd] = 0;
        text += buf;
    }

    t_cfg conf;

    cut_comments(text);

    parse_server(text, &conf);
    std::cout << conf.hostname << ":" << conf.port << "\n";
    srvPort = atoi(conf.port.c_str());

}