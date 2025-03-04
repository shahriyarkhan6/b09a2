#include "fdtable.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int parse_arguments(int argc, char* argv[], int flag) {
    argv++;
    argc--;

    while (argc > 0) {
        if (argv[0][0] == '-' && argv[0][1] == '-') {
            char* option = argv[0] + 2;

            if (strcmp(option, "per-process") == 0) {
                return 1;
            } 
            else if (strcmp(option, "systemWide") == 0) {
                return 2;
            }
            else if (strcmp(option, "Vnodes") == 0) {
                return 3;
            }
            else if (strcmp(option, "composite") == 0) {
                return 4;
            }
            else if (strcmp(option, "summary") == 0) {
                return 5;
            }
            else {
                fprintf(stderr, "Unknown option: %s\n", argv[0]);
                fprintf(stderr, "Usage: %s [--per-process] [--systemWide] [--Vnodes] "
                        "[--composite] [--summary] [PID]\n", argv[0]);
                exit(1);
            }
        }
        else {
            int i;
            int valid = 1;
            for (i = 0; argv[0][i] != '\0'; i++) {
                if (argv[0][i] < '0' || argv[0][i] > '9') {
                    valid = 0;
                    break;
                }
            }
            if (!valid) {
                fprintf(stderr, "Invalid PID value\n");
                exit(1);
            }
            return atoi(argv[0]);
        }

        argv++;
        argc--;
    }

    return 4;  // Default to composite if no arguments
} 