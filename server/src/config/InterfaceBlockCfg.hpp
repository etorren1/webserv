#ifndef InterfaceBlockCfg_hpp
#define InterfaceBlockCfg_hpp

#include "../Utils.hpp"

class Block
{
    public:

        std::string     error_log;
        std::string     error_page;
        std::string     access_log;
        std::string     default_page;
        std::string     root;
        std::vector<std::string>     accepted_methods;
        std::vector<std::string>     index;
        bool            sendfile;
        bool            autoindex;
        size_t          client_max_body_size;

        void set_error_log( const std::string & s ) { error_log = s; }
        void set_error_page( const std::string & s ) { error_page = s; }
        void set_access_log( const std::string & s ) { access_log = s; }
        void set_sendfile( bool t ) { sendfile = t; }
        void set_autoindex( bool t ) { autoindex = t; }
        void set_client_max_body_size( const size_t size ) { client_max_body_size = size; }
        void set_root( const std::string & s ) { root = s; }
        void set_default_page( const std::string & s ) { default_page = s; }
        void set_index( const std::string & s ) { index = split(s, " "); }
        void set_accepted_methods( const std::string & s ) { accepted_methods = split(s, " "); }

        std::string get_error_log() const { return error_log; }
        std::string get_error_page() const { return error_page; }
        std::string get_access_log() const { return access_log; }
        bool get_sendfile() const { return sendfile; }
        bool get_autoindex() const { return autoindex; }
        size_t get_client_max_body_size() const { return client_max_body_size; }
        std::string get_root() const { return root; }
        std::string get_default_page() const { return default_page; }
        std::vector<std::string> get_index() const { return index; }
        std::vector<std::string> get_accepted_methods() const { return accepted_methods; }

        bool    is_index( const std::string & type ) {
            for (size_t i = 0; i < index.size(); i++) {
                size_t pos = index[i].find(type);
                if (pos != std::string::npos && index[i].size() - pos == type.size())
                    return true;
            }
            return false;
        }

        bool    is_accepted_methods( const std::string & type ) {
            for (size_t i = 0; i < accepted_methods.size(); i++) {
                size_t pos = accepted_methods[i].find(type);
                if (pos != std::string::npos && accepted_methods[i].size() - pos == type.size())
                    return true;
            }
            return false;
        }

        void    show_all() {
            std::cout << "error_log = " << error_log << "\n";
            std::cout << "error_page = " << error_page << "\n";
            std::cout << "access_log = " << access_log << "\n";
            std::cout << "root = " << root << "\n";
            std::cout << "sendfile = " << sendfile << "\n";
            std::cout << "autoindex = " << autoindex << "\n";
            std::cout << "client_max_body_size = " << client_max_body_size << "\n";
            std::cout << "default_page = " << default_page << "\n";
            std::cout << "index = ";
            for (size_t i = 0; i < index.size(); i++) { std::cout << index[i] << " "; }
            std::cout << "\n";
            std::cout << "accepted_methods = ";
            for (size_t i = 0; i < accepted_methods.size(); i++) { std::cout << accepted_methods[i] << " "; }
            std::cout << "\n";
        }

        Block() {
            client_max_body_size = 1024;
            error_log = "logs/error.log";
            access_log = "logs/access.log";
            root = "site/";
            sendfile = false;
            autoindex = false;
        }
        ~Block() {}
};

#endif