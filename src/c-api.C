/*----------------------------------------------------------------------*
 * File:	c-api.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005      Marc Lehmann <pcg@goof.com>
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

/////////////////////////////////////////////////////////////////////////////
// C API

#if PTYTTY_C_API

typedef void *PTYTTY;

extern "C"
{
  int
  ptytty_pty (PTYTTY ptytty)
  {
    return ((struct ptytty *)ptytty)->pty;
  }

  int
  ptytty_tty (PTYTTY ptytty)
  {
    return ((struct ptytty *)ptytty)->tty;
  }

  int
  ptytty_get (PTYTTY ptytty)
  {
    return ((struct ptytty *)ptytty)->get ();
  }

  void
  ptytty_login (PTYTTY ptytty, int cmd_pid, int login_shell, const char *hostname)
  {
    return ((struct ptytty *)ptytty)->login (cmd_pid, login_shell, hostname);
  }

  void
  ptytty_close_tty (PTYTTY ptytty)
  {
    return ((struct ptytty *)ptytty)->close_tty ();
  }

  int
  ptytty_make_controlling_tty (PTYTTY ptytty)
  {
    return ((struct ptytty *)ptytty)->make_controlling_tty ();
  }

  void
  ptytty_set_utf8_mode (PTYTTY ptytty, int on)
  {
    return ((struct ptytty *)ptytty)->set_utf8_mode (on);
  }

  void
  ptytty_sanitise_stdfd ()
  {
    return ptytty::sanitise_stdfd ();
  }

  void
  ptytty_init ()
  {
    return ptytty::init ();
  }

  PTYTTY
  ptytty_create ()
  {
    return ptytty::create ();
  }

  void
  ptytty_delete (PTYTTY ptytty)
  {
    delete (struct ptytty *)ptytty;
  }

  void
  ptytty_drop_privileges ()
  {
    return ptytty::drop_privileges ();
  }

#if PTYTTY_HELPER
  void
  ptytty_use_helper ()
  {
    return ptytty::use_helper ();
  }
#endif
}

// send_fd, recv_fd not exposed

#endif
