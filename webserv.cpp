#include "Server.hpp"

int     main(int argc, char **argv)
{
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " (config_file)\n";
        return (1);
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd == -1) {
        std::cerr << argv[0] << ": no such file: " << argv[1] << "\n";
        return (1);
    }
    Server server(fd);

    server.create();
    server.run();

    return (0);
}