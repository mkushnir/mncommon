AUTOMAKE_OPTIONS = foreign

distdir = ../$(PACKAGE)-$(VERSION)/src
dist_HEADERS =

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
dist_HEADERS += conf_private.h
endif
if TRIE
trie_sources = trie.c
trie_include_headers = mrkcommon/trie.h
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
bytestream_sources = bytestream.c
bytestream_include_headers = mrkcommon/bytestream.h
endif
if JSON
json_sources = json.c
json_include_headers = mrkcommon/json.h
endif

nobase_include_HEADERS = $(dumpm_include_headers) $(util_include_headers) $(array_include_headers) $(list_include_headers) $(logging_include_headers) $(profile_include_headers) $(conf_include_headers) $(trie_include_headers) $(rbt_include_headers) $(traversedir_include_headers) $(bytestream_include_headers) $(json_include_headers)

libmrkcommon_la_SOURCES = $(dumpm_sources) $(array_sources) $(list_sources) $(logging_sources) $(profile_sources) $(conf_sources) $(trie_sources) $(rbt_sources) $(traversedir_sources) $(bytestream_sources) $(json_sources)
nodist_libmrkcommon_la_SOURCES = diag.c

if DEBUG
DEBUG_FLAGS = -g -O0 -ftrapv -fcatch-undefined-behavior
else
DEBUG_FLAGS = -DNDEBUG -O3
endif

libmrkcommon_la_CFLAGS = $(DEBUG_FLAGS) -Wall -Wextra -Werror -std=c99
libmrkcommon_la_LDFLAGS = -version-info 0:0:0

SUBDIRS = . test

diag.c diag.h: diag.txt
	sh ./gen-diag

run: all
	for i in $(bin_PROGRAMS); do if test -x ./$$i; then LD_LIRARY_PATH=$(libdir) ./$$i; fi; done;

testrun:
	for i in $(SUBDIRS); do if test "$$i" != "."; then cd $$i && $(MAKE) testrun; fi; done;
