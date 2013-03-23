#ifndef TRAVERSEDIR_H
#define TRAVERSEDIR_H

#include <dirent.h>

int traverse_dir(const char *,
                 int(*)(const char *, struct dirent *, void *),
                 void *);
char *path_join(const char *, const char *);

#endif
