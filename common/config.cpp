#include "config.h"

Config::Config() {
    // default port 8081
    PORT = 8081;
}

void Config::parse_arg(int argc, char *argv[]) {
    int opt = 0;
    const char *str = "p:";
    while((opt = getopt(argc, argv, str)) != -1) {
        switch(opt) {
            case 'p': {
                PORT = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }
}