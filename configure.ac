dnl#
dnl# Process this file with autoconf to produce a configure script.
dnl#

AC_INIT
AC_PREREQ(2.50)
AC_CONFIG_SRCDIR([src/logging.C])
AC_CONFIG_HEADER(config.h:config.h.in)

AC_CANONICAL_HOST

AC_PROG_CXX
AC_PROG_INSTALL

AC_LANG(C++)

PTY_CHECK

UTMP_CHECK

SCM_RIGHTS_CHECK

AC_CONFIG_FILES([Makefile \
doc/Makefile \
src/Makefile \
])
AC_OUTPUT
