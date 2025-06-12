#include "repl.h"
#include "run.h"

#include <iostream>
#include <print>

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
            std::println(std::cerr, "specify a file to run.\n> "
                                    "tachyon run myfile.tachyon");
            return 1;
        }
        return run(argv[2]);
    }
    else
    {
        std::println(std::cerr, "unknown command: {}", argv[1]);
        return 1;
    }
}
