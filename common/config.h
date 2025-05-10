#ifndef CONFIG_H
#define CONFIG_H

#include <unistd.h>
#include <cstdlib>

class Config {
public:
        Config();
        ~Config(){};

        void parse_arg(int argc, char *argv[]);

        int PORT;
        
        int actor_model;
};

#endif
