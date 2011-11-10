dnl#
dnl# Process this file with autoconf to produce a configure script.
dnl#

AC_INIT
AM_INIT_AUTOMAKE(libptytty,1.5)
AC_CONFIG_HEADER(config.h:config.h.in)

AC_CANONICAL_HOST

AC_PROG_CXX
AC_PROG_INSTALL
AC_PROG_LIBTOOL

AC_LANG(C++)

PTY_CHECK

TTY_GROUP_CHECK

UTMP_CHECK

SCM_RIGHTS_CHECK

AC_CONFIG_FILES([Makefile])
AC_OUTPUT
