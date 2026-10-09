#include "new_ls.h"
#include <stdio.h>

int main(int argc, char **argv) {
    Settings configuration;
    int first = parse_cli(argc,argv,&configuration);
    if (first < 0) return 2;
    int status = list_targets(argc-first,argv+first,&configuration);
    if (fflush(stdout) == EOF) { perror("new_ls: stdout"); return 1; }
    return status ? 1 : 0;
}
