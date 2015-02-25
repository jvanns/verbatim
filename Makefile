-include *.d
-include .config

NAME = verbatim
PREFIX ?= $(HOME)
INSTALL ?= install

BINDIR ?= $(PREFIX)/bin
MANDIR ?= $(PREFIX)/share/man
PYTHON_SITELIB ?= $(PREFIX)/lib/python/site-packages

CC ?= gcc
CXX ?= g++
LIBS ?=
CPPFLAGS += -MMD -MP -I./src/
CXXFLAGS += -Wall -pedantic -Wno-long-long

.PHONY: clean dist install all

# Selective, per-module/unit additions
src/Traverse.o: CPPFLAGS += -D_FILE_OFFSET_BITS=64

# Main program dependencies
VERBATIM_DEPS = src/Traverse.o

# Tests
test_delegate: src/tests/delegate.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^

test_traverse: src/tests/traverse.o src/Traverse.o
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^

# Main program
#verbatim: src/verbatim.o $(COMMON_DEPS) $(VERBATIM_DEPS)
#	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(LDFLAGS) $(LIBS) -o $@ $^

# Phony targets
all: test_delegate test_traverse

pkg:
	mkdir -p pkg
	V=$$(git describe --abbrev=0) \
		&& git archive --prefix=$(NAME)-$$V/ HEAD \
		| gzip > pkg/$(NAME)-$$V.tar.gz

install:
	# Directories
	$(INSTALL) -d $(DESTDIR)$(BINDIR)
	$(INSTALL) -d $(DESTDIR)$(MANDIR)/man1
	$(INSTALL) -d $(DESTDIR)$(PYTHON_SITELIB)
	# Main program
	$(INSTALL) -m 0755 verbatim $(DESTDIR)$(BINDIR)
	# Man page
	$(INSTALL) -m 0644 docs/*.1 $(DESTDIR)$(MANDIR)/man1

clean:
	-rm -f test_delegate test_traverse pkg/$(NAME)-*.tar.gz
	-find `pwd` -depth -type f -name '*.[od]' -prune \
		\! -path "`pwd`[/].git/*" | xargs rm -f

