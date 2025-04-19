#ifndef CONFIG_H
#define CONFIG_H

#include <unistd.h>
#include <cstdlib>

#include "webserver.h"

class Config {
public:
        Config();
        ~Config(){};

        void parse_arg(int argc, char *argv[]);

        int PORT;
};

#endif
