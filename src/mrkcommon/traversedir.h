#ifndef MRKCOMMON_TRAVERSEDIR_H
#define MRKCOMMON_TRAVERSEDIR_H

#include <dirent.h>

#ifndef _D_EXACT_NAMLEN
#   define _D_EXACT_NAMLEN(d) ((d)->d_namlen)
#endif

#ifdef __cplusplus
extern "C" {
#endif

int traverse_dir(const char *,
                 int(*)(const char *, struct dirent *, void *),
                 void *);
int traverse_dir_no_recurse(const char *,
                            int(*)(const char *, struct dirent *, void *),
                            void *);
char *path_join(const char *, const char *);

#ifdef __cplusplus
}
#endif

#endif
