#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <getopt.h>
#include "tt.h"

int main(int argc, char **argv) {
    if (argc < 1) {
        fprintf(stderr, "tt: usage: tt <command> [options]");
        exit(1);
    }

    int opt, opt_index, counter = 0;

    tt_init();

    while (true) {
        static struct option options[] = {
                {"set-maximized", required_argument, NULL, 'm'},
                {"maximize", no_argument, NULL, 'M'},
                {"set-decorate", required_argument, NULL, 'd'},
                {"decorate", no_argument, NULL, 'D'},
                {"current", no_argument, NULL, 'C'},
                {"window", required_argument, NULL, 'w'},
                {0, 0, 0, 0}
        };

        opt_index = 0;

        opt = getopt_long(argc, argv, "CDMd:m:w:", options, &opt_index);
        if (opt == -1) {
            break;
        }

        char *arg;
        if (optarg) {
            arg = optarg;
        } else {
            arg = NULL;
        }

        switch (opt) {
            case 'C':
                TT_FLAGS |= TT_DEBUG_CWIN;
                break;
            case 'm':
                TT_FLAGS |= atoi(arg) ? TT_ACT_MAX : TT_ACT_UMAX;
                break;
            case 'M':
                TT_FLAGS |= TT_ACT_TMAX;
            case 'w':
                TT_FLAGS |= TT_SPEC_WIN;
            default:
                exit(1);
        }
        counter++;
    }
    if (BITSET_NC(TT_FLAGS, TT_SPEC_WIN)) {
        TT_FLAGS |= TT_USE_CWIN;
        sel_window = active_window(display);
    }

    if (BITSET_C(TT_FLAGS, TT_DEBUG_CWIN)) {
        printf("Selected Window: %#9lx\n", sel_window);
    }
    if (BITSET_C(TT_FLAGS, TT_ACT_MAX) || BITSET_C(TT_FLAGS, TT_ACT_UMAX)) {
        set_maximized(display, sel_window, (BITSET_C(TT_FLAGS, TT_ACT_MAX)) ? true : false);
    }
}