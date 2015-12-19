#include <stdio.h>
#include "../error.h"

int main(int argc, char *argv[]) {
    if (argc != 2)
        err_quit("Command parameter error");
    return 0;
}
