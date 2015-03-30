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
CXXFLAGS += -Wall -pedantic -Wno-long-long -std=c++11

# Phony targets
.PHONY: clean dist install all lmdb

# Selective, per-module/unit additions
src/utility/Hash.o: CXXFLAGS += -O3
src/Context.o: CPPFLAGS += -Isub/lmdb/libraries/liblmdb
src/Database.o: CPPFLAGS += -Isub/lmdb/libraries/liblmdb
src/verbatim.o: CPPFLAGS += -Isub/lmdb/libraries/liblmdb
src/verbatim-cat.o: CPPFLAGS += -Isub/lmdb/libraries/liblmdb
src/tests/glim.o: CPPFLAGS += -Isub/lmdb/libraries/liblmdb

test_traverse: LDLIBS += -lboost_thread -lboost_system

test_glim: LDFLAGS += -Lsub/lmdb/libraries/liblmdb
test_glim: LDLIBS += -llmdb -lboost_serialization -lboost_thread -lboost_system

verbatim: LDFLAGS += -Lsub/lmdb/libraries/liblmdb
verbatim: LDLIBS += -llmdb -lboost_serialization -lboost_thread -lboost_system -ltag

verbatim-cat: LDFLAGS += -Lsub/lmdb/libraries/liblmdb
verbatim-cat: LDLIBS += -llmdb -lboost_serialization -lboost_thread -lboost_system -ltag

# lmdb submodule
lmdb:
	$(MAKE) -C sub/lmdb/libraries/liblmdb

# Common utility dependencies
UTILITY_OBJS = src/utility/Timer.o \
	src/utility/ThreadPool.o \
	src/utility/Exception.o \
	src/utility/Hash.o

# Main program dependencies
src/Database.o: lmdb
VERBATIM_OBJS = src/Traverse.o \
	src/Database.o \
	src/Context.o \
	src/Tag.o

# Tests
test_delegate: src/tests/delegate.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

test_traverse: src/tests/traverse.o src/Traverse.o $(UTILITY_OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

src/tests/glim.o: lmdb
test_glim: src/tests/glim.o $(UTILITY_OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

suffix_array: src/tests/suffix_array.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

# Main programs
verbatim: src/verbatim.o $(VERBATIM_OBJS) $(UTILITY_OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

verbatim-cat: src/verbatim-cat.o $(VERBATIM_OBJS) $(UTILITY_OBJS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) -o $@ $^ $(LDLIBS)

tests: test_glim test_delegate test_traverse suffix_array
all: tests verbatim verbatim-cat

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
	-rm -f verbatim verbatim-cat test_* suffix_array pkg/$(NAME)-*.tar.gz
	-find `pwd` -depth -type f -name '*.[od]' -prune \
		\! -path "`pwd`[/].git/*" | xargs rm -f
	$(MAKE) -C sub/lmdb/libraries/liblmdb clean
