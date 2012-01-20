AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I .

EXTRA_DIST = Changes README doc/libptytty.3.pod doc/libptytty.3 eg/c-sample.c \
	     src/ptytty.h src/ptytty_conf.h

man_MANS = doc/libptytty.3

include_HEADERS = src/libptytty.h

lib_LTLIBRARIES = libptytty.la

libptytty_la_SOURCES = src/c-api.C src/fdpass.C src/logging.C src/proxy.C src/ptytty.C

noinst_PROGRAMS = c-sample
c_sample_SOURCES = eg/c-sample.c
c_sample_LDADD = libptytty.la
c_sample_CPPFLAGS = -I$(top_srcdir)/src
c_sample_LINK = $(CXXLINK)

doc/libptytty.3: doc/libptytty.3.pod
	pod2man -n libptytty -r "$(VERSION)" -q\" -c LIBPTYTTY -s3 <$< >$@

README: doc/libptytty.3.pod
	pod2text <$< >$@

alldoc: doc/libptytty.3 README
