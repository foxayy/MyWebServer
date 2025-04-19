#include <iostream>
#include "config.h"

using namespace std;

int main(int argc, char *argv[]) {

    Config config;
    config.parse_arg(argc, argv);

    WebServer server;

    printf("port:%d\n", config.PORT);

    server.init(config.PORT);
    server.event_listen();
    server.event_loop();

    return 0;
}