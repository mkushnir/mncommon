AUTOMAKE_OPTIONS = foreign subdir-objects

distdir = $(top_srcdir)/$(PACKAGE)-$(VERSION)/src
dist_HEADERS =
noinst_HEADERS =

BUILT_SOURCES = diag.c diag.h
EXTRA_DIST = diag.txt gen-diag
CLEANFILES = $(BUILT_SOURCES) *.core
#CLEANFILES += *.in

lib_LTLIBRARIES = libmrkcommon.la

dumpm_include_headers = mrkcommon/dumpm.h
dumpm_sources = dumpm.c

util_include_headers = mrkcommon/util.h

if ARRAY
array_sources = array.c
array_include_headers = mrkcommon/array.h
endif

if LIST
list_sources = list.c
list_include_headers = mrkcommon/list.h
endif

if HASH
hash_sources = hash.c
hash_include_headers = mrkcommon/hash.h
endif

if FASTHASH
fasthash_sources = fasthash.c
fasthash_include_headers = mrkcommon/fasthash.h
endif

if LOGGING
logging_sources = logging.c
logging_include_headers = mrkcommon/logging.h
endif

if PROFILE
profile_sources = profile.c
profile_include_headers = mrkcommon/profile.h
endif

if CONF
conf_sources = conf.c
conf_include_headers = mrkcommon/conf.h
noinst_HEADERS += conf_private.h
endif

if TRIE
trie_sources = btrie.c pbtrie.c
trie_include_headers = mrkcommon/btrie.h mrkcommon/pbtrie.h
endif

if RBT
rbt_sources = rbt.c
rbt_include_headers = mrkcommon/rbt.h
endif

if TRAVERSEDIR
traversedir_sources = traversedir.c
traversedir_include_headers = mrkcommon/traversedir.h
endif

if BYTESTREAM
bytestream_sources = bytestream.c bytestream_aux.c vbytestream.c
bytestream_include_headers = mrkcommon/bytestream.h mrkcommon/bytestream_aux.h mrkcommon/vbytestream.h
endif

if JSON
json_sources = json.c jparse.c
json_include_headers = mrkcommon/json.h mrkcommon/jparse.h
endif

if STQUEUE
stqueue_sources =
stqueue_include_headers = mrkcommon/stqueue.h
endif

if DTQUEUE
dtqueue_sources =
dtqueue_include_headers = mrkcommon/dtqueue.h
endif

if MEMDEBUG
memdebug_sources = memdebug.c
memdebug_include_headers = mrkcommon/memdebug.h
endif

if MPOOL
mpool_sources = mpool.c
mpool_include_headers = mrkcommon/mpool.h
endif

if BYTES
bytes_sources = bytes.c
bytes_include_headers = mrkcommon/bytes.h
endif

if CM
cm_sources = cm.c
cm_include_headers = mrkcommon/cm.h
endif

if HEAP
heap_sources = heap.c
heap_include_headers = mrkcommon/heap.h
endif

nobase_include_HEADERS = $(dumpm_include_headers) \
			 $(util_include_headers) \
			 $(array_include_headers) \
			 $(list_include_headers) \
			 $(hash_include_headers) \
			 $(fasthash_include_headers) \
			 $(logging_include_headers) \
			 $(profile_include_headers) \
			 $(conf_include_headers) \
			 $(trie_include_headers) \
			 $(rbt_include_headers) \
			 $(traversedir_include_headers) \
			 $(bytestream_include_headers) \
			 $(json_include_headers) \
			 $(stqueue_include_headers) \
			 $(dtqueue_include_headers) \
			 $(memdebug_include_headers) \
			 $(mpool_include_headers) \
			 $(bytes_include_headers) \
			 $(cm_include_headers) \
			 $(heap_include_headers)

libmrkcommon_la_SOURCES = $(dumpm_sources) \
			  $(array_sources) \
			  $(list_sources) \
			  $(hash_sources) \
			  $(fasthash_sources) \
			  $(logging_sources) \
			  $(profile_sources) \
			  $(conf_sources) \
			  $(trie_sources) \
			  $(rbt_sources) \
			  $(traversedir_sources) \
			  $(bytestream_sources) \
			  $(json_sources) \
			  $(stqueue_sources) \
			  $(dtqueue_sources) \
			  $(memdebug_sources) \
			  $(mpool_sources) \
			  $(bytes_sources) \
			  $(cm_sources) \
			  $(heap_sources)

nodist_libmrkcommon_la_SOURCES = diag.c

if DEBUG
DEBUG_FLAGS = -g -O0 @CLANG_DEBUG@
else
DEBUG_FLAGS = -DNDEBUG -O3
endif

libmrkcommon_la_CFLAGS = @_GNU_SOURCE_MACRO@ $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99
libmrkcommon_la_LDFLAGS = -version-info 0:0:0

SUBDIRS = .

diag.c diag.h: diag.txt
	$(AM_V_GEN) /bin/sh ./gen-diag mrkcommon

run: all
	for i in $(bin_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;

testrun:
	for i in $(SUBDIRS); do if test "$$i" != "."; then cd $$i && $(MAKE) testrun; fi; done;
