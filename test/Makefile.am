AUTOMAKE_OPTIONS = foreign subdir-objects
CLEANFILES = *.core
#CLEANFILES += *.in

noinst_PROGRAMS = testfoo
if DICT
noinst_PROGRAMS += testdict
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
noinst_PROGRAMS += testtrie testperftree testperftrie testperfrb testrborder testtrieorder
endif
if TRAVERSEDIR
noinst_PROGRAMS += testtraversedir
endif
if JSON
noinst_PROGRAMS += testjson
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

distdir = ../$(PACKAGE)-$(VERSION)/test
dist_HEADERS = unittest.h rb.h zltan-fasthash.h

BUILT_SOURCES = ../src/diag.c ../src/diag.h

if DEBUG
DEBUG_FLAGS = -g -O0 @CLANG_DEBUG@
#DEBUG_FLAGS = -g -O0 -ftrapv @CLANG_DEBUG@ -DTRRET_DEBUG
else
DEBUG_FLAGS = -DNDEBUG -O3
endif

CFLAGS = @_GNU_SOURCE_MACRO@ $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99 -I.. -I../src
LDFLAGS += -L../src/.libs -lmrkcommon

testfoo_SOURCES =  testfoo.c
testfoo_CFLAGS =
testfoo_LDFLAGS =

testdict_SOURCES = ../src/dict.c testdict.c
testdict_CFLAGS =
testdict_LDFLAGS =

testfasthash_SOURCES = ../src/fasthash.c zltan-fasthash.c testfasthash.c
testfasthash_CFLAGS =
testfasthash_LDFLAGS =

testlogging_SOURCES = ../src/logging.c testlogging.c
testlogging_CFLAGS =
testlogging_LDFLAGS =

testprofile_SOURCES = ../src/list.c ../src/profile.c testprofile.c
testprofile_CFLAGS =
testprofile_LDFLAGS =

testconf_SOURCES = ../src/array.c ../src/conf.c testconf.c
testconf_CFLAGS =
testconf_LDFLAGS =

testtrie_SOURCES = ../src/trie.c testtrie.c
testtrie_CFLAGS =
testtrie_LDFLAGS =

testperftree_SOURCES = ../src/list.c ../src/trie.c ../src/rbt.c ../src/profile.c rb.c testperftree.c
testperftree_CFLAGS =
testperftree_LDFLAGS =

testperftrie_SOURCES = ../src/trie.c ../src/profile.c testperftrie.c
testperftrie_CFLAGS =
testperftrie_LDFLAGS =

testperfrb_SOURCES = rb.c ../src/profile.c testperfrb.c
testperfrb_CFLAGS =
testperfrb_LDFLAGS =

testrborder_SOURCES = rb.c testrborder.c
testrborder_CFLAGS =
testrborder_LDFLAGS =

testtrieorder_SOURCES = ../src/trie.c testtrieorder.c
testtrieorder_CFLAGS =
testtrieorder_LDFLAGS =

testtraversedir_SOURCES = ../src/traversedir.c testtraversedir.c
testtraversedir_CFLAGS =
testtraversedir_LDFLAGS =

testjson_SOURCES = ../src/json.c testjson.c
testjson_CFLAGS =
testjson_LDFLAGS =

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

../src/diag.c ../src/diag.h: ../src/diag.txt
	sh ../gen-diag mrkcommon

testrun: all
	for i in $(noinst_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;