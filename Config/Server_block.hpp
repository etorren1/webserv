#ifndef Server_block_hpp
#define Server_block_hpp

#include "InterfaceBlockCfg.hpp"
#include "Location_block.hpp"
#include <map>

class Server_block : private Block {
    private:
        std::string listen;
        std::vector<std::string>     server_name;

        Server_block() {}
        
    public:
        std::map<std::string, Location_block*> lctn;
    
        void    set_listen( const std::string & s ) { listen = s; }
        void    set_server_name( const std::string & s ) { server_name = split(s, " "); }

        std::string    get_listen() { return listen; }
        std::vector<std::string> get_server_name() { return server_name; }

        Server_block( const Block & other ) {
            error_log = other.get_error_log();
            error_page = other.get_error_page();
            access_log = other.get_access_log();
            root = other.get_root();
            index = other.get_index();
            sendfile = other.get_sendfile();
            autoindex = other.get_autoindex();
            client_max_body_size = other.get_client_max_body_size();
        }
        ~Server_block() {}
};

#endif