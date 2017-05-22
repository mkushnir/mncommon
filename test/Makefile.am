AUTOMAKE_OPTIONS = foreign subdir-objects
CLEANFILES = *.core
#CLEANFILES += *.in

noinst_PROGRAMS = testfoo
if HASH
noinst_PROGRAMS += testhash testperfhash
endif
if FASTHASH
noinst_PROGRAMS += testfasthash
endif
if LOGGING
noinst_PROGRAMS += testlogging
endif
if PROFILE
noinst_PROGRAMS += testprofile
endif
if CONF
noinst_PROGRAMS += testconf
endif
if TRIE
noinst_PROGRAMS += testtrie testperftree testperftrie testperfpbtrie testperfrb testrborder testtrieorder testpbtrie
endif
if TRAVERSEDIR
noinst_PROGRAMS += testtraversedir
endif
if BYTESTREAM
noinst_PROGRAMS += testvbytestream
endif
if JSON
noinst_PROGRAMS += testjson testjparse
endif
if STQUEUE
noinst_PROGRAMS += teststqueue
endif
if DTQUEUE
noinst_PROGRAMS += testdtqueue
endif
if MEMDEBUG
noinst_PROGRAMS += testmemdebug
endif
if MPOOL
noinst_PROGRAMS += testmpool
endif
if BYTES
noinst_PROGRAMS += testbytes
endif
if CM
noinst_PROGRAMS += testcm
endif

if HEAP
noinst_PROGRAMS += testheap testperfheap
endif

if BASE64
noinst_PROGRAMS += testbase64
endif

noinst_HEADERS = unittest.h rb.h zltan-fasthash.h fnv/fnv.h fnv/longlong.h

BUILT_SOURCES = ../src/diag.c ../src/diag.h

if DEBUG
DEBUG_FLAGS = -g -O0 @CLANG_DEBUG@
#DEBUG_FLAGS = -g -O0 -ftrapv @CLANG_DEBUG@ -DTRRET_DEBUG
else
DEBUG_FLAGS = -DNDEBUG -O3
endif

CFLAGS = @_GNU_SOURCE_MACRO@ @_XOPEN_SOURCE_MACRO@ $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99 -I.. -I../src
LDFLAGS += -L../src/.libs -lmrkcommon

testfoo_SOURCES =  testfoo.c
testfoo_CFLAGS =
testfoo_LDFLAGS =

testhash_SOURCES = ../src/hash.c testhash.c
testhash_CFLAGS =
testhash_LDFLAGS =

testfasthash_SOURCES = ../src/fasthash.c ../src/fnvhash.c zltan-fasthash.c fnv/hash_64a.c testfasthash.c
testfasthash_CFLAGS =
testfasthash_LDFLAGS = -lz

testlogging_SOURCES = ../src/logging.c testlogging.c
testlogging_CFLAGS =
testlogging_LDFLAGS =

testprofile_SOURCES = ../src/list.c ../src/profile.c testprofile.c
testprofile_CFLAGS =
testprofile_LDFLAGS =

testconf_SOURCES = ../src/array.c ../src/conf.c testconf.c
testconf_CFLAGS =
testconf_LDFLAGS =

testtrie_SOURCES = ../src/btrie.c testtrie.c
testtrie_CFLAGS =
testtrie_LDFLAGS =

testperftree_SOURCES = ../src/list.c ../src/btrie.c ../src/rbt.c ../src/profile.c rb.c testperftree.c
testperftree_CFLAGS =
testperftree_LDFLAGS =

testperftrie_SOURCES = ../src/btrie.c ../src/profile.c testperftrie.c
testperftrie_CFLAGS =
testperftrie_LDFLAGS =

testperfpbtrie_SOURCES = ../src/pbtrie.c ../src/profile.c testperfpbtrie.c
testperfpbtrie_CFLAGS =
testperfpbtrie_LDFLAGS =

testperfrb_SOURCES = rb.c ../src/profile.c testperfrb.c
testperfrb_CFLAGS =
testperfrb_LDFLAGS =

testperfhash_SOURCES = rb.c ../src/profile.c testperfhash.c
testperfhash_CFLAGS =
testperfhash_LDFLAGS =

testrborder_SOURCES = rb.c testrborder.c
testrborder_CFLAGS =
testrborder_LDFLAGS =

testtrieorder_SOURCES = ../src/btrie.c testtrieorder.c
testtrieorder_CFLAGS =
testtrieorder_LDFLAGS =

testpbtrie_SOURCES = ../src/pbtrie.c testpbtrie.c
testpbtrie_CFLAGS =
testpbtrie_LDFLAGS =

testtraversedir_SOURCES = ../src/traversedir.c testtraversedir.c
testtraversedir_CFLAGS =
testtraversedir_LDFLAGS =

testvbytestream_SOURCES = ../src/vbytestream.c testvbytestream.c
testvbytestream_CFLAGS =
testvbytestream_LDFLAGS =

testjson_SOURCES = ../src/json.c testjson.c
testjson_CFLAGS =
testjson_LDFLAGS =

testjparse_SOURCES = ../src/jparse.c testjparse.c
testjparse_CFLAGS =
testjparse_LDFLAGS =

teststqueue_SOURCES = teststqueue.c
teststqueue_CFLAGS =
teststqueue_LDFLAGS =

testdtqueue_SOURCES = testdtqueue.c
testdtqueue_CFLAGS =
testdtqueue_LDFLAGS =

testmemdebug_SOURCES = ../src/memdebug.c testmemdebug.c
testmemdebug_CFLAGS =
testmemdebug_LDFLAGS =

testmpool_SOURCES = ../src/mpool.c testmpool.c
testmpool_CFLAGS =
testmpool_LDFLAGS =

testbytes_SOURCES = ../src/bytes.c testbytes.c
testbytes_CFLAGS =
testbytes_LDFLAGS =

testcm_SOURCES = ../src/cm.c testcm.c
testcm_CFLAGS =
testcm_LDFLAGS =

testheap_SOURCES = ../src/heap.c testheap.c
testheap_CFLAGS =
testheap_LDFLAGS =

testperfheap_SOURCES = ../src/heap.c testperfheap.c
testperfheap_CFLAGS =
testperfheap_LDFLAGS =

testbase64_SOURCES = ../src/base64.c testbase64.c
testbase64_CFLAGS =
testbase64_LDFLAGS =

../src/diag.c ../src/diag.h: ../src/diag.txt
	$(AM_V_GEN) /bin/sh cat ../src/diag.txt | sort -u | ../gen-diag mrkcommon ../src

testrun: all
	for i in $(noinst_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;
