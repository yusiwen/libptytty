dnl this file is part of libptytty, do not make local modifications
dnl http://software.schmorp.de/pkg/libptytty

AC_DEFUN([PT_FIND_FILE],
[AC_CACHE_CHECK(where $1 is located, pt_cv_path_$1,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
$5
int main()
{
    char **path, *list[] = { $4, NULL };
    FILE *f = fopen("conftestval", "w");
    if (!f) return 1;
#ifdef $2
    fprintf(f, "%s\n", $2);
#elif defined($3)
    fprintf(f, "%s\n", $3);
#else
    for (path = list; *path; path++) {
        struct stat st;
        if (stat(*path, &st) == 0) {
            fprintf(f, "%s\n", *path);
            break;
        }
    }
#endif
    return fclose(f) != 0;
}]])],[pt_cv_path_$1=`cat conftestval`],[pt_cv_path_$1=],
[AC_MSG_WARN(Define $2 in config.h manually)])])
if test x$pt_cv_path_$1 != x; then
  AC_DEFINE_UNQUOTED($2, "$pt_cv_path_$1", Define location of $1)
fi])

AC_DEFUN([PTY_CHECK],
[
AC_CHECK_HEADERS( \
  pty.h \
  util.h \
  libutil.h \
  sys/ioctl.h \
  sys/stropts.h \
  stropts.h \
)

AC_CHECK_FUNCS( \
  revoke \
  _getpty \
  getpt \
  posix_openpt \
  isastream \
  setuid \
  seteuid \
  setreuid \
  setresuid \
)

have_clone=no

AC_MSG_CHECKING(for /dev/ptc)
if test -e /dev/ptc; then
  AC_MSG_RESULT(yes)
  AC_DEFINE(CLONE_DEVICE, "/dev/ptc", [clone device filename])
  have_clone=yes
else
  AC_MSG_RESULT(no)
fi

case $host in
  *-*-cygwin*)
    have_clone=yes
    AC_DEFINE(CLONE_DEVICE, "/dev/ptmx", [clone device filename])
    ;;
  *)
    AC_MSG_CHECKING(for /dev/ptmx)
    if test -e /dev/ptmx; then
      AC_MSG_RESULT(yes)
      AC_DEFINE(HAVE_DEV_PTMX, 1, [Define to 1 if you have /dev/ptmx])
      AC_DEFINE(CLONE_DEVICE, "/dev/ptmx", [clone device filename])
      have_clone=yes
    else
      AC_MSG_RESULT(no)
    fi
    ;;
esac

if test x$ac_cv_func_getpt = xyes -o x$ac_cv_func_posix_openpt = xyes -o x$have_clone = xyes; then
  AC_MSG_CHECKING(for UNIX98 ptys)
  AC_LINK_IFELSE([AC_LANG_PROGRAM([[#include <stdlib.h>]],
              [[grantpt(0);unlockpt(0);ptsname(0);]])],
              [unix98_pty=yes
               AC_DEFINE(UNIX98_PTY, 1, "")
               AC_MSG_RESULT(yes)],
              [AC_MSG_RESULT(no)])
fi

if test -z "$unix98_pty"; then
  AC_SEARCH_LIBS(openpty, util, AC_DEFINE(HAVE_OPENPTY, 1, ""))
fi
])

AC_DEFUN([UTMP_CHECK],
[
support_utmp=yes
support_wtmp=yes
support_lastlog=yes

AC_ARG_ENABLE(utmp,
  [AS_HELP_STRING([--enable-utmp],[enable utmp (utmpx) support])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_utmp=$enableval
  fi])

AC_ARG_ENABLE(wtmp,
  [AS_HELP_STRING([--enable-wtmp],[enable wtmp (wtmpx) support (requires --enable-utmp)])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_wtmp=$enableval
  fi])

AC_ARG_ENABLE(lastlog,
  [AS_HELP_STRING([--enable-lastlog],[enable lastlog support (requires --enable-utmp)])],
  [if test x$enableval = xyes -o x$enableval = xno; then
    support_lastlog=$enableval
  fi])

if test x$support_utmp = xyes; then
  AC_DEFINE(UTMP_SUPPORT, 1, Define if you want to have utmp/utmpx support)
fi
if test x$support_wtmp = xyes; then
  AC_DEFINE(WTMP_SUPPORT, 1, Define if you want to have wtmp support when utmp/utmpx is enabled)
fi
if test x$support_lastlog = xyes; then
  AC_DEFINE(LASTLOG_SUPPORT, 1, Define if you want to have lastlog support when utmp/utmpx is enabled)
fi

AC_CHECK_FUNCS( \
	updwtmp \
	updwtmpx \
	updlastlogx \
)

AC_CHECK_HEADERS(lastlog.h)

dnl# --------------------------------------------------------------------------
dnl# DO ALL UTMP AND WTMP CHECKING
dnl# --------------------------------------------------------------------------
dnl# check for host field in utmp structure

dnl# --------------------------------------------
AC_CHECK_HEADERS(utmp.h,
[AC_CACHE_CHECK([for struct utmp], pt_cv_struct_utmp,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp ut;]])],[pt_cv_struct_utmp=yes],[pt_cv_struct_utmp=no])])
if test x$pt_cv_struct_utmp = xyes; then
  AC_DEFINE(HAVE_STRUCT_UTMP, 1, Define if utmp.h has struct utmp)
