AUTOMAKE_OPTIONS = foreign subdir-objects

diags = diag.txt
BUILT_SOURCES = diag.c diag.h
EXTRA_DIST = $(diags)
CLEANFILES = $(BUILT_SOURCES) *.core
#CLEANFILES += *.in

lib_LTLIBRARIES = libmncommon.la

nodist_libmncommon_la_SOURCES = diag.c diag.h
noinst_HEADERS =

dumpm_include_headers = mncommon/dumpm.h
dumpm_sources = dumpm.c

util_include_headers = mncommon/util.h mncommon/float.h mncommon/endian.h mncommon/assert.h mncommon/malloc.h mncommon/unittest.h
util_sources = util.c

if ARRAY
array_sources = array.c
array_include_headers = mncommon/array.h
endif

if HASH
hash_sources = hash.c
hash_include_headers = mncommon/hash.h
endif

if FASTHASH
fasthash_sources = fasthash.c
fasthash_include_headers = mncommon/fasthash.h
endif

if PROFILE
profile_sources = profile.c
profile_include_headers = mncommon/profile.h
endif

if TRIE
trie_sources = btrie.c pbtrie.c
trie_include_headers = mncommon/btrie.h mncommon/pbtrie.h
endif

if RBT
rbt_sources = rbt.c
rbt_include_headers = mncommon/rbt.h
endif

if TRAVERSEDIR
traversedir_sources = traversedir.c
traversedir_include_headers = mncommon/traversedir.h
endif

if BYTESTREAM
bytestream_sources = bytestream.c bytestream_aux.c vbytestream.c
bytestream_include_headers = mncommon/bytestream.h mncommon/bytestream_aux.h mncommon/vbytestream.h
endif

if JSON
json_sources = json.c jparse.c
json_include_headers = mncommon/json.h mncommon/jparse.h
endif

if STQUEUE
stqueue_sources =
stqueue_include_headers = mncommon/stqueue.h
endif

if DTQUEUE
dtqueue_sources =
dtqueue_include_headers = mncommon/dtqueue.h
endif

if MEMDEBUG
memdebug_sources = memdebug.c
memdebug_include_headers = mncommon/memdebug.h
endif

if MPOOL
mpool_sources = mpool.c
mpool_include_headers = mncommon/mpool.h
endif

if BYTES
bytes_sources = bytes.c
bytes_include_headers = mncommon/bytes.h
endif

if CM
cm_sources = cm.c
cm_include_headers = mncommon/cm.h
endif

if HEAP
heap_sources = heap.c
heap_include_headers = mncommon/heap.h
endif

if BASE64
base64_sources = base64.c
base64_include_headers = mncommon/base64.h
endif

nobase_include_HEADERS = $(dumpm_include_headers) \
			 $(util_include_headers) \
			 $(array_include_headers) \
			 $(hash_include_headers) \
			 $(fasthash_include_headers) \
			 $(profile_include_headers) \
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
			 $(heap_include_headers) \
			 $(base64_include_headers)

libmncommon_la_SOURCES = $(dumpm_sources) \
			  $(util_sources) \
			  $(array_sources) \
			  $(hash_sources) \
			  $(fasthash_sources) \
			  $(profile_sources) \
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
			  $(heap_sources) \
			  $(base64_sources)

DEBUG_LD_FLAGS =
if DEBUG
DEBUG_CC_FLAGS = -g -O0 @CC_DEBUG@
DEBUG_LD_FLAGS += @LIBTOOL_NO_INSTALL@
else
DEBUG_CC_FLAGS = -DNDEBUG -O3 -fomit-frame-pointer
if LTO
DEBUG_CC_FLAGS += @CC_LTO@
DEBUG_LD_FLAGS += @LD_LTO@
endif
endif

if ALLSTATIC
libmncommon_la_LDFLAGS = -all-static
else
libmncommon_la_LDFLAGS =
endif

libmncommon_la_CFLAGS = @_GNU_SOURCE_MACRO@ @_XOPEN_SOURCE_MACRO@ $(DEBUG_CC_FLAGS) -Wall -Wextra -Werror -std=c99 -I$(includedir)
libmncommon_la_LDFLAGS += $(DEBUG_LD_FLAGS) -version-info 0:0:0 -L$(libdir)
libmncommon_la_LIBADD = -lmndiag

SUBDIRS = .

diag.c diag.h: $(diags)
	$(AM_V_GEN) cat $(diags) | sort -u >diag.txt.tmp && mndiagen -v -S diag.txt.tmp -L mncommon -H diag.h -C diag.c *.[ch]

run: all
	for i in $(bin_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;

testrun:
	for i in $(SUBDIRS); do if test "$$i" != "."; then cd $$i && $(MAKE) testrun; fi; done;
