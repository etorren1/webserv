#include "../Server.hpp"
#include "InterfaceBlockCfg.hpp"

int    Server::get_block(const std::string& prompt, const std::string& content, std::string& dest, int last) {
    int pos = content.find(prompt);
    if (prompt.size() > content.size() || pos == std::string::npos)
        return (0);
    int brackets = 1;
    int end = pos;
    while (content[end] != '{') {
        if (end++ == content.length())
            errorShutdown(255, http->get_error_log(), "error: configuration file: not closed brackets.", content);
    }
    end++;
    while (brackets) {
        if (content[end] == '{')
            brackets++;
        else if (content[end] == '}')
            brackets--;
        if (end++ == content.length())
            errorShutdown(255, http->get_error_log(), "error: configuration file: not closed brackets.", content);
    }
    dest = content.substr(pos, end - pos);
    return last + end;
}

static bool    in_other_block(std::string & text, size_t pos) {
    size_t begin, end;
    begin = text.find("{");
    end = text.find("{", begin + 1);
    if (end == std::string::npos)
            return false;
    while (begin != std::string::npos) {
        if (end == std::string::npos && pos > begin && pos < end)
            return false;
        if ( pos > begin && pos < end )
            return false;
        begin = text.find("}", begin + 1);
        end = text.find("{", end + 1);
    }
    return true;
}

std::string Server::get_raw_param(std::string key, std::string & text) {
    size_t pos = text.find(key);
    if (pos == std::string::npos)
        return "";
    if (in_other_block(text, pos))
        return "";
    size_t end = text.find("\n", pos);
    std::string res = text.substr(pos, end - pos);
    if (trim(res, " \n\t\r;{}").size() == key.size())
        return "";
    res.erase(0, key.size());
    return trim(res, " \n\t\r;{}");
}

void    Server::cut_comments( std::string & text ) {
    int rd;
    while ((rd = text.find("#")) != std::string::npos)
    {
        int end = text.find("\n", rd);
        text.replace(rd, text.size() - end, &text[end]);
        text.erase(text.size() - (end - rd));
    }
}

template<class T>
void Server::cfg_listen(std::string & text, T * block ) {
    std::string raw = get_raw_param("listen", text);
    if (!raw.size()) {
        delete block; block = NULL;
        errorShutdown(255, http->get_error_log(), "error: configuration file: requaired parameter: listen.");
    }
    if (raw.find_first_not_of("0123456789.:*") != std::string::npos) {
        delete block; block = NULL;
        errorShutdown(255, http->get_error_log(), "error: configuration file: bad parameter: listen.");
    }
    std::string hostname, port;
    size_t sep = raw.find(":");
    if (sep == std::string::npos) {
        port = raw;
        hostname = "0.0.0.0";
    }
    else {
        hostname = raw.substr(0, sep);
        if (hostname == "*" || !hostname.size())
            hostname = "0.0.0.0";
        port = raw.substr(sep + 1, raw.size() - sep - 1);
    }
    block->set_listen(hostname + ":" + port);
}

template <class T>
void    Server::cfg_server_name(std::string & text, T * block ) {
    std::string raw = get_raw_param("server_name", text);
    if (raw.size())
        block->set_server_name(raw);
}

template <class T>
void    Server::cfg_index(std::string & text, T * block ) {
    std::string raw = get_raw_param("index", text);
    if (raw.size())
        block->set_index(raw);
}

template <class T>
void    Server::cfg_accepted_methods(std::string & text, T * block ) {
    std::string raw = get_raw_param("accepted_methods", text);
    if (raw.size())
        block->set_accepted_methods(raw);
}

template <class T>
void    Server::cfg_location_block( std::string & text, T * block ) {
    std::string tmp;

    int last = 0;
    while ((last = get_block("location", &text[last], tmp, last)) > 0) {
        Location_block *nw = new Location_block(*block);

        cfg_set_attributes(tmp, nw);
        std::string raw = get_raw_param("location", tmp);
        if (!raw.size()) {
            delete nw;
            errorShutdown(255, http->get_error_log(), "error: configuration file: invalid value: location.");
        }
        nw->set_location(raw);
        block->lctn.insert(std::make_pair(raw, nw));
    }
}

template <class T>
void    Server::cfg_server_block( std::string & text, T * block ) {
    std::string tmp;

    int last = 0;
    while ((last = get_block("server", &text[last], tmp, last)) > 0) {
        Server_block *nw = new Server_block(*block);

        cfg_listen(tmp, nw);
        cfg_server_name(tmp, nw);
        cfg_set_attributes(tmp, nw);
        cfg_location_block(tmp, nw);
        srvs.insert(std::make_pair(nw->get_listen(), nw));
    }
}