fi
]

AC_CACHE_CHECK(for ut_host in utmp struct, pt_cv_struct_utmp_host,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp ut; ut.ut_host;]])],[pt_cv_struct_utmp_host=yes],[pt_cv_struct_utmp_host=no])])
if test x$pt_cv_struct_utmp_host = xyes; then
  AC_DEFINE(HAVE_UTMP_HOST, 1, Define if struct utmp contains ut_host)
fi

AC_CACHE_CHECK(for ut_pid in utmp struct, pt_cv_struct_utmp_pid,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>]], [[struct utmp ut; ut.ut_pid;]])],[pt_cv_struct_utmp_pid=yes],[pt_cv_struct_utmp_pid=no])])
if test x$pt_cv_struct_utmp_pid = xyes; then
  AC_DEFINE(HAVE_UTMP_PID, 1, Define if struct utmp contains ut_pid)
fi
) dnl# AC_CHECK_HEADERS(utmp.h

dnl# --------------------------------------------

AC_CHECK_HEADERS(utmpx.h,
[AC_CACHE_CHECK([for struct utmpx], pt_cv_struct_utmpx,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmpx.h>]], [[struct utmpx ut;]])],[pt_cv_struct_utmpx=yes],[pt_cv_struct_utmpx=no])])
if test x$pt_cv_struct_utmpx = xyes; then
  AC_DEFINE(HAVE_STRUCT_UTMPX, 1, Define if utmpx.h has struct utmpx)
fi
]

AC_CACHE_CHECK(for host in utmpx struct, pt_cv_struct_utmpx_host,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmpx.h>]], [[struct utmpx utx; utx.ut_host;]])],[pt_cv_struct_utmpx_host=yes],[pt_cv_struct_utmpx_host=no])])
if test x$pt_cv_struct_utmpx_host = xyes; then
  AC_DEFINE(HAVE_UTMPX_HOST, 1, Define if struct utmpx contains ut_host)
fi

AC_CACHE_CHECK(for session in utmpx struct, pt_cv_struct_utmpx_session,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmpx.h>]], [[struct utmpx utx; utx.ut_session;]])],[pt_cv_struct_utmpx_session=yes],[pt_cv_struct_utmpx_session=no])])
if test x$pt_cv_struct_utmpx_session = xyes; then
  AC_DEFINE(HAVE_UTMPX_SESSION, 1, Define if struct utmpx contains ut_session)
fi
) dnl# AC_CHECK_HEADERS(utmpx.h

dnl# --------------------------------------------------------------------------
dnl# check for struct lastlog
AC_CACHE_CHECK(for struct lastlog, pt_cv_struct_lastlog,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmp.h>
#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#endif
]], [[struct lastlog ll;]])],[pt_cv_struct_lastlog=yes],[pt_cv_struct_lastlog=no])])
if test x$pt_cv_struct_lastlog = xyes; then
  AC_DEFINE(HAVE_STRUCT_LASTLOG, 1, Define if utmp.h or lastlog.h has struct lastlog)
fi

