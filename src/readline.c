/* Copyright 2023 MarcosHCK
 * Copyright 2023 DavierSB
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
#include <readline/readline.h>
#include <readline/history.h>
#include <readline.h>

struct _JReadline
{
  guint ref_count;
  guint history_file_size;
  guint history_size;
  gchar* history_file;
  gchar* lastpwd;
  gchar* prompt;
};

#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

JReadline* j_readline_new ()
{
  JReadline* self = NULL;

  const gchar* name = ".jash_history";
  const gchar* home = g_get_home_dir ();

  self = g_slice_new0 (JReadline);
  self->ref_count = 1;
  self->history_file = g_build_filename (home, name, NULL);
return self;
}

JReadline* j_readline_ref (JReadline* readline)
{
  g_return_val_if_fail (readline != NULL, NULL);
  JReadline* self = (readline);
return (g_atomic_int_inc (&self->ref_count), self);
}

void j_readline_unref (JReadline* readline)
{
  g_return_if_fail (readline != NULL);
  JReadline* self = (readline);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      _g_free0 (self->history_file);
      _g_free0 (self->lastpwd);
      _g_free0 (self->prompt);
      g_slice_free (JReadline, self);
    }
}

gchar* j_readline_getline (JReadline* readline_)
{
  g_return_val_if_fail (readline_ != NULL, NULL);
  JReadline* self = (readline_);
  gchar* pwd = NULL;

  if (!g_strcmp0 (pwd = g_get_current_dir (), self->lastpwd))
    g_free (pwd);
  else
    {
      _g_free0 (self->lastpwd);
      const gchar* user = g_get_user_name ();
      const gchar* host = g_get_host_name ();

      self->lastpwd = pwd;
      self->prompt = g_strdup_printf ("%s@%s:%s$ ", user, host, pwd);
    }

  gchar* line1 = readline (self->prompt);
  gchar* line2 = g_utf8_make_valid (line1, -1);
                 rl_free (line1);
    return line2;
}

void j_readline_save (JReadline* readline, GError** error)
{
  g_return_if_fail (readline != NULL);
  JReadline* self = (readline);
}

void j_readline_load (JReadline* readline, GError** error)
{
  g_return_if_fail (readline != NULL);
  JReadline* self = (readline);
}
