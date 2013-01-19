/*
  Copyright (C) 2012 Tomash Brechko.  All rights reserved.

  This header is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This header is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this header.  If not, see <http://www.gnu.org/licenses/>.


  DESCRIPTION:

  Define macros for error checking and reporting:

    warn(format, ...);
      Call dprintf(STDERR_FILENO, format, ...) prepending program name
      plus colon and space, and appending newline.  Return the result
      of dprintf().

    error(format, ...);
      Call warn() then exit(EXIT_FAILURE).  Do not return.

    die(format, ...);
      Call warn() prepending source file name and line number, then
      _Exit(EXIT_FAILURE).  Do not return.

    SYS(expr);
    SYS(expr, format, ...);
      Evaluate expression 'expr' (normally a system call).  If 'expr'
      yields -1 then call die() and do not return (when 'format, ...'
      is provided it will be appended to the message).  Otherwise
      return the result of 'expr'.  For instance:

        int fd = SYS(open(path, O_RDONLY));

      will set fd to file descriptor if open() succeeded, or will
      die() with the message like

        program: source.c:123: open(path, O_RDONLY) == -1: No such file or directory

      Passing 'format, ...' as in

        int fd = SYS(open(path, O_RDONLY), ": %s", path);

      will produce

        program: source.c:123: open(path, O_RDONLY) == -1: No such file or directory: /no/such/path

      Be aware that not all system calls return -1 on error.  Check
      manual page before using SYS().

    MEM(expr);
    MEM(expr, format, ...);
      Like SYS(), but check for NULL.  For instance to die() on failed
      realloc() it is safe to write

        ptr = MEM(realloc(ptr, size));


    POSIX(expr);
    POSIX(expr, format, ...);
      Like SYS(), but expect 'expr' to return error value directly
      instead of setting 'errno'.  Do not return when 'expr' yields
      non-zero.  For instance

        POSIX(pthread_key_create(&key, NULL));


    CHECK(expr, cond, action, format, ...);
      Generic macro that implements SYS() and MEM().  May be used like

        void *ptr = CHECK(mmap(NULL, length, PROT_READ | PROT_WRITE,
                               MAP_PRIVATE | MAP_ANONYMOUS, -1, 0),
                          == MAP_FAILED, die, "%m (length=%zu)", length);

      (note the comma that separates arguments 'expr' (mmap()) and
      'cond' (== MAP_FAILED)).

    ERRNO(expr, action, format, ...);
      Generic macro that implements POSIX().  When 'expr' yields
      non-zero set global 'errno' to the returned value and execute
      'action' passing 'format, ...' to it.  If 'action' returns then
      old value of 'errno' is restored.  May be used like

        ERRNO(posix_fallocate(fd, 0, size), warn, "%m: %zu", (size_t) size);


    RESTART(expr);
      Evaluate 'expr' (normally blocking system call).  When 'expr'
      yields -1 and errno == EINTR the expression is re-evaluated.
      Otherwise return the result of 'expr'.  For instance

        SYS(RESTART(connect(sock, addr, sizeof(addr))));

      Note that the reverse nesting RESTART(SYS(...)) is meaningless
      as SYS() will die on EINTR.

    char *program_invocation_short_name;
      Program name (argv[0]) with path prefix up to last '/' removed.

  Defining KROKI_ERROR_NOPOLLUTE will result in omitting alias
  definitions, but functionality will still be available with the
  namespace prefix 'kroki_'.

  Implementation requires GCC and Glibc.
*/

#ifndef KROKI_ERROR_NOPOLLUTE

