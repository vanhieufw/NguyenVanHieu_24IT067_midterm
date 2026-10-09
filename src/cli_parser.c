#include "new_ls.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

static unsigned long parse_blocksize(void) {
    const char *env = getenv("BLOCKSIZE");
    if (!env || !*env || *env == '-') return 512;
    errno = 0;
    char *end;
    unsigned long n = strtoul(env, &end, 10);
    if (errno || !n || end == env) return 512;
    unsigned long multiplier = 1;
    if (*end) {
        switch (toupper((unsigned char)*end++)) {
            case 'K': multiplier = 1024UL; break;
            case 'M': multiplier = 1024UL*1024UL; break;
            case 'G': multiplier = 1024UL*1024UL*1024UL; break;
            default: return 512;
        }
        if (*end == 'B') end++;
        if (*end) return 512;
    }
    if (n > ULONG_MAX / multiplier) return 512;
    return n * multiplier;
}

int parse_cli(int argc, char **argv, Settings *cfg) {
    memset(cfg, 0, sizeof(*cfg));
    cfg->sort = SORT_NAME;
    cfg->time_field = TIME_MTIME;
    cfg->name_mode = isatty(STDOUT_FILENO) ? NAME_QUESTION : NAME_RAW;
    cfg->block_unit = parse_blocksize();

    /* Man page: -A is always set for the super-user. */
#ifdef __NetBSD__
    if (geteuid() == 0)
        cfg->dots = 1;
#endif

    int flag;
    opterr = 0;
    while ((flag = getopt(argc, argv, "AacdFfhiklnqRrSstuw")) != -1) {
        switch (flag) {
            case 'A': cfg->dots = 1; break;
            case 'a': cfg->dots = 2; break;
            case 'c': cfg->time_field = TIME_CTIME; break;
            case 'd': cfg->directory = 1; cfg->recursive = 0; break;
            case 'F': cfg->classify = 1; break;
            case 'f': cfg->sort = SORT_NONE; break;
            case 'h': cfg->human = 1; cfg->kilo = 0; break;
            case 'i': cfg->inode = 1; break;
            case 'k': cfg->kilo = 1; cfg->human = 0; break;
            case 'l': cfg->long_view = 1; cfg->numeric = 0; break;
            case 'n': cfg->long_view = 1; cfg->numeric = 1; break;
            case 'q': cfg->name_mode = NAME_QUESTION; break;
            case 'R': cfg->recursive = 1; cfg->directory = 0; break;
            case 'r': cfg->reverse = 1; break;
            case 'S': cfg->sort = SORT_SIZE; break;
            case 's': cfg->blocks = 1; break;
            case 't': cfg->sort = SORT_TIME; break;
            case 'u': cfg->time_field = TIME_ATIME; break;
            case 'w': cfg->name_mode = NAME_RAW; break;
            default:
                fprintf(stderr, "new_ls: invalid option -- '%c'\n", optopt ? optopt : '?');
                fprintf(stderr, "usage: new_ls [-AacdFfhiklnqRrSstuw] [file ...]\n");
                return -1;
        }
    }
    return optind;
}
