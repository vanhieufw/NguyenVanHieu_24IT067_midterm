#include "new_ls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <stdint.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/stat.h>
#if defined(__linux__)
#include <sys/sysmacros.h>
#endif
#ifndef S_ISVTX
#define S_ISVTX 01000
#endif
#include <time.h>
#include <ctype.h>
#include <limits.h>

static char kind(mode_t m) {
    if (S_ISDIR(m)) return 'd';
    if (S_ISLNK(m)) return 'l';
    if (S_ISCHR(m)) return 'c';
    if (S_ISBLK(m)) return 'b';
    if (S_ISFIFO(m)) return 'p';
    if (S_ISSOCK(m)) return 's';
    return '-';
}
static void permissions(mode_t m, char out[11]) {
    const mode_t bits[9] = { S_IRUSR,S_IWUSR,S_IXUSR,S_IRGRP,S_IWGRP,S_IXGRP,S_IROTH,S_IWOTH,S_IXOTH };
    const char letters[] = "rwxrwxrwx";
    out[0] = kind(m);
    for (int i=0;i<9;i++) out[i+1] = m & bits[i] ? letters[i] : '-';
    if (m & S_ISUID) out[3] = m & S_IXUSR ? 's' : 'S';
    if (m & S_ISGID) out[6] = m & S_IXGRP ? 's' : 'S';
    if (m & S_ISVTX) out[9] = m & S_IXOTH ? 't' : 'T';
    out[10] = '\0';
}
static uintmax_t used_blocks(const struct stat *st, const Settings *cfg) {
    uintmax_t b = st->st_blocks > 0 ? (uintmax_t)st->st_blocks : 0;
    uintmax_t unit = cfg->kilo ? 1024 : cfg->block_unit;
    if (!unit) unit = 512;
    /* ceil(st_blocks * 512 / unit), without overflow */
    uintmax_t q = b / unit, r = b % unit;
    return q * 512 + (r * 512 + unit - 1) / unit;
}
static void friendly(uintmax_t bytes, char out[40]) {
    static const char *suffix[] = {"B","K","M","G","T","P","E"};
    double val = (double) bytes;
    size_t k = 0;
    while (val >= 1024 && k < 6) { val /= 1024; ++k; }
    if (k == 0) snprintf(out,40,"%ju",bytes);
    else snprintf(out,40,val < 10 ? "%.1f%s" : "%.0f%s",val,suffix[k]);
}
static void printed_name(const char *name, const Settings *cfg) {
    if (cfg->name_mode == NAME_RAW) { fputs(name, stdout); return; }
    for (const unsigned char *p = (const unsigned char *)name; *p; ++p)
        putchar(isprint(*p) ? *p : '?');
}
static void suffix(mode_t m, const Settings *cfg) {
    if (!cfg->classify) return;
    char c = '\0';
    if (S_ISDIR(m)) c = '/';
    else if (S_ISLNK(m)) c = '@';
    else if (S_ISFIFO(m)) c = '|';
    else if (S_ISSOCK(m)) c = '=';
    else if (m & (S_IXUSR|S_IXGRP|S_IXOTH)) c = '*';
    if (c) putchar(c);
}
static void datetime(const Item *item, const Settings *cfg, char text[40]) {
    time_t t = cfg->time_field == TIME_ATIME ? item->meta.st_atime :
        cfg->time_field == TIME_CTIME ? item->meta.st_ctime : item->meta.st_mtime;
    struct tm tmv;
    if (!localtime_r(&t, &tmv)) { strcpy(text, "??? ?? ??:??"); return; }
    time_t now = time(NULL);
    /* NetBSD ls prints year for sufficiently old or future timestamps. */
    const double six_months = 15552000.0;
    const char *format = difftime(now, t) < -3600.0 || difftime(now,t) > six_months ? "%b %e  %Y" : "%b %e %H:%M";
    if (!strftime(text, 40, format, &tmv)) strcpy(text,"??? ?? ??:??");
}
static void owner_group(const Item *it, const Settings *cfg, char who[64], char group[64]) {
    struct passwd *pw = cfg->numeric ? NULL : getpwuid(it->meta.st_uid);
    if (pw) snprintf(who,64,"%s",pw->pw_name);
    else snprintf(who,64,"%ju",(uintmax_t)it->meta.st_uid);
    struct group *gr = cfg->numeric ? NULL : getgrgid(it->meta.st_gid);
    if (gr) snprintf(group,64,"%s",gr->gr_name);
    else snprintf(group,64,"%ju",(uintmax_t)it->meta.st_gid);
}
static void print_row(const Item *it, const Settings *cfg, int ownerw, int groupw, int sizew, int linkw) {
    if (cfg->inode) printf("%ju ",(uintmax_t)it->meta.st_ino);
    if (cfg->blocks) {
        if (cfg->human) {
            char buf[40]; friendly((uintmax_t)(it->meta.st_blocks < 0 ? 0 : it->meta.st_blocks)*512,buf);
            printf("%s ",buf);
        } else printf("%ju ",used_blocks(&it->meta,cfg));
    }
    if (cfg->long_view) {
        char perm[11], when[40], own[64], grp[64], size[80];
        permissions(it->meta.st_mode,perm);
        owner_group(it,cfg,own,grp);
        datetime(it,cfg,when);
        if (S_ISBLK(it->meta.st_mode) || S_ISCHR(it->meta.st_mode))
            snprintf(size,sizeof(size),"%u, %u",(unsigned)major(it->meta.st_rdev),(unsigned)minor(it->meta.st_rdev));
        else if (cfg->human) friendly((uintmax_t)(it->meta.st_size < 0 ? 0 : it->meta.st_size),size);
        else snprintf(size,sizeof(size),"%jd",(intmax_t)it->meta.st_size);
        printf("%s %*ju %-*s %-*s %*s %s ",perm,linkw,(uintmax_t)it->meta.st_nlink,
            ownerw,own,groupw,grp,sizew,size,when);
    }
    printed_name(it->label,cfg);
    suffix(it->meta.st_mode,cfg);
    if (cfg->long_view && S_ISLNK(it->meta.st_mode)) {
        size_t capacity = 128;
        for (;;) {
            char *dest = malloc(capacity);
            if (!dest) break;
            ssize_t n = readlink(it->path,dest,capacity-1);
            if (n < 0) { free(dest); break; }
            if ((size_t)n < capacity-1) {
                dest[n] = 0;
                fputs(" -> ",stdout);
                printed_name(dest,cfg);
                free(dest); break;
            }
            free(dest);
            if (capacity > 1024*1024) break;
            capacity *= 2;
        }
    }
    putchar('\n');
}
void show_one(const Item *it, const Settings *cfg) { print_row(it,cfg,0,0,0,1); }
void show_total(const Catalog *cat, const Settings *cfg) {
    uintmax_t total = 0;
    for (size_t i=0;i<cat->length;i++) {
        uintmax_t b = cat->items[i].meta.st_blocks > 0 ? (uintmax_t)cat->items[i].meta.st_blocks : 0;
        if (UINTMAX_MAX - total < b) { total = UINTMAX_MAX; break; }
        total += b;
    }
    if (cfg->human) {
        char text[40]; friendly(total > UINTMAX_MAX / 512 ? UINTMAX_MAX : total*512,text);
        printf("total %s\n",text);
    } else {
        uintmax_t unit = cfg->kilo ? 1024 : cfg->block_unit;
        if (!unit) unit = 512;
        uintmax_t q = total / unit, r = total % unit;
        printf("total %ju\n",q*512+(r*512+unit-1)/unit);
    }
}
void show_catalog(const Catalog *cat, const Settings *cfg, int directory_contents) {
    if (directory_contents && cfg->long_view) show_total(cat,cfg);
    int ow=0,gw=0,sw=0,lw=1;
    if (cfg->long_view) for (size_t i=0;i<cat->length;i++) {
        const Item *it=&cat->items[i];
        char own[64],grp[64],size[80];
        owner_group(it,cfg,own,grp);
        int a=(int)strlen(own),b=(int)strlen(grp);
        if (a>ow) ow=a;
        if (b>gw) gw=b;
        if (cfg->human) friendly((uintmax_t)(it->meta.st_size<0?0:it->meta.st_size),size);
        else snprintf(size,sizeof(size),"%jd",(intmax_t)it->meta.st_size);
        int c=(int)strlen(size); if(c>sw)sw=c;
        char link[64]; snprintf(link,sizeof(link),"%ju",(uintmax_t)it->meta.st_nlink);
        int d=(int)strlen(link);if(d>lw)lw=d;
    }
    if (cfg->blocks && directory_contents && !cfg->long_view && isatty(STDOUT_FILENO)) show_total(cat,cfg);
    for (size_t i=0;i<cat->length;i++) print_row(&cat->items[i],cfg,ow,gw,sw,lw);
}
