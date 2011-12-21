AUTOMAKE_OPTIONS = foreign subdir-objects
ACLOCAL_AMFLAGS = -I .

EXTRA_DIST = Changes README doc/libptytty.3.pod doc/libptytty.3 eg/c-sample.c \
	     src/ptytty.h src/ptytty_conf.h

man_MANS = doc/libptytty.3

include_HEADERS = src/libptytty.h

lib_LTLIBRARIES = libptytty.la

libptytty_la_SOURCES = src/c-api.C src/fdpass.C src/logging.C src/proxy.C src/ptytty.C

doc/libptytty.3: doc/libptytty.3.pod
	pod2man -n libptytty -r "$(VERSION)" -q\" -c LIBPTYTTY -s3 <$< >$@

README: doc/libptytty.3.pod
	pod2text <$< >$@

alldoc: doc/libptytty.3 README
