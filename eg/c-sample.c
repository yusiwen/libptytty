// gcc -I../src -L../src/.libs c-sample.c -lptytty
//
// if your C compiler can't grok this, it isn't a C compiler
//
// on my system, this program outputs:
//
// who; echo 'Hello, World!'; exit
// # root     pts/9        Jan 25 12:12 (libptytty.example.net)
// Hello, World!
//

#include <stdio.h>
#include <stdlib.h>

#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

#include <libptytty.h>

int main (void)
{
  ptytty_init ();

  PTYTTY pty = ptytty_create ();

  if (!ptytty_get (pty))
    printf ("unable to open pty\n"), exit (EXIT_FAILURE);

  pid_t pid = fork ();

  if (pid < 0)
    printf ("fork error\n"), exit (EXIT_FAILURE);

  int pty_fd = ptytty_pty (pty);
  int tty_fd = ptytty_tty (pty);

  if (pid)
    {
      fcntl (pty_fd, F_SETFL, 0); // might be in non-blocking mode
      ptytty_close_tty (pty);
      ptytty_login (pty, pid, 1, "libptytty.example.net");

      char s[] = "who; echo 'Hello, World!'; exit\015";

      write (pty_fd, s, sizeof (s) - 1);

      char buf[1024];

      for (;;)
        {
          int len = read (pty_fd, buf, 1024);

          if (len <= 0)
            break;

          write (STDOUT_FILENO, buf, len);
        }

      ptytty_delete (pty);
    }
  else
    {
      ptytty_make_controlling_tty (pty);

      close (pty_fd);
      dup2 (tty_fd, STDIN_FILENO );
      dup2 (tty_fd, STDOUT_FILENO);
      dup2 (tty_fd, STDERR_FILENO);
      close (tty_fd);

      execl ("/bin/sh", "-sh", 0);
      _exit (EXIT_FAILURE);
    }
  
  return EXIT_SUCCESS;
}


