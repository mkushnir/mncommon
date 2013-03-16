Markiyan's library of "commonly used" functions.


mrkcommon/array.h
=================

Resizable _realloc(3)_-based arrays of the specified element size.
Elements are always allocated contiguously in memory.

Automatic initializers (on element allocation) and finalizers (on element
deallocation).

Array resizing: increment (tail grow by one), decrement (tail shrink by
one), arbitrary resize. Arbitrary resize can either preserve the items, or
clean them up and produce a freshly initialized array of the given size.

Array iterators: first, last, next, previous.

Array traversion given a callback.

In-place sorting, a thin wrapper over standard _qsort(3)_.

Limitations: element addresses are not guaranteed to be preserved after
any array resizing. Never refer to element addresses from elsewhere unless
you use allocate-once arrays.


mrkcommon/list.h
================

A limited alternative to _sys/queue.h_'s _LIST_, although with the
completely different interface.

Provides the same functionality as _mrkcommon/array.h_ plus: element
addresses are guaranteed to be preserved after list resizing. Elements are
never allocated in continuous memory. As in array, provides O(1) access to
a list element by index at the cost of maintaining internal index
structure.  Internal index is implemented as a contiguous array.

Limitations: no insert/delete in the middle of the list, no in-place sort.


mrkcommon/logging.h
===================

A thin wrapper over _syslog(3)_ mostly for debugging purposes. Adaptor
over _syslog(3)_ and user-defined _FILE *_ -based output (for example
_stderr_) which is very convenient during program development.

A set of macros for convenient logging: _TRACE()_, _DEBUG()_, _INFO()_,
_WARNING()_, _ERROR()_.

Define file-level scope of logging: _LOGGING\_DECLARE()_,
_LOGGING\_SETLEVEL()_, _LOGGING\_CLEARLEVEL()_. Scoped logging (_M_ for
"module"): _MTRACE()_, _MDEBUG()_, _MINFO()_, _MWARNING()_, _MERROR()_.


mrkcommon/profile.h
===================

Selective profiling of program code using x86 _rdtsc_ instruction.

Named profiling blocks. Profiling block is a fragment of a linear code
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
_RB\_\*_ macros because of probbaly recursive algorithm as opposed to
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


