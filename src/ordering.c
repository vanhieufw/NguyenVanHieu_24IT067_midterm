#include "new_ls.h"
#include <stdlib.h>
#include <string.h>

static const Settings *active;
static int compare_time(const Item *a, const Item *b) {
    time_t ta, tb;
    long na, nb;
    if (active->time_field == TIME_CTIME) {
        ta = a->meta.st_ctime; tb = b->meta.st_ctime;
#if defined(__NetBSD__) || defined(__linux__)
        na = a->meta.st_ctim.tv_nsec; nb = b->meta.st_ctim.tv_nsec;
#else
        na = nb = 0;
#endif
    } else if (active->time_field == TIME_ATIME) {
        ta = a->meta.st_atime; tb = b->meta.st_atime;
#if defined(__NetBSD__) || defined(__linux__)
        na = a->meta.st_atim.tv_nsec; nb = b->meta.st_atim.tv_nsec;
#else
        na = nb = 0;
#endif
    } else {
        ta = a->meta.st_mtime; tb = b->meta.st_mtime;
#if defined(__NetBSD__) || defined(__linux__)
        na = a->meta.st_mtim.tv_nsec; nb = b->meta.st_mtim.tv_nsec;
#else
        na = nb = 0;
#endif
    }
    if (ta != tb) return ta > tb ? -1 : 1;
    if (na != nb) return na > nb ? -1 : 1;
    return 0;
}
static int compare_items(const void *lhs, const void *rhs) {
    const Item *a = lhs, *b = rhs;
    int result = 0;
    if (active->sort == SORT_SIZE && a->meta.st_size != b->meta.st_size)
        result = a->meta.st_size > b->meta.st_size ? -1 : 1;
    else if (active->sort == SORT_TIME) result = compare_time(a, b);
    if (!result) result = strcmp(a->label, b->label);
    return active->reverse ? -result : result;
}
void order_catalog(Catalog *cat, const Settings *cfg) {
    if (cfg->sort == SORT_NONE || cat->length < 2) return;
    active = cfg;
    qsort(cat->items, cat->length, sizeof(cat->items[0]), compare_items);
}