template <class T>
void    Server::cfg_error_log( std::string & text, T * block ) {
    std::string raw = get_raw_param("error_log", text);
    if (raw.size())
        block->set_error_log(raw);
}

template <class T>
void    Server::cfg_access_log( std::string & text, T * block ) {
    std::string raw = get_raw_param("access_log", text);
    if (raw.size())
        block->set_access_log(raw);
}

template <class T>
void    Server::cfg_root( std::string & text, T * block ) {
    std::string raw = get_raw_param("root", text);
    if (raw.size())
        block->set_root(raw);
}

template <class T>
void    Server::cfg_sendfile( std::string & text, T * block ) {
    std::string raw = get_raw_param("sendfile", text);
    if (raw.size()) {
        if (raw == "on")
            block->set_sendfile(true);
        else if (raw == "off")
            block->set_sendfile(false);
        else {
            delete block; block = NULL;
            errorShutdown(255, http->get_error_log(), "error: configuration file: invalid value: sendfile.");
        }
    }
}

template <class T>
void    Server::cfg_autoindex( std::string & text, T * block ) {
    std::string raw = get_raw_param("autoindex", text);
    if (raw.size()) {
        if (raw == "on")
            block->set_autoindex(true);
        else if (raw == "off")
            block->set_autoindex(false);
        else {
            delete block; block = NULL;
            errorShutdown(255, http->get_error_log(), "error: configuration file: invalid value: autoindex.");
        }
    }
}

template <class T>
void    Server::cfg_client_max_body_size( std::string & text, T * block ) {
    std::string raw = get_raw_param("client_max_body_size", text);
    if (raw.find_first_not_of("0123456789Mm") != std::string::npos) {
        delete block; block = NULL;
        errorShutdown(255, http->get_error_log(), "error: configuration file: invalid value: client_max_body_size.");
    }
    int     size = atoi(raw.c_str());
    if (raw[raw.size() - 1] == 'm' || raw[raw.size() - 1] == 'M')
        size *= 1024;
    if (size != 0)
        block->set_client_max_body_size(size);
}

template <class T>
void    Server::cfg_return( std::string & text, T * block ) {
    std::string raw = get_raw_param("return", text);
    if (raw.size()) {
        int code = atoi(raw.c_str());
        if (code < 300 || code > 309)
            errorShutdown(255, http->get_error_log(), "error: configuration file: invalid value: return.");
        std::string nwloc = trim(raw.substr(3), " \t");
        block->set_redirect(code, nwloc);
    }
}

template <class T>
void    Server::cfg_error_page( std::string & text, T * block ) {
    std::string raw = get_raw_param("error_page", text);
    if (raw.size()) {
        int code = atoi(raw.c_str());
        if (code < 400 || code > 509)
            errorShutdown(255, http->get_error_log(), "error: configuration file: invalid value: error_page.");
        std::string nwloc = trim(raw.substr(3), " \t");
        block->set_error_page(code, nwloc);
    }
}

template <class T>
void    Server::cfg_set_attributes( std::string & text, T * block ) {
    cfg_error_log(text, block);
    cfg_access_log(text, block);
    cfg_sendfile(text, block);
    cfg_autoindex(text, block);
    cfg_index(text, block);
    cfg_root(text, block);
    cfg_return(text, block);
    cfg_error_page(text, block);
    cfg_accepted_methods(text, block);
    cfg_client_max_body_size(text, block);
}

void    Server::config( const int & fd ) {
    if (fd == -1) {
        status = STOP;
        if (http->get_error_log() != "off") {
            writeLog(http->get_error_log(), "error: configuration file: bad descriptor.");
            std::cerr << "error: see error_log for more information\n";
        }
        exit(1);
    }
    char buf[BUF_SIZE];
    int rd;
    std::string text;
    while ((rd = read(fd, buf, BUF_SIZE)) > 0) {
        buf[rd] = 0;
        text += buf;
    } // read config file


    http = new Http_block();

    cut_comments(text);

    if ((rd = get_block("http", text, text)) == -1)
        errorShutdown(255, http->get_error_log(), "error: configuration file: not closed brackets.", text);
    cfg_set_attributes(text, http);
    cfg_server_block(text, http);

    // std::cout << YELLOW << "http block" << RESET << "\n";
    // http->show_all();

    // for (srvs_iterator it = srvs.begin(); it != srvs.end(); it++) {

    //     std::cout << RED << (*it).first << RESET << "\n";
    //     (*it).second->show_all();

    //     for (lctn_iterator jt = (*it).second->lctn.begin(); jt != (*it).second->lctn.end(); jt++) {

    //         std::cout << GREEN << (*jt).first << RESET << "\n";
    //         (*jt).second->show_all();
    //     }
    // }
    // exit(1);
    close (fd);
    fcntl(fileno(stdin), F_SETFL, O_NONBLOCK);
}