#include <assert.h>
#include <stdlib.h>
#include <time.h>

#include "unittest.h"
#include "diag.h"

#include "mrkcommon/dumpm.h"
#include "mrkcommon/logging.h"
#include "mrkcommon/conf.h"

LOGGING_DECLARE(NULL, 0, LOG_TRACE);

/*
 * foo param1 [param2 [param3]]
 * bar param1 param2
 * baz [param1 [param2]]
 */
static int
my_handler(conf_parser_ctx_t *ctx, UNUSED void *udata)
{
    int idx = conf_parser_tokidx(ctx);
    const char *tok = conf_parser_tok(ctx);

    switch (idx) {
    case 0:
        TRACE("%s:", tok);
        break;

    case 1:
    case 2:
    case 3:
        TRACE(" ... param %d is %s", idx, tok);
        break;

    default:
        /* unreach */
        assert(0);

    }
    return CONF_HANDLER_CONT;
}

static int
my_error_handler(conf_parser_ctx_t *ctx, UNUSED void *udata)
{
    int idx = conf_parser_tokidx(ctx);
    const char *tok = conf_parser_tok(ctx);
    const char *otok = conf_parser_orig_tok(ctx);
    FILE *f = conf_parser_file(ctx);
    int flags = conf_parser_flags(ctx);

    TRACE("tok=%s otok=%s idx=%d flags=%08x pos=%ld", tok, otok, idx, flags, ftell(f));
    return CONF_HANDLER_INTR;
}

static void
test0(void)
{
    int res;
    struct {
        long rnd;
        int in;
        int expected;
    } data[] = {
        {0, 0, 0},
    };
    UNITTEST_PROLOG_RAND;

    FOREACHDATA {
        TRACE("in=%d expected=%d", CDATA.in, CDATA.expected);
        assert(CDATA.in == CDATA.expected);
    }
    conf_register_prefix_handler("#", 0, 1, line_comment_handler, my_error_handler, NULL);
    conf_register_word_handler("foo", 1, 2, my_handler, my_error_handler, NULL);
    conf_register_word_handler("bar", 2, 0, my_handler, my_error_handler, NULL);
    conf_register_word_handler("baz", 0, 2, my_handler, my_error_handler, NULL);
    res = conf_parse_buf("# This is a comment\n"
                   "foo qwe asd zxc\n"
                   "foo qwe asd\n"
                   "foo qwe\n"
                   "foo qwe 12345678901234567\n"
                   "foo\n"
                   "bar rty fgh\n"
                   "bar rty\n"
                   "bar\n"
                   "baz uio jkl\n"
                   "baz uio\n"
                   "baz\n"
                  );
    //TRACE("res=%s", diag_str(res));
    res = conf_parse_buf("# This is a comment\n"
                   "foo qwe asd zxc "
                   "foo qwe asd "
                   "foo qwe "
                   "foo "
                   "bar rty fgh "
                   "bar rty "
                   "bar "
                   "baz uio jkl "
                   "baz uio "
                   "baz "
                  );
    //TRACE("res=%s", diag_str(res));
}

int
main(void)
{
    //LOGGING_CLEARLEVEL(LOG_TRACE);
    logging_init(stdout, "testconf", LOG_PID);
    conf_init();
    test0();
    conf_fini();
    logging_fini();
    return 0;
}


