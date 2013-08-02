Markiyan's library of "commonly used" functions. The primary platform is
FreeBSD. Default compiler: clang (please see _configure.ac_).


TODO
====

* Linux;

* ...


mrkcommon/array.h
=================

Resizable _realloc(3)_-based arrays of the specified element size.
Elements are always allocated contiguously in memory.

Automatic initializers (on element allocation) and finalizers (on element
deallocation).

Array re-sizing: increment (tail grow by one), decrement (tail shrink by
one), arbitrary re-size. Arbitrary re-size can either preserve the items, or
clean them up and produce a freshly initialized array of the given size.

Array iterators: first, last, next, previous.

Array traversing given a callback.

In-place sorting, a thin wrapper over standard _qsort(3)_.

Limitations: element addresses are not guaranteed to be preserved after
any array re-sizing. Never refer to element addresses from elsewhere unless
you use allocate-once arrays.


mrkcommon/list.h
================

A limited alternative to _sys/queue.h_'s _LIST_, although with the
completely different interface.

Provides the same functionality as _mrkcommon/array.h_ plus: element
addresses are guaranteed to be preserved after list re-sizing. Elements are
never allocated in continuous memory. As in array, provides O(1) access to
a list element by index at the cost of maintaining internal index
structure.  Internal index is implemented as a contiguous array.

Limitations: no insert/delete in the middle of the list, no in-place sort.


mrkcommon/logging.h
===================

A thin wrapper over _syslog(3)_ mostly for debugging purposes. Adapter
over _syslog(3)_ and user-defined _FILE *_ -based output (for example
_stderr_) which is very convenient during program development.

A set of macros for convenient logging: _TRACE()_, _DEBUG()_, _INFO()_,
_WARNING()_, _ERROR()_.

Define file-level scope of logging: _LOGGING\_DECLARE()_,
_LOGGING\_SETLEVEL()_, _LOGGING\_CLEARLEVEL()_. Scoped logging \(_M_ for
"module"): _MTRACE()_, _MDEBUG()_, _MINFO()_, _MWARNING()_, _MERROR()_.


mrkcommon/profile.h
===================

Selective profiling of program code using x86 _rdtsc_ instruction.

Named profiling blocks. Profiling block is a linear fragment of code
enclosed between a symmetric pair of _PROFILE\_START()_/_PROFILE\_STOP()_
macros.  Each block can be assigned a distinctive name.

Pretty printing of profile statistics: total of calls, min/max/avg
execution time by profile block.

Compile time turning on/off.

Limitations: recursive blocks are not supported.


mrkcommon/conf.h
================

A tokenizer to build parsers from a simple space-separated configuration
language into internal structures.

For each keyword, register a callback. Streaming input processing: parse
as you read in. In a callback, _conf\_parser\_tok()_
/ _conf\_parser\_toklen()_ / _conf\_parser\_tokidx()_ can be used to
access current lexical token.


mrkcommon/rbt.h
================

My own implementation of red-black tree. Not as fast as _sys/tree.h_'s
_RB\_\*_ macros because of probably recursive algorithm as opposed to
iterative one in _sys/tree.h_.


mrkcommon/trie.h
================

My own implementation of trie. Reasonably fast, my quick tests over rbt
show it's a bit faster.


mrkcommon/traversedir.h
=======================

A thin wrapper over _directory(3)_ 4.2BSD API. Just call
_traverse\_dir(path, cb, udata)_ to traverse a directory using your own
callback.


