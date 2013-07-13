#ifndef MRKCOMMON_TRAVERSEDIR_H
#define MRKCOMMON_TRAVERSEDIR_H

#include <dirent.h>

#ifdef __cplusplus
extern "C" {
#endif

int traverse_dir(const char *,
                 int(*)(const char *, struct dirent *, void *),
                 void *);
char *path_join(const char *, const char *);

#ifdef __cplusplus
}
#endif

#endif
