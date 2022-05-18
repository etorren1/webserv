#include "Server.hpp"

int     main(int argc, char **argv)
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << argv[0] << " port\n";
        return (1);
    }

    Server server(argv[1]);

    server.create();
    server.run();

    return (0);
}