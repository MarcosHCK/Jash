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

void j_builtin_help (JRunner* runner, const gchar* parameter)
{
}

#define J_SET_CLOSURE_ERROR_SYSCALL(NAME,name) \
  void j_set_closure_error_##name (GError** error, int errno_value, const gchar* fmt, ...) \
    { \
      gchar* error_prefix; \
      GValue* error_value; \
      GError* error_pointer; \
      va_list l; \
 ; \
      va_start (l, fmt); \
 ; \
      error_prefix = g_strdup_vprintf (fmt, l); \
      error_pointer = g_error_new (J_CLOSURE_ERROR, J_CLOSURE_ERROR_##NAME, "%s (%s)", error_prefix, g_strerror (errno_value)); \
      error_value = g_value_init (j_closure_error_value (error_pointer), G_TYPE_INT); \
 ; \
      g_value_set_int (error_value, errno_value); \
      g_propagate_error (error, error_pointer); \
      g_free (error_prefix); \
    }

  J_SET_CLOSURE_ERROR_SYSCALL (CHDIR, chdir);
  J_SET_CLOSURE_ERROR_SYSCALL (DUP2, dup2);
  J_SET_CLOSURE_ERROR_SYSCALL (EXECVP, execvp);
  J_SET_CLOSURE_ERROR_SYSCALL (FORK, fork);
  J_SET_CLOSURE_ERROR_SYSCALL (OPEN, open);
  J_SET_CLOSURE_ERROR_SYSCALL (PIPE, pipe);
  J_SET_CLOSURE_ERROR_SYSCALL (WAITPID, waitpid);
#undef J_SET_CLOSURE_ERROR_SYSCALL

void j_set_closure_error_again (GError** error, const gchar* value)
{
  GValue* error_value;
  GError* error_pointer;

#if DEVELOPER == 1
  error_pointer = g_error_new (J_CLOSURE_ERROR, J_CLOSURE_ERROR_AGAIN, "again: %s", value);
#else // !DEVELOPER
  error_pointer = g_error_new_literal (J_CLOSURE_ERROR, J_CLOSURE_ERROR_EXIT, "again");
#endif // DEVELOPER
  error_value = j_closure_error_value (error_pointer);

  g_value_set_string (g_value_init (error_value, G_TYPE_STRING), value);
  g_propagate_error (error, error_pointer);
}

void j_set_closure_error_exit (GError** error, int value)
{
  GValue* error_value;
  GError* error_pointer;

#if DEVELOPER == 1
  error_pointer = g_error_new (J_CLOSURE_ERROR, J_CLOSURE_ERROR_EXIT, "exit %i", value);
#else // !DEVELOPER
  error_pointer = g_error_new_literal (J_CLOSURE_ERROR, J_CLOSURE_ERROR_EXIT, "exit");
#endif // DEVELOPER
  error_value = j_closure_error_value (error_pointer);

  g_value_set_int (g_value_init (error_value, G_TYPE_INT), value);
  g_propagate_error (error, error_pointer);
}
