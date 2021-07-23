#ifndef PTYTTY_H
#define PTYTTY_H

#include "libptytty.h"
#include "ptytty_conf.h"

#include <sys/types.h>

#if UTMP_SUPPORT
# if defined(HAVE_STRUCT_UTMPX) && !defined(__GLIBC__)
#  define USE_UTMPX
# elif defined(HAVE_STRUCT_UTMP)
#  define USE_UTMP
# else
#  error cannot build with utmp support - no utmp or utmpx struct found
# endif
#endif

struct ptytty_unix : ptytty
{
private:

  char *name;
#if UTMP_SUPPORT
  int utmp_pos;
  int cmd_pid;
  bool login_shell;
#endif

  void log_session (bool login, const char *hostname);
  void put ();

public:

  ptytty_unix ();
  ~ptytty_unix ();

  bool get ();
  void login (int cmd_pid, bool login_shell, const char *hostname);
};

#endif

