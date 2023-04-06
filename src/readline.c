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
#include <histcontrol.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <readline.h>

struct _JReadline
{
  guint ref_count;

  guint history_file_size;
  guint history_flags;
  guint history_size;
  gchar* history_file;
  gchar* lastpwd;
  gchar* prompt;
};

#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

static gint parsecontrol (const gchar* histcontrol)
{
  gunichar c;
  const JHistControlIndex* index;
  const gchar* first = histcontrol;
  const gchar* ptr = histcontrol;
  gint value = 0;

  do
  {
    switch (c = g_utf8_get_char (ptr))
    {
      case (gunichar) 0:
      case (gunichar) ':':
        {
          index = j_hist_control_index_lookup (first, ptr - first);
          first = g_utf8_next_char (ptr);

          if (index)
            value |= index->value;
          else
            {
              g_warning ("Ignoring HISTCONTROL value which is invalid");
              return 0;
            }

          if (c != (gunichar) 0)
            break;
          else
            return value;
        }
    }
  }
  while (ptr = g_utf8_next_char (ptr));
}

JReadline* j_readline_new ()
{
  JReadline* self = NULL;

  rl_initialize ();
  using_history ();

  rl_bind_key ('\t', rl_complete);

  const gchar* name = ".jash_history";
  const gchar* home = g_get_home_dir ();
  const gchar* history_file_size = NULL;
  const gchar* history_file = NULL;
  const gchar* history_flags = NULL;
  const gchar* history_size = NULL;

  const gint deflags = J_HIST_CONTROL_IGNORE_DUPLICATED | J_HIST_CONTROL_IGNORE_FIRST_SPACE;

  rl_catch_signals = TRUE;
  rl_catch_sigwinch = TRUE;

  self = g_slice_new0 (JReadline);
  self->ref_count = 1;
  self->history_file_size = (history_file_size = g_getenv ("HISTFILESIZE")) ? (gint) g_strtod (history_file_size, NULL) : 2000;
  self->history_file = (history_file = g_getenv ("HISTFILE")) ? g_strdup (history_file) : g_build_filename (home, name, NULL);
  self->history_flags = (history_flags = g_getenv ("HISTCONTROL")) ? (gint) parsecontrol (history_flags) : deflags;
  self->history_size = (history_size = g_getenv ("HISTSIZE")) ? (gint) g_strtod (history_size, NULL) : 1000;
return (stifle_history (self->history_size), self);
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

static void saveline (JReadline* self, const gchar* line)
{
  const HIST_ENTRY* entry = history_get (history_length);
  const gchar* last = entry->line;

  if (g_strcmp0 (line, last))
    {
      add_history (line);
    }
}

static int checkduplicated (const gchar* line)
{
  const HIST_ENTRY* entry = history_get (history_length);
  const gchar* last = entry->line;
return g_str_equal (line, last);
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

  rl_set_signals ();

  gchar* line1 = readline (self->prompt);
  gchar* line2 = g_utf8_make_valid (line1, -1);
  gchar* line3 = g_strstrip (line2);
                 rl_clear_signals ();

  gboolean nosave =
     (((self->history_flags & J_HIST_CONTROL_IGNORE_DUPLICATED) != 0)
        && checkduplicated (line3) == TRUE)
  || (((self->history_flags & J_HIST_CONTROL_IGNORE_FIRST_SPACE) != 0)
        && g_utf8_get_char (line1) == 0x20);
    g_clear_pointer (&line1, rl_free);
return ((nosave) ? 0 : (add_history (line3), 1), line3);
}

void j_readline_save (JReadline* readline, GError** error)
{
  g_return_if_fail (readline != NULL);
  JReadline* self = (readline);
  GFileError erro_;
  gint errn_;

  if ((errn_ = write_history (self->history_file)) != 0)
    {
      erro_ = g_file_error_from_errno (errn_);
      g_set_error (error, G_FILE_ERROR, erro_, g_strerror (errn_));
      return;
    }

  if ((errn_ = history_truncate_file (self->history_file, self->history_file_size)) != 0)
    {
      erro_ = g_file_error_from_errno (errn_);
      g_set_error (error, G_FILE_ERROR, erro_, g_strerror (errn_));
      return;
    }
}

void j_readline_load (JReadline* readline, GError** error)
{
  g_return_if_fail (readline != NULL);
  JReadline* self = (readline);
  GFileError erro_;
  gint errn_;

  if ((errn_ = read_history (self->history_file)) != 0)
    {
      if ((erro_ = g_file_error_from_errno (errn_)) == G_FILE_ERROR_NOENT)
        j_readline_save (readline, error);
      else
        g_set_error (error, G_FILE_ERROR, erro_, g_strerror (errn_));
    }
}
