#ifndef Location_block_hpp
#define Location_block_hpp

#include "InterfaceBlockCfg.hpp"


class Location_block : public Block {
    private:
        Location_block() {}

    public:
    
        Location_block( Block & other ) {
            error_log = other.get_error_log();
            error_page = other.get_error_page();
            access_log = other.get_access_log();
            root = other.get_root();
            index = other.get_index();
            sendfile = other.get_sendfile();
            autoindex = other.get_autoindex();
            client_max_body_size = other.get_client_max_body_size();
            default_page = other.get_default_page();
            accepted_methods = other.get_accepted_methods();
        }
        ~Location_block() {}
};

#endif