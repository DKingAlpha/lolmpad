#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/input.h>
#include <errno.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>

typedef struct
{
    int x;
    int y;
} ScreenPos;
