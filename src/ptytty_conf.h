/*
 * This file contains a few tunables for libptytty. Normally there should
 * be little reason to change this file. It is provided to suit rare or
 * special cases only.
 */

/*
 * Default mode to restore when releasing the PTS device. It is relaxed to be
 * compatible with most systems, change it to a more secure value if your
 * system supports it (0640 for example).
 */
#define RESTORE_TTY_MODE 0666

/*
 * Define if you want to use a separate process for pty/tty handling
 * when running setuid/setgid. You need this when making it setuid/setgid.
 */
#define PTYTTY_HELPER 1

/*
 * Define if you want to use a single helper process form multiple threads
 * OR forked processes. Without it, the helper will only work from the
 * process having started it, and it might not be possible to start another
 * helper.
 */
#define PTYTTY_REENTRANT 1

/*
 * Provide a STL-like vector class and find algorithm.
 * The default below fine for normal C++ environments.
 */
#include <vector>
#include <algorithm>
using namespace std;

/*
 * printf-like functions to be called on fatal conditions
 * (must exit), or warning conditions (only print message)
 * TODO move elsewhere, e.g. util.C
 */
#define ptytty_fatal(msg) do { ptytty_warn (msg); _exit (1); } while (0)
#define ptytty_warn(msg) fprintf (stderr, msg) // TODO
