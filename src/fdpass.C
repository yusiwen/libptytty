/*----------------------------------------------------------------------*
 * File:	fdpass.C
 *----------------------------------------------------------------------*
 *
 * All portions of code are copyright by their respective author/s.
 * Copyright (c) 2005-2006,2012 Marc Lehmann <schmorp@schmorp.de>
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
 *----------------------------------------------------------------------*/

#include "config.h"

#include <stddef.h> // needed by broken bsds for NULL used in sys/uio.h
#include <stdlib.h>

#include <unistd.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>

#include "libptytty.h"

// CMSG_SPACE & CMSG_LEN are rfc2292 extensions to unix
#ifndef CMSG_SPACE
# define CMSG_SPACE(len) (sizeof (cmsghdr) + len)
#endif

#ifndef CMSG_LEN
# define CMSG_LEN(len) (sizeof (cmsghdr) + len)
#endif

bool
ptytty::send_fd (int socket, int fd)
{
  void *buf = malloc (CMSG_SPACE (sizeof (int)));

  if (!buf)
    return 0;

  msghdr msg;
  iovec iov;
  cmsghdr *cmsg;
  char data = 0;

  iov.iov_base = &data;
  iov.iov_len  = 1;

  msg.msg_name       = 0;
  msg.msg_namelen    = 0;
  msg.msg_iov        = &iov;
  msg.msg_iovlen     = 1;
  msg.msg_control    = buf;
  msg.msg_controllen = CMSG_SPACE (sizeof (int));

  cmsg = CMSG_FIRSTHDR (&msg);
  cmsg->cmsg_level = SOL_SOCKET;
  cmsg->cmsg_type  = SCM_RIGHTS;
  cmsg->cmsg_len   = CMSG_LEN (sizeof (int));

  *(int *)CMSG_DATA (cmsg) = fd;

  ssize_t result = sendmsg (socket, &msg, 0);

  free (buf);

  return result >= 0;
}

int
ptytty::recv_fd (int socket)
{
  void *buf = malloc (CMSG_SPACE (sizeof (int)));

  if (!buf)
    return -1;

  msghdr msg;
  iovec iov;
  char data = 1;

  iov.iov_base = &data;
  iov.iov_len  = 1;

  msg.msg_name       = 0;
  msg.msg_namelen    = 0;
  msg.msg_iov        = &iov;
  msg.msg_iovlen     = 1;
  msg.msg_control    = buf;
  msg.msg_controllen = CMSG_SPACE (sizeof (int));

  int fd = -1;

  if (recvmsg (socket, &msg, 0) > 0)
    {
      // there *should* be at most one cmsg, as more should not fit
      // in the buffer, although this depends on somewhat sane
      // alignment imposed by CMSG_SPACE.
      cmsghdr *cmsg = CMSG_FIRSTHDR (&msg);

      if (cmsg)
        {
          // some operating systems (i.e. osx) allow msg->cmsg_len to be larger than
          // msg.msg_controllen, so limit the size here
          if (cmsg->cmsg_len > CMSG_SPACE (sizeof (int)))
            cmsg->cmsg_len = CMSG_SPACE (sizeof (int));

          if (   cmsg->cmsg_level == SOL_SOCKET
              && cmsg->cmsg_type  == SCM_RIGHTS
              && cmsg->cmsg_len   >= CMSG_LEN (sizeof (int)))
            {
              // close any extra fds that might have been passed.
              // this does not work around osx/freebsd bugs where a malicious sender
              // can send us more fds than we can receive, leaking the extra fds,
              // which must be fixed in the kernel, really.
              for (fd = 1; cmsg->cmsg_len >= CMSG_LEN (sizeof (int) * (fd + 1)); ++fd)
                close (((int *)CMSG_DATA (cmsg))[fd]);

              // we are only interested in the first fd
              fd = *(int *)CMSG_DATA (cmsg);
            }
        }
    }

  free (buf);

  if (data != 0)
    {
      close (fd);
      fd = -1;
    }

  return fd;
}

