/*----------------------------------------------------------------------*
 * File:	ptytty.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 1999-2001 Geoff Wing <gcw@pobox.com>
 * Copyright (c) 2004-2006 Marc Lehmann <schmorp@schmorp.de>
 * Copyright (c) 2006      Emanuele Giaquinta <emanuele.giaquinta@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *---------------------------------------------------------------------*/

#include "config.h"

#include "ptytty.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif
#ifdef HAVE_STROPTS_H
# include <stropts.h>
#endif
#if defined(HAVE_PTY_H)
# include <pty.h>
#elif defined(HAVE_LIBUTIL_H)
# include <libutil.h>
#elif defined(HAVE_UTIL_H)
# include <util.h>
#endif
#ifdef TTY_GID_SUPPORT
#include <grp.h>
#endif

#ifndef O_NOCTTY
# define O_NOCTTY 0
#endif

/////////////////////////////////////////////////////////////////////////////

/* ------------------------------------------------------------------------- *
 *                  GET PSEUDO TELETYPE - MASTER AND SLAVE                   *
 * ------------------------------------------------------------------------- */
/*
 * Returns pty file descriptor, or -1 on failure
 * If successful, ttydev is set to the name of the slave device.
 * fd_tty _may_ also be set to an open fd to the slave device
 */
#if defined(UNIX98_PTY)

  static int
  get_pty (int *fd_tty, char **ttydev)
  {
    int pfd;

# if defined(HAVE_GETPT)
    pfd = getpt ();
# elif defined(HAVE_POSIX_OPENPT)
    pfd = posix_openpt (O_RDWR | O_NOCTTY);
# else
#  ifdef _AIX
    pfd = open ("/dev/ptc", O_RDWR | O_NOCTTY, 0);
#  else
    pfd = open ("/dev/ptmx", O_RDWR | O_NOCTTY, 0);
#  endif
# endif

    if (pfd >= 0)
      {
        if (grantpt (pfd) == 0	/* change slave permissions */
            && unlockpt (pfd) == 0)
          {
            /* slave now unlocked */
            *ttydev = strdup (ptsname (pfd));	/* get slave's name */
            return pfd;
          }

        close (pfd);
      }

    return -1;
  }

#elif defined(HAVE_OPENPTY)

  static int
  get_pty (int *fd_tty, char **ttydev)
  {
    int pfd;
    int res;

    res = openpty (&pfd, fd_tty, NULL, NULL, NULL);

    if (res != -1)
      {
        *ttydev = strdup (ttyname (*fd_tty));
        return pfd;
      }

    return -1;
  }

#elif defined(HAVE__GETPTY)

  static int
  get_pty (int *fd_tty, char **ttydev)
  {
    int pfd;
    char *slave;

    slave = _getpty (&pfd, O_RDWR | O_NOCTTY, 0622, 0);

    if (slave != NULL)
      {
        *ttydev = strdup (slave);
        return pfd;
      }

    return -1;
  }

#else
# define SET_TTY_OWNER

  /* Based on the code in openssh/openbsd-compat/bsd-openpty.c */
  static int
  get_pty (int *fd_tty, char **ttydev)
  {
    int pfd;
    int i;
    char pty_name[32];
    char tty_name[32];
    const char *majors = "pqrstuvwxyzabcde";
    const char *minors = "0123456789abcdef";

    for (i = 0; i < 256; i++)
      {
        snprintf (pty_name, 32, "/dev/pty%c%c", majors[i / 16], minors[i % 16]);
        snprintf (tty_name, 32, "/dev/tty%c%c", majors[i / 16], minors[i % 16]);

        if ((pfd = open (pty_name, O_RDWR | O_NOCTTY, 0)) == -1)
          {
            snprintf (pty_name, 32, "/dev/ptyp%d", i);
            snprintf (tty_name, 32, "/dev/ttyp%d", i);
            if ((pfd = open (pty_name, O_RDWR | O_NOCTTY, 0)) == -1)
              continue;
          }

        if (access (tty_name, R_OK | W_OK) == 0)
          {
            *ttydev = strdup (tty_name);
            return pfd;
          }

        close (pfd);
      }

    return -1;
  }

#endif

/*----------------------------------------------------------------------*/
/*
 * Returns tty file descriptor, or -1 on failure
 */
static int
get_tty (char *ttydev)
{
  return open (ttydev, O_RDWR | O_NOCTTY, 0);
}

/*----------------------------------------------------------------------*/
/*
 * Make our tty a controlling tty so that /dev/tty points to us
 */
static int
control_tty (int fd_tty)
{
  int fd;

  setsid ();

#ifdef TIOCSCTTY
  ioctl (fd_tty, TIOCSCTTY, NULL);
#else
  fd = open (ttyname (fd_tty), O_RDWR);
  if (fd >= 0)
    close (fd);
#endif

  fd = open ("/dev/tty", O_WRONLY);
  if (fd < 0)
    return -1; /* fatal */

  close (fd);

  return 0;
}

void
ptytty::close_tty ()
{
  if (tty < 0)
    return;

  close (tty);
  tty = -1;
}

bool
ptytty::make_controlling_tty ()
{
  return control_tty (tty) >= 0;
}

