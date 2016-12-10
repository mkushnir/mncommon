#include <assert.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "unittest.h"
#include "diag.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/util.h"
#include "mrkcommon/array.h"
#include "mrkcommon/traversedir.h"

typedef struct _file_info {
    long rnd;
    char *path;
    size_t sz;
    unsigned int flags;
} file_info_t;

typedef struct _file_info2 {
    long rnd;
    file_info_t *fi;
} file_info2_t;

static mnarray_t files;

static int
mycb(const char *path, struct dirent *de, void *ctx)
{
    mnarray_t *files = (mnarray_t *)ctx;
    file_info_t *fi;
    UNUSED struct stat sb;

    if (de != NULL) {
        //TRACE("path=%s fileno=%d reclen=%hd type=%hhd namelen=%hhd name=%s",
        //       path, de->d_fileno, de->d_reclen, de->d_type, de->d_namlen,
        //       de->d_name);

        if ((fi = array_incr(files)) == NULL) {
            return 1;
        }
        if ((fi->path = path_join(path, de->d_name)) == NULL) {
            return 1;
        }
        fi->sz = strlen(fi->path);
        fi->flags = 0;
    }
    //if (stat(fi->path, &sb) == 0) {
    //    //TRACE("S_ISDIR=%d", S_ISDIR(sb.st_mode));
    //}
    return 0;
}

static int
file_info_fini(file_info_t *fi)
{
    if (fi->path != NULL) {
        free(fi->path);
        fi->path = NULL;
    }
    fi->flags = 0;
    return 0;
}

UNUSED static int
file_info_print(file_info_t *fi)
{
    TRACE("%s", fi->path);
    return 0;
}

UNUSED static void
test0(void)
{
    struct {
        long rnd;
        const char *path;
    } data[] = {
        //{0, "/usr"},
        //{0, "/usr/local"},
        //{0, "/usr/local/bin/"},
        //{0, "/usr/local/lib"},
        //{0, "/home/mkushnir/music"},
        //{0, "/usr/data"},
        //{0, "/usr/data/amule-temp"},
        //{0, "/skyrta/data/music/bach"},
        //{0, "/skyrta/data/music/bruckner"},
        //{0, "/skyrta/data/music/beethowen"},
        //{0, "/skyrta/data/music/wagner"},
        {0, "."},
        {0, "./"},
    };
    UNITTEST_PROLOG;

    array_init(&files, sizeof(file_info_t), 0,
               NULL, (array_finalizer_t)file_info_fini);
    FOREACHDATA {
        if (traverse_dir(CDATA.path, mycb, &files) != 0) {
            //perror("opendir");
            //return;
            //assert(0);
            //break;
        }
        array_traverse(&files, (array_traverser_t)file_info_print, NULL);
    }

    array_fini(&files);
}


int
main(void)
{
    test0();
    return 0;
}

// vim:list
