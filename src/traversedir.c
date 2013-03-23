#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "diag.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/util.h"
#include "mrkcommon/traversedir.h"

UNUSED static void
dumpdir(struct dirent *de)
{
    TRACE("fileno=%d reclen=%hd type=%hhd namelen=%hhd name=%s", de->d_fileno, de->d_reclen, de->d_type, de->d_namlen, de->d_name);
}

char *
path_join(const char *a, const char *b)
{
    char *res;
    size_t sz1, sz2;
    sz1 = strlen(a);
    sz2 = strlen(b);

    if ((res = malloc(sz1 + 1 + sz2 + 1)) == NULL) {
        FAIL("malloc");
    }

    memcpy(res, a, sz1);
    memcpy(res + sz1, "/", 1);
    memcpy(res + sz1 + 1, b, sz2);
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

    if ((d = opendir(path)) == NULL) {
        return 0;
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