dnl# check for struct lastlogx
AC_CACHE_CHECK(for struct lastlogx, pt_cv_struct_lastlogx,
[AC_COMPILE_IFELSE([AC_LANG_PROGRAM([[#include <sys/types.h>
#include <utmpx.h>
#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#endif
]], [[struct lastlogx ll;]])],[pt_cv_struct_lastlogx=yes],[pt_cv_struct_lastlogx=no])])
if test x$pt_cv_struct_lastlogx = xyes; then
  AC_DEFINE(HAVE_STRUCT_LASTLOGX, 1, Define if utmpx.h or lastlog.h has struct lastlogx)
fi

dnl# --------------------------------------------------------------------------
dnl# FIND FILES
dnl# --------------------------------------------------------------------------

dnl# find utmp
PT_FIND_FILE([utmp], [UTMP_FILE], [_PATH_UTMP],
["/var/run/utmp", "/var/adm/utmp", "/etc/utmp", "/usr/etc/utmp", "/usr/adm/utmp"],[
#include <sys/types.h>
#include <utmp.h>
])

dnl# --------------------------------------------------------------------------

dnl# find wtmp
PT_FIND_FILE([wtmp], [WTMP_FILE], [_PATH_WTMP],
["/var/log/wtmp", "/var/adm/wtmp", "/etc/wtmp", "/usr/etc/wtmp", "/usr/adm/wtmp"],[
#include <sys/types.h>
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
])
dnl# --------------------------------------------------------------------------

dnl# find wtmpx
PT_FIND_FILE([wtmpx], [WTMPX_FILE], [_PATH_WTMPX],
["/var/log/wtmpx", "/var/adm/wtmpx"],[
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif
])
dnl# --------------------------------------------------------------------------

dnl# find lastlog
PT_FIND_FILE([lastlog], [LASTLOG_FILE], [_PATH_LASTLOG],
["/var/log/lastlog"],[
#include <sys/types.h>
#ifdef HAVE_UTMP_H
#include <utmp.h>
#endif
#ifdef HAVE_LASTLOG_H
#include <lastlog.h>
#endif
])
dnl# --------------------------------------------------------------------------

dnl# find lastlogx
PT_FIND_FILE([lastlogx], [LASTLOGX_FILE], [_PATH_LASTLOGX],
["/var/log/lastlogx", "/var/adm/lastlogx"],[
#ifdef HAVE_UTMPX_H
#include <utmpx.h>
#endif
])
])

AC_DEFUN([SCM_RIGHTS_CHECK],
[
AC_CACHE_CHECK(for unix-compliant filehandle passing ability, pt_cv_can_pass_fds,
[AC_LINK_IFELSE([AC_LANG_PROGRAM([[
#include <cstddef> // broken bsds (is that redundant?) need this
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/uio.h>
]], [[
{
  msghdr msg;
  iovec iov;
  char buf [100];
  char data = 0;

  iov.iov_base = &data;
  iov.iov_len  = 1;

  msg.msg_iov        = &iov;
  msg.msg_iovlen     = 1;
  msg.msg_control    = buf;
  msg.msg_controllen = sizeof buf;

  cmsghdr *cmsg = CMSG_FIRSTHDR (&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type  = SCM_RIGHTS;
  cmsg->cmsg_len   = 100;

  *(int *)CMSG_DATA (cmsg) = 5;

  return sendmsg (3, &msg, 0);
}
]])],[pt_cv_can_pass_fds=yes],[pt_cv_can_pass_fds=no])])
if test x$pt_cv_can_pass_fds = xyes; then
   AC_DEFINE(HAVE_UNIX_FDPASS, 1, Define if sys/socket.h defines the necessary macros/functions for file handle passing)
else
   AC_MSG_ERROR([libptytty requires unix-compliant filehandle passing ability])
fi
])

AC_DEFUN([TTY_GROUP_CHECK],
[
AC_CACHE_CHECK([for tty group], pt_cv_tty_group,
[AC_RUN_IFELSE([AC_LANG_SOURCE([[
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <grp.h>

int main()
{
  struct stat st;
  struct group *gr;
  char *tty;
  gr = getgrnam("tty");
  tty = ttyname(0);
  if (gr != 0
      && tty != 0
      && (stat(tty, &st)) == 0
      && st.st_gid == gr->gr_gid)
    return 0;
  else
    return 1;
}]])],[pt_cv_tty_group=yes],[pt_cv_tty_group=no],[pt_cv_tty_group=no])])
if test x$pt_cv_tty_group = xyes; then
  AC_DEFINE(TTY_GID_SUPPORT, 1, "")
fi])