#define warn(format, ...)  kroki_warn(format, ## __VA_ARGS__)
#define error(format, ...)  kroki_error(format, ## __VA_ARGS__)
#define die(format, ...)  kroki_die(format, ## __VA_ARGS__)
#define SYS(expr, ...)  _KROKI_SYS(expr, #expr, ## __VA_ARGS__)
#define MEM(expr, ...)  _KROKI_MEM(expr, #expr, ## __VA_ARGS__)
#define POSIX(expr, ...)  _KROKI_POSIX(expr, #expr, ## __VA_ARGS__)
#define CHECK(expr, cond, action, format, ...)                          \
  _KROKI_CHECK(expr, #expr, cond, #cond, action, format, ## __VA_ARGS__)
#define ERRNO(expr, action, format, ...)                        \
  _KROKI_ERRNO(expr, #expr, action, format, ## __VA_ARGS__)
#define RESTART(expr)  KROKI_RESTART(expr)

#endif  /* ! KROKI_ERROR_NOPOLLUTE */


#ifndef KROKI_ERROR_H
#define KROKI_ERROR_H 1

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>


extern char *program_invocation_short_name;


#define kroki_warn(format, ...)                                 \
  dprintf(STDERR_FILENO, "%s: " format "\n",                    \
          program_invocation_short_name, ## __VA_ARGS__)

#define kroki_error(format, ...)                \
  do                                            \
    {                                           \
      kroki_warn(format, ## __VA_ARGS__);       \
      exit(EXIT_FAILURE);                       \
    }                                           \
  while (0)

#define kroki_die(format, ...)                                          \
  do                                                                    \
    {                                                                   \
      kroki_warn("%s:%d: " format, __FILE__, __LINE__, ## __VA_ARGS__); \
      _Exit(EXIT_FAILURE);                                              \
    }                                                                   \
  while (0)

#define KROKI_SYS(expr, ...)                    \
  _KROKI_SYS(expr, #expr, ## __VA_ARGS__)

#define KROKI_MEM(expr, ...)                    \
  _KROKI_MEM(expr, #expr, ## __VA_ARGS__)

#define KROKI_POSIX(expr, ...)                  \
  _KROKI_POSIX(expr, #expr, ## __VA_ARGS__)

#define KROKI_CHECK(expr, cond, action, format, ...)                    \
  _KROKI_CHECK(expr, #expr, cond, #cond, action, format, ## __VA_ARGS__)

#define KROKI_ERRNO(expr, action, format, ...)                  \
  _KROKI_ERRNO(expr, #expr, action, format, ## __VA_ARGS__)

#define KROKI_RESTART(expr)                                     \
  ({                                                            \
    __typeof__(expr) _kroki_res;                                \
    do                                                          \
      _kroki_res = (expr);                                      \
    while (__builtin_expect(_kroki_res == -1 && errno == EINTR, \
                            0));                                \
    _kroki_res;                                                 \
  })


#define _KROKI_SYS(expr, exprstr, ...)          \
  _KROKI_CHECK(expr, exprstr, == -1, "== -1",   \
               kroki_die, "%m" __VA_ARGS__)

#define _KROKI_MEM(expr, exprstr, ...)                  \
  _KROKI_CHECK(expr, exprstr, == NULL, "== NULL",       \
               kroki_die, "%m" __VA_ARGS__)

#define _KROKI_POSIX(expr, exprstr, ...)        \
  _KROKI_ERRNO(expr, exprstr,                   \
               kroki_die, "%m" __VA_ARGS__)

#define _KROKI_CHECK(expr, exprstr, cond, condstr, action, format, ...) \
  ({                                                                    \
    __typeof__(expr) _kroki_res = (expr);                               \
    if (__builtin_expect(!! (_kroki_res cond), 0))                      \
      action("%s %s: " format, exprstr, condstr, ## __VA_ARGS__);       \
    _kroki_res;                                                         \
  })

#define _KROKI_ERRNO(expr, exprstr, action, format, ...)                \
  ({                                                                    \
    int _kroki_err = (expr);                                            \
    if (__builtin_expect(_kroki_err != 0, 0))                           \
      {                                                                 \
        int save_errno = errno;                                         \
        errno = _kroki_err;                                             \
        action("%s %s: " format, exprstr, "!= 0", ## __VA_ARGS__);      \
        errno = save_errno;                                             \
      }                                                                 \
    0;                                                                  \
  })


#endif  /* ! KROKI_ERROR_H */
