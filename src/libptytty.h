#ifndef LIBPTYTTY_H_ /* public libptytty header file */
#define LIBPTYTTY_H_

struct ptytty {
  int pty; // pty file descriptor; connected to rxvt
  int tty; // tty file descriptor; connected to child

  virtual ~ptytty ()
  {
  }

  virtual bool get () = 0;
  virtual void login (int cmd_pid, bool login_shell, const char *hostname) = 0;

  void close_tty ();
  bool make_controlling_tty ();
  void set_utf8_mode (bool on);

protected:
  ptytty ()
  : pty(-1), tty(-1)
  {
  }
};

ptytty *ptytty_new (); // create a new pty object
void ptytty_server (); // start the ptytty server process

#endif

