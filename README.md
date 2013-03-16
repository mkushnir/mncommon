Markiyan's library of "commonly used" functions.


mrkcommon/array.h
=================

Resizable realloc(3)-based arrays of the specified element size.  Elements
are always allocated contiguously in memory.

Automatic initializers (on element allocation) and finalizers (on element
deallocation).

Array resizing: increment (tail grow by one), decrement (tail shrink by
one), arbitrary resize. Arbitrary resize can either preserve the items, or
clean them up and produce a freshly initialized array of the given size.

Array iterators: first, last, next, previous.

Array traversion given a callback.

In-place sorting, a thin wrapper over standard qsort(3).

Limitations: element addresses are not guaranteed to be preserved after
any array resizing. Never refer to element addresses from elsewhere unless
you use allocate-once arrays.


mrkcommon/list.h
================

A limited alternative to sys/queue.h's LIST, although with the completely
different interface.

Provides the same functionality as mrkcommon/array.h plus: element
addresses are guaranteed to be preserved after list resizing. Elements are
never allocated in continuous memory. As in array, provides O(1) access to
a list element by index at the cost of maintaining internal index
structure.  Internal index is implemented as a contiguous array.

Limitations: no insert/delete in the middle of the list, no in-place sort.


mrkcommon/logging.h
===================

A thin wrapper over syslog(3) mostly for debugging purposes. Adaptor over
syslog(3) and user-defined FILE * -based output (for example stderr) which
is very convenient during program development.

A set of macros for convenient logging: TRACE(), DEBUG(), INFO(),
WARNING(), ERROR().

Define file-level scope of logging: LOGGING_DECLARE(), LOGGING_SETLEVEL(),
LOGGING_CLEARLEVEL(). Scoped logging (M for "module"): MTRACE(), MDEBUG(), MINFO(),
MWARNING(), MERROR().


mrkcommon/profile.h
===================

Selective profiling of program code using x86 rdtsc instruction.

Named profiling blocks. Profiling block is a fragment of a linear code
enclosed between a symmetric pair of PROFILE_START()/PROFILE_STOP()
macros.  Each block can be assigned a distinctive name.

Pretty printing of profile statistics: total of calls, min/max/avg
execution time by profile block.

Compile time turning on/off.

Limitations: recursive blocks are not supported.


mrkcommon/conf.h
================

A tokenizer to build parsers from a simple space separated configuration
language into internal structures.

For each keyword, register a callback. Streaming input processing: parse
as you read in. In a callback, conf_parser_tok() / conf_parser_toklen()
/ conf_parser_tokidx() can be used to access current lexical token.


mrkcommon/rbt.h
================

My own implementation of red-black tree. Not as fast as sys/tree.h's RB_*
macros because of probbaly recursive algorithm as opposed to iterative one
in sys/tree.h.


mrkcommon/trie.h
================

My own implementation of trie. Reasonably fast, my quick tests over rbt
show it's a bit faster.


mrkcommon/traversedir.h
=======================

A thin wrapper over directory(3) 4.2BSD API. Just call traverse_dir(path,
cb, udata) to traverse a directory using your own callback.


