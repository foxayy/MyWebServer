#include <iostream>
#include "config.h"

using namespace std;

int main(int argc, char *argv[]) {

    Config config;
    config.parse_arg(argc, argv);

    printf("%d\n", config.PORT);

    return 0;
}