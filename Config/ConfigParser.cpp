#include "../Server.hpp"
#include "InterfaceBlockCfg.hpp"

int    Server::get_block(const std::string& prompt,const std::string& content, std::string& dest, int last)
{
    int pos = content.find(prompt);
    if (prompt.size() > content.size() || pos == std::string::npos)
        return (0);
    int brackets = 1;
    int end = pos;
    while (content[end] != '{') {
        if (end++ == content.length())
            errorShutdown(255, http.get_error_log(), "error: configuration file: not closed brackets.", content);
    }
    end++;
    while (brackets) {
        if (content[end] == '{')
            brackets++;
        else if (content[end] == '}')
            brackets--;
        if (end++ == content.length())
            errorShutdown(255, http.get_error_log(), "error: configuration file: not closed brackets.", content);
    }
    dest = content.substr(pos, end - pos);
    return last + end;
}

bool    in_other_block(std::string & text, size_t pos) {
    size_t  open, close, start = text.find("{");
    start = text.find("{", start+1);
    open = text.find("{", start+1);
    close = open + 1;
    while (open < close) {
        open = text.find("{", open+1);
        close = text.find("}", close+1);
    }
    // std::cout << start << " " << open << " < " << close << " " << pos << "\n";
    if (pos > start && pos < close)
        return true;
    return false;
}

std::string Server::get_raw_param(std::string key, std::string & text)
{
    // if (key == "listen")
    //     std::cout << text << "\n";
    size_t pos = text.find(key);
    if (pos == std::string::npos)
        return "";
    if (in_other_block(text, pos))
        return "";
    size_t end = text.find("\n", pos) - 1;
    std::string res = text.substr(pos, end - pos);
    if (trim(res, " \n\t\r;{}").size() == key.size())
        return "";
    res.erase(0, key.size());
    return trim(res, " \n\t\r;{}");
}

// void Server::cfg_listen(std::string & text, Block & block )
// {
//     std::string raw;
//     raw = get_raw_param("listen", text);
//     if (!raw.size())
//         errorShutdown(255, http.get_error_log(), "error: configuration file: requaired parameter: listen.");
//     size_t sep = raw.find(":");
//     if (sep == std::string::npos) {
//         conf.port = raw;
//         conf.hostname = "0.0.0.0";
//     }
//     else {
//         conf.hostname = raw.substr(0, sep);
//         if (conf.hostname == "*" || !conf.hostname.size())
//             conf.hostname = "0.0.0.0";
//         conf.port = raw.substr(sep + 1, raw.size() - sep - 1);
//     }
//     if (!conf.port.size())
//         errorShutdown(255, http.get_error_log(), "error: configuration file: no such port.");
// }

template<class T>
void Server::cfg_listen(std::string & text, T & block )
{
    std::string raw;
    raw = get_raw_param("listen", text);
    if (!raw.size())
        errorShutdown(255, http.get_error_log(), "error: configuration file: requaired parameter: listen.");
    block.set_listen(raw);
}

template <class T>
void    Server::cfg_server_block( std::string & text, T & block )
{
    std::string tmp;

    int last = 0;
    while ((last = get_block("server", &text[last], tmp, last)) > 0) {
        Server_block *nw = new Server_block(block);
        
        cfg_listen(tmp, *nw);
        cfg_error_log(tmp, *nw);
        cfg_access_log(tmp, *nw);

        srvs.insert(std::make_pair(nw->get_listen(), nw));
    }
    std::map<std::string, Server_block*>::iterator it = srvs.begin();
    for ( ; it != srvs.end(); it++)
    {
        std::cout << (*it).first << "\n";
    }
    
    // cfg_access_log(text, block);
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

template <class T>
void    Server::cfg_error_log( std::string & text, T & block )
{
    std::string raw; 
    raw = get_raw_param("error_log", text);
    if (raw.size())
        block.set_error_log(raw);
}

template <class T>
void    Server::cfg_access_log( std::string & text, T & block )
{
    std::string raw; 
    raw = get_raw_param("access_log", text);
    if (raw.size())
        block.set_access_log(raw);
}

void    Server::parseConfig( const int & fd ) {
    char buf[BUF_SIZE];
    int rd;
    std::string text;
    while ((rd = read(fd, buf, BUF_SIZE)) > 0) {
        buf[rd] = 0;
        text += buf;
    } // read config file

    cut_comments(text);

    if ((rd = get_block("http", text, text)) == -1)
        errorShutdown(255, http.get_error_log(), "error: configuration file: not closed brackets.", text);
    cfg_error_log(text, http);
    cfg_access_log(text, http);
    cfg_server_block(text, http);
    close (fd);

    
    exit (1);
}