mrkcommon/bytestream.h
=======================
Combine standard POSIX _read(2)_/_write(2))_ and related system calls
(_send(2)_, _recv(2)_, etc) with memory management features: automatically
extend via _realloc(3)_ when placing data into internal buffer.
Conveniently access memory within the buffer, track current position and
end of data that have been read up to now. When needed, reuse the buffer
before the next read/write. One important thing that has to be remembered:
memory locations inside the internal buffer are never guaranteed to be
preserved across read()'s and write()'s due to automatic memory management
inside the bytestream. Typical patterns of use are suitable for streaming
processors or parsers:

    static bytestream_t in, out;

    void
    init(void)
    {
        if (bytestream_init(&in) != 0) {
            FAIL("bytestream_init");
        }
        if (bytestream_init(&out) != 0) {
            FAIL("bytestream_init");
        }
        in.read_more = bytestream_recv_more;
        in.write = bytestream_send;
    }

    void
    fini(void)
    {
        bytestream_fini(&in);
        bytestream_fini(&out);
    }

    void write_test_request(int fd)
    {
        bytestream_cat(&out, "GET / HTTP/1.0\r\n", strlen("GET / HTTP/1.0\r\n"));
        bytestream_cat(&out, "Host: example.com\r\n", strlen("Host: example.com\r\n"));
        bytestream_cat(&out, "\r\n", strlen("\r\n"));
        bytestream_produce_data(&out, fd);
        bytestream_rewind(&out);
    }

    void read_test_response(int fd)
    {
        int done = 0;

        while (!done) {
            if (SNEEDMORE(&in)) {
                bytestream_consume_data(&in, fd);
            }
            /*
             *  -   use SPDATA(&in) to access current data of your interest,
             *      starting from zero offset.
             *  -   use SEDATA(&in) to get the pointer right after the
             *      last byte of the available data.
             *  -   use SAVAIL(&in) to get the size of data you can
             *      process in this cycle.
             *  -   use SADVANCEPOS(&in, ...) to advance current data
             *      pointer as you want. Don't try to advance current data
             *      pointer more than by currently SAVAIL(&in) bytes.
             *      Always do SADVANCEPOS() within the loop to
             *      indicate to bytestream_consume_data() how much data
             *      you have processed.
             *  -   read mrkcommon/bytestream.h for more macros.
             */
            if (... /* done? */ ) {
                done = 1;
            } else {
                /* state machine to parse a piece of response */
            }
        }
        bytestream_recycle(&in, SPOS(&in));
    }


mrkcommon/json.h
================
A fast callback-style JSON parser. Unlike traditional JSON parsers,
doesn't build objects in memory. Individual callbacks can be registered
for the following events: object start, object end, object key, object
value, array start, array end, array item. Supported simple data types:
string, signed integer, float, boolean, as well as special value _null_.


mrkcommon/stqueue.h
===================
Singly-linked tail queue with O(1) insertions at queue's tail (_enqueue_),
O(1) "insertions after" the given element in the middle, and O(1)
deletions from head (_dequeue_). "Insertions before" and removals of any
given element to/from the middle are O(n). This is a simpler and more
limited implementation of singly-linked tail queue comparing to STAILQ from
\<sys/queue.h\>. It can be best used when you only need enqueue/dequeue
operations.

mrkcommon/dtqueue.h
===================
Doubly-linked tail queue with O(1) additions at queue's tail, _enqueue_,
O(1) removals from head, _dequeue_, as well as O(1) additions before/after
and removals of any given element to/from the middle. This is a simpler and more
limited implementation of doubly-linked tail queue comparing to TAILQ from
\<sys/queue.h\>. It can be best used when you only need enqueue operations
combined with either dequeue or removals from the middle.

mrkcommon/memdebug.h
====================
A thin macro-based layer over _memory(3)_ and _str\{n\}dup(3)_ that allows
to track currently allocated memory. Limitation: doesn't track memory
allocated by those compilation units that didn't include
_"mrkcommon/memdebug.h"_.

mrkcommon/dumpm.h
=================

Miscellaneous debugging utilities:

* logging macros (not from _mrkcommon/logging.h_): _TRACE()_ & co;

* hexadecimal dump of a memory region: _D8()_, _D16()_, _D32()_,
  _D64()_;

* debugging wrappers over the "return" statement: _TRRET()_,
  _TRRETNULL()_;

* colored text formatting for the tty output; _F<...>()_;


mrkcommon/util.h
================

Miscellaneous macros.


