#include "config.h"

Config::Config() {
    // default port 8081
    PORT = 8081;

    // Concurrency model, default reactor
    actor_model = 1;
}

void Config::parse_arg(int argc, char *argv[]) {
    int opt = 0;
    const char *str = "p:a:";
    while((opt = getopt(argc, argv, str)) != -1) {
        switch(opt) {
            case 'p': {
                PORT = atoi(optarg);
                break;
            }
            case 'a': {
                actor_model = atoi(optarg);
                break;
            }
            default:
                break;
        }
    }
}