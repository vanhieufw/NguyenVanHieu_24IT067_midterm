#include "new_ls.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

static int is_visible(const char *name, const Settings *cfg) {
    if (cfg->dots == 2) return 1;
    if (cfg->dots == 1) return strcmp(name,".") && strcmp(name,"..");
    return name[0] != '.';
}
static int scan_directory(const char *dirpath, const Settings *cfg, int title) {
    int failed = 0;
    DIR *dir = opendir(dirpath);
    if (!dir) { fprintf(stderr,"new_ls: %s: %s\n",dirpath,strerror(errno));return 1; }
    Catalog contents; catalog_init(&contents);
    errno = 0;
    struct dirent *node;
    while ((node=readdir(dir)) != NULL) {
        if (!is_visible(node->d_name,cfg)) { errno = 0; continue; }
        char *full = path_combine(dirpath,node->d_name);
        if (!full) { perror("new_ls: malloc"); failed=1;break; }
        struct stat st;
        if (lstat(full,&st) != 0) {
            fprintf(stderr,"new_ls: %s: %s\n",full,strerror(errno)); failed=1;
        } else if (catalog_add(&contents,node->d_name,full,&st,S_ISDIR(st.st_mode)) != 0) {
            perror("new_ls: allocation"); failed=1; free(full); break;
        }
        free(full);
        errno=0;
    }
    if (errno) { fprintf(stderr,"new_ls: read %s: %s\n",dirpath,strerror(errno));failed=1; }
    if (closedir(dir)) { perror("new_ls: closedir"); failed=1; }
    if (title) printf("%s:\n",dirpath);
    order_catalog(&contents,cfg);
    show_catalog(&contents,cfg,1);
    if (cfg->recursive) {
        for (size_t i=0; i<contents.length; i++) {
            Item *it = &contents.items[i];
            if (!it->is_dir || !strcmp(it->label,".") || !strcmp(it->label,"..")) continue;
            putchar('\n');
            if (scan_directory(it->path,cfg,1)) failed=1;
        }
    }
    catalog_clear(&contents);
    return failed;
}
int list_targets(int count, char **paths, const Settings *cfg) {
    char *current[] = {"."};
    if (count == 0) { count = 1; paths = current; }
    Catalog files, directories;
    catalog_init(&files); catalog_init(&directories);
    int failed=0;
    for (int i=0;i<count;i++) {
        struct stat st;
        int follow=!cfg->directory;
        if (item_stat(paths[i],&st,follow)) {
            fprintf(stderr,"new_ls: %s: %s\n",paths[i],strerror(errno));failed=1;continue;
        }
        Catalog *into = S_ISDIR(st.st_mode) && !cfg->directory ? &directories : &files;
        if (catalog_add(into,paths[i],paths[i],&st,S_ISDIR(st.st_mode))) {
            perror("new_ls: allocation");failed=1;break;
        }
    }
    order_catalog(&files,cfg);
    order_catalog(&directories,cfg);
    if (files.length) show_catalog(&files,cfg,0);
    int multiple = count > 1;
    for (size_t i=0;i<directories.length;i++) {
        if (i || files.length) putchar('\n');
        if (scan_directory(directories.items[i].path,cfg,multiple || cfg->recursive)) failed=1;
    }
    catalog_clear(&files);
    catalog_clear(&directories);
    return failed;
}
