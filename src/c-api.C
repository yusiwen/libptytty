/*----------------------------------------------------------------------*
 * File:	c-api.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005      Marc Lehmann <pcg@goof.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
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

#ifndef PTYTTY_NO_C_API

typedef void *PTYTTY;

#define DEFINE_METHOD(retval, name, args1, args2) \
extern "C" retval ptytty_ ## name args1           \
{ return ((struct ptytty *)ptytty)->name args2; }

DEFINE_METHOD(int,pty,(PTYTTY ptytty),)
DEFINE_METHOD(int,tty,(PTYTTY ptytty),)
DEFINE_METHOD(int,get,(PTYTTY ptytty),())
DEFINE_METHOD(void,login,(PTYTTY ptytty, int cmd_pid, int login_shell, const char *hostname),(cmd_pid,login_shell,hostname))

DEFINE_METHOD(void,close_tty,(PTYTTY ptytty),())
DEFINE_METHOD(int,make_controlling_tty,(PTYTTY ptytty),())
DEFINE_METHOD(void,set_utf8_mode,(PTYTTY ptytty, int on),(on))

#define DEFINE_STATIC(retval, name, args) \
extern "C" retval ptytty_ ## name args           \
{ return ptytty::name args; }

DEFINE_STATIC(void,drop_privileges,())
DEFINE_STATIC(void,use_helper,())
DEFINE_STATIC(void,sanitise_stdfd,())
DEFINE_STATIC(void,init,())

DEFINE_STATIC(PTYTTY,create,())

extern "C" void ptytty_delete (PTYTTY ptytty)
{
  delete (struct ptytty *)ptytty;
}

// send_fd, recv_fd not exposed

#endif

