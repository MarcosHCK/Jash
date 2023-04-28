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
#ifndef __JASH_CODEGEN_EXTERNS__
#define __JASH_CODEGEN_EXTERNS__ 1
#include <glib.h>
#include <unistd.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

  G_GNUC_INTERNAL void j_chdir (const gchar* path, GError** error);
  G_GNUC_INTERNAL void j_dup2 (gint fd_old, gint fd_new, GError** error);
  G_GNUC_INTERNAL void j_execvp (const gchar* program, gchar* const arguments [], GError** error);
  G_GNUC_INTERNAL pid_t j_fork (GError** error);
  G_GNUC_INTERNAL guint j_invoke_get_open_flags (gint fileno, gboolean append);
  G_GNUC_INTERNAL guint j_invoke_get_open_mode (gint fileno, gboolean append);
  G_GNUC_INTERNAL gint j_open (const gchar* filename, gint flags, gint mode, GError** error);
  G_GNUC_INTERNAL gint j_parse_int (const gchar* value, GError** error);
  G_GNUC_INTERNAL void j_pipe_clear_many (JPipe* pipes, guint n_pipes);
  G_GNUC_INTERNAL void j_pipe_init_many (JPipe* pipes, guint n_pipes, GError** error);
  G_GNUC_INTERNAL void j_waitpid (pid_t pid, gint* status_code, gint flags, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_CODEGEN_EXTERNS__
