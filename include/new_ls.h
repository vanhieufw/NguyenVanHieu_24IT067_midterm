#ifndef NEW_LS_H
#define NEW_LS_H

#define _POSIX_C_SOURCE 200809L
#ifdef __NetBSD__
#define _NETBSD_SOURCE 1
#endif
#include <sys/types.h>
#include <sys/stat.h>
#include <stddef.h>
#include <time.h>

typedef enum { SORT_NAME, SORT_SIZE, SORT_TIME, SORT_NONE } SortMode;
typedef enum { TIME_MTIME, TIME_CTIME, TIME_ATIME } TimeMode;
typedef enum { NAME_RAW, NAME_QUESTION } NameMode;
typedef struct {
    int dots;          /* 0: none; 1: omit . and ..; 2: all */
    int long_view;     /* -l or -n */
    int numeric;       /* -n */
    int directory;     /* -d overrides -R */
    int recursive;
    int classify;
    int inode;
    int blocks;
    int human;
    int kilo;
    int reverse;
    SortMode sort;
    TimeMode time_field;
    NameMode name_mode;
    unsigned long block_unit;
} Settings;

typedef struct {
    char *label;
    char *path;
    struct stat meta;
    int is_dir;
    int stat_ok;
} Item;

typedef struct {
    Item *items;
    size_t length;
    size_t capacity;
} Catalog;

int parse_cli(int argc, char **argv, Settings *cfg);
void catalog_init(Catalog *cat);
void catalog_clear(Catalog *cat);
int catalog_add(Catalog *cat, const char *name, const char *path, const struct stat *st, int is_dir);
char *path_combine(const char *base, const char *leaf);
int item_stat(const char *path, struct stat *st, int follow_directory_link);
void order_catalog(Catalog *cat, const Settings *cfg);
int list_targets(int count, char **paths, const Settings *cfg);
void show_catalog(const Catalog *cat, const Settings *cfg, int directory_contents);
void show_one(const Item *item, const Settings *cfg);
void show_total(const Catalog *cat, const Settings *cfg);

#endif
