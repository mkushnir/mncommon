AUTOMAKE_OPTIONS = foreign subdir-objects
CLEANFILES = $(BUILT_SOURCES) *.core
#CLEANFILES += *.in

diags = ../src/diag.txt

src_sources = ../src/dumpm.c ../src/array.c ../src/rbt.c

noinst_PROGRAMS = testfoo
if HASH
noinst_PROGRAMS += testhash testperfhash
src_sources += ../src/hash.c
endif
if FASTHASH
noinst_PROGRAMS += testfasthash
src_sources += ../src/fasthash.c
endif
if PROFILE
noinst_PROGRAMS += testprofile
src_sources += ../src/profile.c
endif
if TRIE
noinst_PROGRAMS += testtrie testperftree testperftrie testperfpbtrie testperfrb testrborder testtrieorder testpbtrie
src_sources += ../src/btrie.c ../src/pbtrie.c
endif
if TRAVERSEDIR
noinst_PROGRAMS += testtraversedir
src_sources += ../src/traversedir.c
endif
if BYTESTREAM
noinst_PROGRAMS += testvbytestream
src_sources += ../src/bytestream.c ../src/bytestream_aux.c ../src/vbytestream.c
endif
if JSON
noinst_PROGRAMS += testjson testjparse
src_sources += ../src/json.c ../src/jparse.c
endif
if STQUEUE
noinst_PROGRAMS += teststqueue
endif
if DTQUEUE
noinst_PROGRAMS += testdtqueue testdtqueueperf
endif
if MEMDEBUG
noinst_PROGRAMS += testmemdebug
src_sources += ../src/memdebug.c
endif
if MPOOL
noinst_PROGRAMS += testmpool
src_sources += ../src/mpool.c
endif
if BYTES
noinst_PROGRAMS += testbytes
src_sources += ../src/bytes.c
endif
if CM
noinst_PROGRAMS += testcm
src_sources += ../src/cm.c
endif
if HEAP
noinst_PROGRAMS += testheap testperfheap
src_sources += ../src/heap.c
endif
if BASE64
noinst_PROGRAMS += testbase64
src_sources += ../src/base64.c
endif
if CMOCKA
noinst_PROGRAMS += testbar
endif

noinst_HEADERS = unittest.h rb.h zltan-fasthash.h fnv/fnv.h fnv/longlong.h freebsd/tree.h

BUILT_SOURCES = diag.c diag.h

static_sources =
DEBUG_LD_FLAGS =
if DEBUG
DEBUG_CC_FLAGS = -g -O0 @CC_DEBUG@
DEBUG_LD_FLAGS += @LIBTOOL_NO_INSTALL@
else
DEBUG_CC_FLAGS = -DNDEBUG -O3 -fomit-frame-pointer
if LTO
static_sources += $(src_sources)
DEBUG_CC_FLAGS += @CC_LTO@
DEBUG_LD_FLAGS += @LD_LTO@
endif
endif

if ALLSTATIC
LDFLAGS += -all-static
endif

CFLAGS += @_GNU_SOURCE_MACRO@ @_XOPEN_SOURCE_MACRO@ $(DEBUG_CC_FLAGS) -Wall -Wextra -Werror -std=c99 -I.. -I../src -I$(includedir)
LDFLAGS += $(DEBUG_LD_FLAGS) -L../src/.libs -L$(libdir)
LDADD = -lmrkcommon -lmndiag

if CMOCKA
CFLAGS += -I$(includedir) -DCMOCKA_TESTING
LDFLAGS += -L$(libdir)
LDADD += -lcmocka
endif

testfoo_SOURCES =  testfoo.c
testfoo_CFLAGS =
testfoo_LDFLAGS =

testhash_SOURCES = $(static_sources) testhash.c
testhash_CFLAGS =
testhash_LDFLAGS =

