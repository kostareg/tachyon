#include "repl.hpp"
#include "run.hpp"

#include <iostream>
#include <cstring>

int main(int argc, char **argv)
{
    if (argc == 1)
    {
        return repl();
    }
    else if (strcmp(argv[1], "run") == 0)
    {
        if (argc == 2)
        {
            std::cerr << "specify a file to run.\n> tachyon run myfile.tachyon" << std::endl;
            return 1;
        }
        return run(argv[2]);
    }
    else
    {
        std::cerr << "unknown command: " << argv[1] << std::endl;
        return 1;
    }
}
