#ifndef Location_block_hpp
#define Location_block_hpp

#include "InterfaceBlockCfg.hpp"


class Location_block : public Block {
    private:
        std::string     location;

        Location_block() {}

    public:
    
        void    set_location( const std::string & s ) { location = s; }

        std::string get_location( void ) const { return location; }

        void    show_all() {
            std::cout << "location = " << location << "\n";
            Block::show_all();
        }

        Location_block( Block & other ) {
            error_log = other.get_error_log();
            error_page = other.get_error_page();
            access_log = other.get_access_log();
            root = other.get_root();
            index = other.get_index();
            cgi_index = other.get_cgi_index();
            sendfile = other.get_sendfile();
            autoindex = other.get_autoindex();
            client_max_body_size = other.get_client_max_body_size();
            accepted_methods = other.get_accepted_methods();
        }
        ~Location_block() {}
};

#endif