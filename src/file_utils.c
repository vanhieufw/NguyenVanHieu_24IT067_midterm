#include "new_ls.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>

void catalog_init(Catalog *cat) { cat->items = NULL; cat->length = cat->capacity = 0; }
void catalog_clear(Catalog *cat) {
    for (size_t i = 0; i < cat->length; ++i) {
        free(cat->items[i].label);
        free(cat->items[i].path);
    }
    free(cat->items);
    catalog_init(cat);
}
int catalog_add(Catalog *cat, const char *name, const char *path, const struct stat *st, int is_dir) {
    if (cat->length == cat->capacity) {
        if (cat->capacity > (SIZE_MAX / 2) / sizeof(Item)) { errno = ENOMEM; return -1; }
        size_t next = cat->capacity ? cat->capacity * 2 : 16;
        Item *ptr = realloc(cat->items, next * sizeof(*ptr));
        if (!ptr) return -1;
        cat->items = ptr;
        cat->capacity = next;
    }
    char *display = strdup(name), *full = strdup(path);
    if (!display || !full) { free(display); free(full); return -1; }
    Item *entry = &cat->items[cat->length++];
    entry->label = display;
    entry->path = full;
    entry->meta = *st;
    entry->is_dir = is_dir;
    entry->stat_ok = 1;
    return 0;
}
char *path_combine(const char *base, const char *leaf) {
    size_t a = strlen(base), b = strlen(leaf);
    if (a > SIZE_MAX - b - 2) { errno = ENOMEM; return NULL; }
    char *path = malloc(a + b + 2);
    if (!path) return NULL;
    memcpy(path, base, a);
    size_t pos = a;
    if (pos && path[pos - 1] != '/') path[pos++] = '/';
    memcpy(path + pos, leaf, b + 1);
    return path;
}
int item_stat(const char *path, struct stat *st, int follow_directory_link) {
    if (lstat(path, st) != 0) return -1;
    if (follow_directory_link && S_ISLNK(st->st_mode)) {
        struct stat target;
        if (stat(path, &target) == 0 && S_ISDIR(target.st_mode)) *st = target;
    }
    return 0;
}
