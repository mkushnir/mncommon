AUTOMAKE_OPTIONS = foreign
CLEANFILES = *.core
#CLEANFILES += *.in

noinst_PROGRAMS = testfoo
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

distdir = ../../$(PACKAGE)-$(VERSION)/src/test
dist_HEADERS = unittest.h rb.h

BUILT_SOURCES = ../diag.c ../diag.h

if DEBUG
DEBUG_FLAGS = -g -O0 @CLANG_DEBUG@
#DEBUG_FLAGS = -g -O0 -ftrapv @CLANG_DEBUG@ -DTRRET_DEBUG
else
DEBUG_FLAGS = -DNDEBUG -O3
endif

CFLAGS = $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99 -I..
LDFLAGS += -L../.libs -lmrkcommon

testfoo_SOURCES =  testfoo.c
testfoo_CFLAGS =
testfoo_LDFLAGS =

testlogging_SOURCES = ../logging.c testlogging.c
testlogging_CFLAGS =
testlogging_LDFLAGS =

testprofile_SOURCES = ../list.c ../profile.c testprofile.c
testprofile_CFLAGS =
testprofile_LDFLAGS =

testconf_SOURCES = ../array.c ../conf.c testconf.c
testconf_CFLAGS =
testconf_LDFLAGS =

testtrie_SOURCES = ../trie.c testtrie.c
testtrie_CFLAGS =
testtrie_LDFLAGS =

testperftree_SOURCES = ../list.c ../trie.c ../rbt.c ../profile.c rb.c testperftree.c
testperftree_CFLAGS =
testperftree_LDFLAGS =

testperftrie_SOURCES = ../trie.c ../profile.c testperftrie.c
testperftrie_CFLAGS =
testperftrie_LDFLAGS =

testperfrb_SOURCES = rb.c ../profile.c testperfrb.c
testperfrb_CFLAGS =
testperfrb_LDFLAGS =

testrborder_SOURCES = rb.c testrborder.c
testrborder_CFLAGS =
testrborder_LDFLAGS =

testtrieoeder_SOURCES = ../trie.c testtrieoeder.c
testtrieoeder_CFLAGS =
testtrieoeder_LDFLAGS =

testtraversedir_SOURCES = ../traversedir.c testtraversedir.c
testtraversedir_CFLAGS =
testtraversedir_LDFLAGS =

testjson_SOURCES = ../json.c testjson.c
testjson_CFLAGS =
testjson_LDFLAGS =

teststqueue_SOURCES = teststqueue.c
teststqueue_CFLAGS =
teststqueue_LDFLAGS =

testdtqueue_SOURCES = testdtqueue.c
testdtqueue_CFLAGS =
testdtqueue_LDFLAGS =

testmemdebug_SOURCES = ../memdebug.c testmemdebug.c
testmemdebug_CFLAGS =
testmemdebug_LDFLAGS =

../diag.c ../diag.h: ../diag.txt
	sh ../gen-diag

testrun: all
	for i in $(noinst_PROGRAMS); do if test -x ./$$i; then LD_LIBRARY_PATH=$(libdir) ./$$i; fi; done;