testfasthash_SOURCES = $(static_sources) ../src/fnvhash.c ../src/mrkcommon/fnvhash.h zltan-fasthash.c fnv/hash_64a.c testfasthash.c
testfasthash_CFLAGS = -Wno-implicit-fallthrough
testfasthash_LDADD = $(LDADD) -lz

testprofile_SOURCES = $(static_sources) testprofile.c
testprofile_CFLAGS =
testprofile_LDFLAGS =

testtrie_SOURCES = $(static_sources) testtrie.c
testtrie_CFLAGS =
testtrie_LDFLAGS =

testperftree_SOURCES = $(static_sources) rb.c testperftree.c
testperftree_CFLAGS =
testperftree_LDFLAGS =

testperftrie_SOURCES = $(static_sources) testperftrie.c
testperftrie_CFLAGS =
testperftrie_LDFLAGS =

testperfpbtrie_SOURCES = $(static_sources) testperfpbtrie.c
testperfpbtrie_CFLAGS =
testperfpbtrie_LDFLAGS =

testperfrb_SOURCES = $(static_sources) rb.c testperfrb.c
testperfrb_CFLAGS =
testperfrb_LDFLAGS =

testperfhash_SOURCES = $(static_sources) rb.c testperfhash.c
testperfhash_CFLAGS =
testperfhash_LDFLAGS =

testrborder_SOURCES = rb.c testrborder.c
testrborder_CFLAGS =
testrborder_LDFLAGS =

testtrieorder_SOURCES = $(static_sources) testtrieorder.c
testtrieorder_CFLAGS =
testtrieorder_LDFLAGS =

testpbtrie_SOURCES = $(static_sources) testpbtrie.c
testpbtrie_CFLAGS =
testpbtrie_LDFLAGS =

testtraversedir_SOURCES = $(static_sources) testtraversedir.c
testtraversedir_CFLAGS =
testtraversedir_LDFLAGS =

testvbytestream_SOURCES = $(static_sources) testvbytestream.c
testvbytestream_CFLAGS =
testvbytestream_LDFLAGS =

testjson_SOURCES = $(static_sources) testjson.c
testjson_CFLAGS =
testjson_LDFLAGS =

testjparse_SOURCES = $(static_sources) testjparse.c
testjparse_CFLAGS =
testjparse_LDFLAGS =

teststqueue_SOURCES = teststqueue.c
teststqueue_CFLAGS =
teststqueue_LDFLAGS =

testdtqueue_SOURCES = testdtqueue.c
testdtqueue_CFLAGS =
testdtqueue_LDFLAGS =

testdtqueueperf_SOURCES = testdtqueueperf.c
testdtqueueperf_CFLAGS =
testdtqueueperf_LDFLAGS =

testmemdebug_SOURCES = $(static_sources) testmemdebug.c
testmemdebug_CFLAGS =
testmemdebug_LDFLAGS =

testmpool_SOURCES = $(static_sources) testmpool.c
testmpool_CFLAGS =
testmpool_LDFLAGS =

testbytes_SOURCES = $(static_sources) testbytes.c
testbytes_CFLAGS =
testbytes_LDFLAGS =

testcm_SOURCES = $(static_sources) testcm.c
testcm_CFLAGS =
testcm_LDFLAGS =

testheap_SOURCES = $(static_sources) testheap.c
testheap_CFLAGS =
testheap_LDFLAGS =

testperfheap_SOURCES = $(static_sources) testperfheap.c
testperfheap_CFLAGS =
testperfheap_LDFLAGS =

testbase64_SOURCES = $(static_sources) testbase64.c
testbase64_CFLAGS =
testbase64_LDFLAGS =

if CMOCKA
testbar_SOURCES = $(static_sources) testbar.c
testbar_CFLAGS =
testbar_LDFLAGS =
endif

diag.c diag.h: ../src/diag.txt
	$(AM_V_GEN) cat $(diags) | sort -u >diag.txt.tmp && mndiagen -v -S diag.txt.tmp -L mrkcommon -H diag.h -C diag.c ../src/*.[ch]

testrun: all
	for i in $(noinst_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;
