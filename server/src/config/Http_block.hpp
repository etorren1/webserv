#ifndef Http_block_hpp
#define Http_block_hpp

#include "InterfaceBlockCfg.hpp"

class Http_block : public Block {
    private:
        int             connection_timeout;

    public:
        void set_connection_timeout( const int time ) { connection_timeout = time; }

        int get_connection_timeout() const { return connection_timeout; }


        void    show_all() {
            std::cout << "connection_timeout" << connection_timeout << "\n";
            Block::show_all();
        }

        Http_block() {
            connection_timeout = 0;
        }
        ~Http_block() {}
};

#endif