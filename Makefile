-include *.d
-include .config

NAME = verbatim
PREFIX ?= $(HOME)
INSTALL ?= install

BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man

CC ?= gcc
CXX ?= g++
LDLIBS ?=
LDFLAGS ?=
CPPFLAGS += -MMD -MP -Isrc -Iext -Isub -Wno-unused-local-typedefs
CXXFLAGS += -Wall -pedantic -Wno-long-long

.PHONY: clean dist install all lmdb

# Selective, per-module/unit additions
src/Traverse.o: CPPFLAGS += -D_FILE_OFFSET_BITS=64
src/Database.o: CPPFLAGS += -Isub/lmdb/libraries/liblmdb
src/verbatim.o: CPPFLAGS += -Isub/lmdb/libraries/liblmdb
src/tests/glim.o: CPPFLAGS += -Isub/lmdb/libraries/liblmdb

verbatim: LDFLAGS += -Lsub/lmdb/libraries/liblmdb
verbatim: LDLIBS += -llmdb -lboost_serialization
test_glim: LDFLAGS += -Lsub/lmdb/libraries/liblmdb
test_glim: LDLIBS += -llmdb -lboost_serialization

# lmdb submodule
lmdb:
	$(MAKE) -C sub/lmdb/libraries/liblmdb

# Common utility dependencies
COMMON_DEPS = src/utility/Timer.o \
	src/utility/Hash.o

# Main program dependencies
src/Database.o: lmdb
VERBATIM_DEPS = src/Traverse.o \
	src/Database.o

# Tests
test_delegate: src/tests/delegate.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

test_traverse: src/tests/traverse.o src/Traverse.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

src/tests/glim.o: lmdb
test_glim: src/tests/glim.o $(COMMON_DEPS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Main program
verbatim: src/verbatim.o $(VERBATIM_DEPS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Phony targets
all: lmdb test_delegate test_traverse test_glim verbatim

pkg:
	mkdir -p pkg
	V=$$(git describe --abbrev=0) \
		&& git archive --prefix=$(NAME)-$$V/ HEAD \
		| gzip > pkg/$(NAME)-$$V.tar.gz

install:
	# Directories
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)/man1
	# Main program
	$(INSTALL) -m 0755 verbatim $(DESTDIR)$(BINDIR)
	# Man page
	$(INSTALL) -m 0644 docs/*.1 $(DESTDIR)$(MANDIR)/man1

clean:
	-rm -f verbatim test_* pkg/$(NAME)-*.tar.gz
	-find `pwd` -depth -type f -name '*.[od]' -prune \
		\! -path "`pwd`[/].git/*" | xargs rm -f
	$(MAKE) -C sub/lmdb/libraries/liblmdb clean
