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
#ifndef __JASH_TERM_READLINE__
#define __JASH_TERM_READLINE__ 1
#include <glib-object.h>

#define J_TYPE_READLINE (j_readline_get_type ())
#define J_READLINE(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), J_TYPE_READLINE, JReadline))
#define J_IS_READLINE(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), J_TYPE_READLINE))
typedef struct _JReadline JReadline;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  G_GNUC_INTERNAL GType j_readline_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL JReadline* j_readline_new ();
  G_GNUC_INTERNAL gchar* j_readline_get (JReadline* readline);
  G_GNUC_INTERNAL gboolean j_readline_get_signaled (JReadline* readline);
  G_GNUC_INTERNAL void j_readline_history_add (JReadline* readline, const gchar* line);
  G_GNUC_INTERNAL const gchar* j_readline_history_get (JReadline* readline);
  G_GNUC_INTERNAL const gchar* j_readline_history_get_nth (JReadline* readline, guint nth);
  G_GNUC_INTERNAL void j_readline_history_load (JReadline* readline, GError** error);
  G_GNUC_INTERNAL void j_readline_history_save (JReadline* readline, GError** error);
  G_GNUC_INTERNAL void j_readline_history_print (JReadline* readline);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_TERM_READLINE__
