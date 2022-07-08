#ifndef InterfaceBlockCfg_hpp
#define InterfaceBlockCfg_hpp

#include "../Utils.hpp"

class Block
{
    public:

        std::string     error_log;
        std::string     access_log;
        std::string     root;
        std::string     cgi_root;
        std::vector<std::string>     accepted_methods;
        std::vector<std::string>     index;
        std::vector<std::string>     cgi_index;
        bool            sendfile;
        bool            autoindex;
        size_t          client_max_body_size;
        std::pair<int, std::string> redirect;
        std::pair<int, std::string> error_page;

        void set_error_log( const std::string & s ) { error_log = s; }
        void set_access_log( const std::string & s ) { access_log = s; }
        void set_sendfile( bool t ) { sendfile = t; }
        void set_autoindex( bool t ) { autoindex = t; }
        void set_client_max_body_size( const size_t size ) { client_max_body_size = size; }
        void set_cgi_root( const std::string & s ) { cgi_root = s; }
        void set_root( const std::string & s ) { root = s; }
        void set_index( const std::string & s ) { index = split(s, " ", " \t"); }
        void set_cgi_index( const std::string & s ) { cgi_index = split(s, " ", " \t"); }
        void set_accepted_methods( const std::string & s ) { accepted_methods = split(s, " ", " \t"); }
        void set_redirect( const int code, const std::string & location ) { redirect = std::make_pair(code, location); }
        void set_error_page( const int code, const std::string & location ) { error_page = std::make_pair(code, location); }

        std::string get_error_log() const { return error_log; }
        std::string get_access_log() const { return access_log; }
        bool get_sendfile() const { return sendfile; }
        bool get_autoindex() const { return autoindex; }
        size_t get_client_max_body_size() const { return client_max_body_size; }
        std::string get_root() const { return root; }
        std::string get_cgi_root() const { return cgi_root; }
        std::vector<std::string> get_index() const { return index; }
        std::vector<std::string> get_cgi_index() const { return cgi_index; }
        std::vector<std::string> get_accepted_methods() const { return accepted_methods; }
        std::pair<int, std::string> get_redirect() const { return redirect; }
        std::pair<int, std::string> get_error_page() const { return error_page; }

        bool    is_index( const std::string & file ) {
            for (size_t i = 0; i < index.size(); i++)
                if (file == index[i])
                    return true;
            return false;
        }

        bool    is_cgi_index( const std::string & file ) {
            for (size_t i = 0; i < cgi_index.size(); i++) {
                if (file == cgi_index[i])
                    return true;
                size_t pos = cgi_index[i].find("*");
                // std::cout << file << " == " << cgi_index[i] << "\n";
                if (pos != std::string::npos) {
                    std::string sufix = cgi_index[i].substr(pos + 1);
                    // std::cout << sufix << " == " << file.substr(file.size() - sufix.size()) << "\n";
                    if (file.size() > sufix.size() && sufix == file.substr(file.size() - sufix.size()))
                        return true;
                }
            }
            return false;
        }

        bool    is_accepted_method( const std::string & method ) {
            for (size_t i = 0; i < accepted_methods.size(); i++)
                if (method == accepted_methods[i])
                    return true;
            return false;
        }

        void    show_all() {
            std::cout << "error_log = " << error_log << "\n";
            std::cout << "access_log = " << access_log << "\n";
            std::cout << "root = " << root << "\n";
            std::cout << "cgi_root = " << cgi_root << "\n";
            std::cout << "sendfile = " << sendfile << "\n";
            std::cout << "autoindex = " << autoindex << "\n";
            std::cout << "client_max_body_size = " << client_max_body_size << "\n";
            std::cout << "redirect = " << redirect.first << " " << redirect.second << "\n";
            std::cout << "error_page = " << error_page.first << " " << error_page.second << "\n";
            std::cout << "index = ";
            for (size_t i = 0; i < index.size(); i++) { std::cout << index[i] << " "; }
            std::cout << "\n";
            std::cout << "cg_index = ";
            for (size_t i = 0; i < cgi_index.size(); i++) { std::cout << cgi_index[i] << " "; }
            std::cout << "\n";
            std::cout << "accepted_methods = ";
            for (size_t i = 0; i < accepted_methods.size(); i++) { std::cout << accepted_methods[i] << " "; }
            std::cout << "\n";
        }

        Block() {
            root = "/";
            cgi_root = "/";
            client_max_body_size = 0;
            error_log = "logs/error.log";
            access_log = "logs/access.log";
            accepted_methods.push_back("GET");
            accepted_methods.push_back("POST");
            index.clear();
            cgi_index.clear();
            redirect = std::make_pair(0, "");
            error_page = std::make_pair(0, "");
            sendfile = false;
            autoindex = false;
        }
        ~Block() {}
};

#endif