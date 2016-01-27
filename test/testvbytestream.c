#include <assert.h>
#include <stdlib.h>

#include "unittest.h"
#include "diag.h"
#include "mrkcommon/dumpm.h"
#include "mrkcommon/util.h"
#include "mrkcommon/vbytestream.h"

static void
test0(void)
{
    int i;
    vbytestream_t bs;

    vbytestream_init(&bs, 256, 0);

    for (i = 0; i < 40; ++i) {
        if (vbytestream_nprintf(&bs, 64, "%d ", i) < 0) {
            FAIL("vbytestream_nprintf");
        }
    }

    vbytestream_dump(&bs, /* 0 */ VBYTESTREAM_DUMP_FULL);

    vbytestream_write(&bs, 1);
    vbytestream_fini(&bs);


}


int
main(void)
{
    test0();
    return 0;
}

// vim:list
