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
#ifndef __JASH_RUNTIME_RUNNER__
#define __JASH_RUNTIME_RUNNER__ 1
#include <glib-object.h>

#define J_TYPE_RUNNER (j_runner_get_type ())
#define J_RUNNER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), J_TYPE_RUNNER, JRunner))
#define J_IS_RUNNER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), J_TYPE_RUNNER))
typedef struct _JRunner JRunner;

#if __cplusplus
extern "C" {
#endif // __cplusplus

  G_GNUC_INTERNAL GType j_runner_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL JRunner* j_runner_new (gboolean interactive);
  G_GNUC_INTERNAL gboolean j_runner_get_interactive (JRunner* runner);
  G_GNUC_INTERNAL gboolean j_runner_job_pop (JRunner* runner, GQueue* waitq);
  G_GNUC_INTERNAL gboolean j_runner_job_pop_nth (JRunner* runner, GQueue* waitq, gint index);
  G_GNUC_INTERNAL void j_runner_job_print_all (JRunner* runner);
  G_GNUC_INTERNAL void j_runner_job_push (JRunner* runner, GQueue* waitq);
  G_GNUC_INTERNAL gboolean j_runner_run (JRunner* runner, GClosure* closure, gint* exit_code, GError** error);
  G_GNUC_INTERNAL const gchar* j_runner_variable_get (JRunner* runner, const gchar* key);
  G_GNUC_INTERNAL void j_runner_variable_print (JRunner* runner, const gchar* key);
  G_GNUC_INTERNAL void j_runner_variable_print_all (JRunner* runner);
  G_GNUC_INTERNAL void j_runner_variable_remove (JRunner* runner, const gchar* key);
  G_GNUC_INTERNAL void j_runner_variable_set (JRunner* runner, const gchar* key, const gchar* value);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_RUNTIME_RUNNER__
