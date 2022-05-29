#include "Server.hpp"

int     main(int argc, char **argv)
{
    int fd;
    std::string path;
    if (argc > 2) {
        std::cerr << "Usage: " << argv[0] << " (config_file)\n";
        return (1);
    }
    else if (argc == 2) {
        fd = open(argv[1], O_RDONLY);
        path = argv[1];
        if (fd == -1) {
            std::cerr << argv[0] << ": no such file: " << argv[1] << "\n";
            return (1);
        }
    }
    else {
        fd = open(DEFAULT_PATH, O_RDONLY);
        if (fd == -1) {
            std::cerr << "webserv: can not find webserv.conf in the standard path\n"
            << "specify the path to the config file: " << argv[0] << " (path/config_file)" "\n";
            return (1);
        }
    }
    Server server(path);

    server.config(fd);
    server.create();
    server.run();

    return (0);
}