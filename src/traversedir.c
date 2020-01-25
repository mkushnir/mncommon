#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "diag.h"
#include "mncommon/malloc.h"
#include "mncommon/dumpm.h"
#include "mncommon/util.h"
#include "mncommon/traversedir.h"

UNUSED static void
dumpdir(struct dirent *de)
{
#if defined(_BITS_TYPESIZES_H) && defined(__INO_T_TYPE)
    TRACE("fileno=%ld reclen=%hd type=%d namelen=%ld name=%s",
          (long)de->d_fileno,
          de->d_reclen,
          de->d_type,
          _D_EXACT_NAMLEN(de),
          de->d_name);
#else
    TRACE("fileno=%ld reclen=%hd type=%hhd namelen=%d name=%s",
          (long)de->d_fileno,
          de->d_reclen,
          de->d_type,
          (int)_D_EXACT_NAMLEN(de),
          de->d_name);
#endif
}

char *
path_join(const char *a, const char *b)
{
    char *res;
    size_t sz1, sz2;
    sz1 = (a != NULL) ? strlen(a) : 0;
    sz2 = (b != NULL) ? strlen(b) : 0;

    if ((res = malloc(sz1 + 1 + sz2 + 1)) == NULL) {
        FAIL("malloc");
    }

    if (a != NULL) {
        memcpy(res, a, sz1);
        *(res + sz1) = '\0';
    }

    if (*(res + sz1 - 1) != '/') {
        *(res + sz1) = '/';
    } else {
        --sz1;
    }
    if (sz2 > 0) {
        if (b != NULL) {
            if (*b == '/') {
                ++b;
                --sz2;
            }
            memcpy(res + sz1 + 1, b, sz2);
        }
    }

    *(res + sz1 + 1 + sz2) = '\0';

    return res;
}


int
traverse_dir(const char *path,
             int(*cb)(const char *, struct dirent *, void *),
             void *udata)
{
    int res = 0;
    DIR *d;
    struct dirent *de;

    //TRACE("traversing %s", path);

    if ((d = opendir(path)) == NULL) {
        return 1;
    }

    while ((de = readdir(d)) != NULL) {
        char *newpath;

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }


        if (de->d_type == DT_DIR) {
            newpath = path_join(path, de->d_name);
            res = traverse_dir(newpath, cb, udata);
            free(newpath);
            if (res != 0) {
                break;
            }
        } else {
            struct stat sb;

            newpath = path_join(path, de->d_name);

            if (lstat(newpath, &sb) == 0) {
                if (S_ISDIR(sb.st_mode)) {
                    if ((res = traverse_dir(newpath, cb, udata)) != 0) {
                        free(newpath);
                        break;
                    }
                } else {
                    if ((res = cb(path, de, udata)) != 0) {
                        free(newpath);
                        closedir(d);
                        return res;
                    }
                }
            } else {
                free(newpath);
                break;
            }
            free(newpath);
            newpath = NULL;
        }
    }

    closedir(d);
    return cb(path, NULL, udata);
}


int
traverse_dir_no_recurse(const char *path,
                        int(*cb)(const char *, struct dirent *, void *),
                        void *udata)
{
    int res = 0;
    DIR *d;
    struct dirent *de;

    //TRACE("traversing %s", path);

    if ((d = opendir(path)) == NULL) {
        return 1;
    }

    while ((de = readdir(d)) != NULL) {
        char *newpath;

        if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0) {
            continue;
        }

        if (de->d_type == DT_DIR) {
            newpath = path_join(path, de->d_name);

            if ((res = cb(newpath, de, udata)) != 0) {
                free(newpath);
                closedir(d);
                return res;
            }

            free(newpath);
            newpath = NULL;
            continue;

        } else {
            struct stat sb;

            newpath = path_join(path, de->d_name);

            if (lstat(newpath, &sb) == 0) {
                if (S_ISDIR(sb.st_mode)) {
                    free(newpath);
                    newpath = NULL;
                    continue;
                } else {
                    if ((res = cb(newpath, de, udata)) != 0) {
                        free(newpath);
                        closedir(d);
                        return res;
                    }
                }
            } else {
                free(newpath);
                newpath = NULL;
                continue;
            }
            free(newpath);
            newpath = NULL;
        }
    }

    closedir(d);
    return cb(path, NULL, udata);
}
