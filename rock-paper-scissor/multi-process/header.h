#include <stdio.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>

enum {
    PAPER = 1,
    SCISOR = 2,
    ROCK = 3
};
