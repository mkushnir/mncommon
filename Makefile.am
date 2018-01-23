distdir = $(top_srcdir)/$(PACKAGE)-$(VERSION)
SUBDIRS = src man test
#CLEANFILES = *.in *.m4 *.log *.scan configure *.core

run:
	for i in $(SUBDIRS); do if test "$$i" != "."; then cd $$i && $(MAKE) run && cd ..; fi; done;

testrun:
	for i in $(SUBDIRS); do if test "$$i" != "."; then cd $$i && $(MAKE) testrun && cd ..; fi; done;