void
ptytty::set_utf8_mode (bool on)
{
#ifdef IUTF8
  if (pty < 0)
    return;

  struct termios tio;

  if (tcgetattr (pty, &tio) != -1)
    {
      tcflag_t new_cflag = tio.c_iflag;

      if (on)
        new_cflag |= IUTF8;
      else
        new_cflag &= ~IUTF8;

      if (new_cflag != tio.c_iflag)
        {
          tio.c_iflag = new_cflag;
          tcsetattr (pty, TCSANOW, &tio);
        }
    }
#endif
}

static struct ttyconf
{
  gid_t gid;
  mode_t mode;

  ttyconf ()
  {
#ifdef TTY_GID_SUPPORT
    struct group *gr = getgrnam ("tty");

    if (gr)
      {
        /* change group ownership of tty to "tty" */
        mode = S_IRUSR | S_IWUSR | S_IWGRP;
        gid = gr->gr_gid;
      }
    else
#endif /* TTY_GID_SUPPORT */
      {
        mode = S_IRUSR | S_IWUSR | S_IWGRP | S_IWOTH;
        gid = 0;
      }
  }
} ttyconf;

ptytty_unix::ptytty_unix ()
{
  name = 0;
#if UTMP_SUPPORT
  cmd_pid = 0;
#endif
}

ptytty_unix::~ptytty_unix ()
{
#if UTMP_SUPPORT
  if (cmd_pid)
    log_session (false, 0);
#endif
  put ();
}

void
ptytty_unix::put ()
{
  if (name)
    {
      chmod (name, RESTORE_TTY_MODE);
      chown (name, 0, ttyconf.gid);
    }

  close_tty ();

  if (pty >= 0)
    close (pty);

  free (name);

  pty = tty = -1;
  name = 0;
}

bool
ptytty_unix::get ()
{
  /* get master (pty) */
  if ((pty = get_pty (&tty, &name)) < 0)
    return false;

  /* get slave (tty) */
  if (tty < 0)
    {
#ifdef SET_TTY_OWNER
      chown (name, getuid (), ttyconf.gid);      /* fail silently */
      chmod (name, ttyconf.mode);
# ifdef HAVE_REVOKE
      revoke (name);
# endif
#endif

      if ((tty = get_tty (name)) < 0)
        {
          put ();
          return false;
        }
    }

#if defined(I_PUSH)
  /*
   * Push STREAMS modules:
   *    ptem: pseudo-terminal hardware emulation module.
   *    ldterm: standard terminal line discipline.
   *    ttcompat: V7, 4BSD and XENIX STREAMS compatibility module.
   *
   * On Solaris, a process can acquire a controlling terminal in the
   * following ways:
   * - open() of /dev/ptmx or of a slave device without O_NOCTTY
   * - I_PUSH ioctl() of the "ptem" or "ldterm" module on a slave device
   * The second case is problematic, because it cannot be disabled.
   * Fortunately, Solaris (10 and 11 at least) provides an undocumented
   * __IPUSH_NOCTTY ioctl which does not have this side-effect, so we
   * use it if defined. See
   * https://github.com/illumos/illumos-gate/blob/19d32b9ab53d17ac6605971e14c45a5281f8d9bb/usr/src/uts/common/os/streamio.c#L3755
   * https://github.com/illumos/illumos-gate/blob/19d32b9ab53d17ac6605971e14c45a5281f8d9bb/usr/src/uts/common/io/ptem.c#L203
   * https://github.com/illumos/illumos-gate/blob/19d32b9ab53d17ac6605971e14c45a5281f8d9bb/usr/src/uts/common/io/ldterm.c#L794
   * Note that an open() of a slave device autoloads the modules,
   * with __I_PUSH_NOCTTY, if xpg[46] mode is enabled (which requires
   * linking /usr/lib/values-xpg[46].o).
   * https://github.com/illumos/illumos-gate/blob/19d32b9ab53d17ac6605971e14c45a5281f8d9bb/usr/src/lib/libc/port/sys/open.c#L173
   */

#ifdef __I_PUSH_NOCTTY
# define PT_I_PUSH __I_PUSH_NOCTTY
#else
# define PT_I_PUSH I_PUSH
#endif

#if defined(HAVE_ISASTREAM) && defined(HAVE_STROPTS_H)
  if (isastream (tty) == 1)
# endif
    {
      if (!ioctl (tty, I_FIND, "ptem"))
        ioctl (tty, PT_I_PUSH, "ptem");

      if (!ioctl (tty, I_FIND, "ldterm"))
        ioctl (tty, PT_I_PUSH, "ldterm");

      if (!ioctl (tty, I_FIND, "ttcompat"))
        ioctl (tty, PT_I_PUSH, "ttcompat");
    }
#endif

#if defined(USE_UTMP) && !defined(HAVE_UTMP_PID)
  int fd_stdin = dup (STDIN_FILENO);
  dup2 (tty, STDIN_FILENO);

  utmp_pos = ttyslot ();

  dup2 (fd_stdin, STDIN_FILENO);
  close (fd_stdin);
#endif

  return true;
}

void
ptytty_unix::login (int cmd_pid, bool login_shell, const char *hostname)
{
#if UTMP_SUPPORT
  if (!name || !*name)
    return;

  this->cmd_pid     = cmd_pid;
  this->login_shell = login_shell;

  log_session (true, hostname);
#endif
}
