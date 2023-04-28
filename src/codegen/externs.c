/* Copyright 2023 MarcosHCK
 * This file is part of JASH.
 *
 * JASH is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * JASH is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with JASH. If not, see <http://www.gnu.org/licenses/>.
 */
#include <config.h>
#include <codegen/codegen.h>
#include <codegen/context.h>
#include <codegen/externs.h>
#include <codegen/walker.h>
#include <errno.h>
#include <fcntl.h>
#include <glib/gstdio.h>
#include <glib-object.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <wait.h>

void j_chdir (const gchar* path, GError** error)
{
  if (g_chdir (path) < 0)
    j_set_closure_error_chdir (error, errno, "chdir (\"%s\")!", path);
}

void j_dup2 (gint fd_old, gint fd_new, GError** error)
{
  if (dup2 (fd_old, fd_new) < 0)
#if DEVELOPER == 1
    j_set_closure_error_dup2 (error, errno, "dup2 (%i, %i)!", fd_old, fd_new);
#else // DEVELOPER
    j_set_closure_error_dup2 (error, errno, "dup2 ()!");
#endif // DEVELOPER
}

void j_execvp (const gchar* program, gchar* const arguments [], GError** error)
{
  execvp (program, arguments);
  j_set_closure_error_execvp (error, errno, "exec (\"%s\")!", program);
}

pid_t j_fork (GError** error)
{
  pid_t pid;
  if ((pid = fork ()) < 0)
    j_set_closure_error_fork (error, errno, "fork ()!");
return pid;
}

guint j_invoke_get_open_flags (gint fileno, gboolean append)
{
  guint flags = 0;

  switch (fileno)
  {
    default: g_assert_not_reached ();
    case STDIN_FILENO: flags = O_RDONLY; break;
    case STDOUT_FILENO: flags = O_WRONLY;
      {
        if (append)
          flags |= O_APPEND;
        else
          flags |= O_CREAT | O_TRUNC;
        break;
      }
  }
return (flags);
}

guint j_invoke_get_open_mode (gint fileno, gboolean append)
{
  return (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
}

gint j_open (const gchar* filename, gint flags, gint mode, GError** error)
{
  int fd;
  if ((fd = g_open (filename, flags, mode)) < 0)
    j_set_closure_error_open (error, errno, "open (\"%s\")!", filename);
return fd;
}

void j_pipe_clear_many (JPipe* pipes, guint n_pipes)
{
  guint i;

  for (i = 0; i < n_pipes; ++i)
    {
      g_close (pipes [i] [0], NULL);
      g_close (pipes [i] [1], NULL);
    }
}

void j_pipe_init_many (JPipe* pipes, guint n_pipes, GError** error)
{
  guint i;

  for (i = 0; i < n_pipes; ++i)
  if (pipe (pipes [i]) < 0)
    {
      j_set_closure_error_pipe (error, errno, "pipe ()!");
      break;
    }
}

static gint detectbase (const gchar* value, const gchar** begin)
{
  if (g_utf8_get_char (value) == (gunichar) '-')
    return detectbase (g_utf8_next_char (value), begin);
  else
    {
      gchar* n = g_utf8_next_char (value);
      gunichar c1 = g_utf8_get_char (value);
      gunichar c2 = g_utf8_get_char (n);

      if (c1 != (gunichar) '0')
        return (*begin = value, 10);
      else
        {
          switch (c2)
            {
              case (gunichar) 'b': return (*begin = g_utf8_next_char (n), 2);
              case (gunichar) 'o': return (*begin = g_utf8_next_char (n), 8);
              case (gunichar) 'd': return (*begin = g_utf8_next_char (n), 10);
              case (gunichar) 'x': return (*begin = g_utf8_next_char (n), 16);
              default: return (*begin = value, 10);
            }
        }
    }
}

gint j_parse_int (const gchar* value, GError** error)
{
  const gchar* number = NULL;
  const gint base = detectbase (value, &number);
  gint64 result = -1;

  g_ascii_string_to_signed (number, base, G_MININT, G_MAXINT, &result, error);
return (gint) result;
}

void j_waitpid (pid_t pid, gint* status_code, gint flags, GError** error)
{
  if (waitpid (pid, status_code, flags) < 0)
#if DEVELOPER == 1
    j_set_closure_error_waitpid (error, errno, "waitpid (%i)!", pid);
#else // DEVELOPER
    j_set_closure_error_waitpid (error, errno, "waitpid ()!");
#endif // DEVELOPER
}
