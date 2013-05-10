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
noinst_PROGRAMS += testtrie testperftree
endif
if TRAVERSEDIR
noinst_PROGRAMS += testtraversedir
endif
if JSON
noinst_PROGRAMS += testjson
endif

distdir = ../../$(PACKAGE)-$(VERSION)/src/test
dist_HEADERS = unittest.h rb.h

BUILT_SOURCES = ../diag.c ../diag.h

#CFLAGS = -Wall -Wextra -Werror -std=c99 -I.. -g -O3 -DNDEBUG
CFLAGS = -Wall -Wextra -Werror -std=c99 -I.. -g -O0 -ftrapv -fcatch-undefined-behavior
#CFLAGS = -Wall -Wextra -Werror -std=c99 -I.. -g -O0 -DTRRET_DEBUG
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

testtraversedir_SOURCES = ../traversedir.c testtraversedir.c
testtraversedir_CFLAGS =
testtraversedir_LDFLAGS =

testjson_SOURCES = ../json.c testjson.c
testjson_CFLAGS =
testjson_LDFLAGS =

../diag.c ../diag.h: ../diag.txt
	sh ../gen-diag

testrun: all
	for i in $(noinst_PROGRAMS); do if test -x ./$$i; then LD_LIRARY_PATH=$(libdir) ./$$i; fi; done;
