dnl#
dnl# Process this file with autoconf to produce a configure script.
dnl#

AC_INIT([libptytty], [1.9])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])

AM_INIT_AUTOMAKE

AC_CANONICAL_HOST

AC_PROG_CXX
AC_PROG_INSTALL
LT_INIT

AC_LANG([C++])

PTY_CHECK

TTY_GROUP_CHECK

UTMP_CHECK

SCM_RIGHTS_CHECK

AC_CONFIG_FILES([Makefile libptytty.pc])
AC_OUTPUT
