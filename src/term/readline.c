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
#include <term/histcontrol.h>
#include <term/readline.h>

#define J_READLINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), J_TYPE_READLINE, JReadlineClass))
#define J_IS_READLINE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), J_TYPE_READLINE))
#define J_READLINE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), J_TYPE_READLINE, JReadlineClass))
typedef struct _JReadlineClass JReadlineClass;
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

struct _JReadline
{
  GObject parent;

  /*<parent>*/
  guint history_file_size;
  guint history_flags;
  guint history_size;
  gchar* history_file;
  GRegex* homerepl;
  gchar* lastpwd;
  gchar* prompt;
};

struct _JReadlineClass
{
  GObjectClass parent;
};

G_DEFINE_FINAL_TYPE (JReadline, j_readline, G_TYPE_OBJECT);

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
  while ((ptr = g_utf8_next_char (ptr)));
g_assert_not_reached ();
}

static void j_readline_class_constructed (GObject* pself)
{
  GError* tmperr = NULL;
  JReadline* self = (gpointer) pself;
G_OBJECT_CLASS (j_readline_parent_class)->constructed (pself);
  const gchar* name = ".jash_history";
  const gchar* home = g_get_home_dir ();
  const gchar* history_file_size = NULL;
  const gchar* history_file = NULL;
  const gchar* history_flags = NULL;
  const gchar* history_size = NULL;

  const gint deflags = J_HIST_CONTROL_IGNORE_DUPLICATED | J_HIST_CONTROL_IGNORE_FIRST_SPACE;

  GRegex* regex1 = g_regex_new ("/?$", 0, 0, &tmperr);
                g_assert_no_error (tmperr);
  gchar* path = g_regex_replace_literal (regex1, home, -1, 0, "", 0, &tmperr);
                g_assert_no_error (tmperr);
                g_regex_unref (regex1);
  gchar* escaped = g_regex_escape_string (path, strlen (path));
                g_free (path);
  gchar* pattern = g_strdup_printf ("^%s", escaped);
                g_free (escaped);
  GRegex* regex2 = g_regex_new (pattern, G_REGEX_OPTIMIZE, 0, &tmperr);
                g_assert_no_error (tmperr);
                g_free (pattern);

  self->homerepl = g_steal_pointer (&regex2);

  rl_catch_signals = TRUE;
  rl_catch_sigwinch = TRUE;

  self->history_file_size = (history_file_size = g_getenv ("HISTFILESIZE")) ? (gint) g_strtod (history_file_size, NULL) : 2000;
  self->history_file = (history_file = g_getenv ("HISTFILE")) ? g_strdup (history_file) : g_build_filename (home, name, NULL);
  self->history_flags = (history_flags = g_getenv ("HISTCONTROL")) ? (gint) parsecontrol (history_flags) : deflags;
  self->history_size = (history_size = g_getenv ("HISTSIZE")) ? (gint) g_strtod (history_size, NULL) : 1000;

  rl_initialize ();
  using_history ();

  stifle_history (self->history_size);
  rl_bind_key ('\t', rl_complete);
}

static void j_readline_class_finalize (GObject* pself)
{
  JReadline* self = (gpointer) pself; 
  _g_free0 (self->history_file);
  _g_free0 (self->lastpwd);
  _g_free0 (self->prompt);
G_OBJECT_CLASS (j_readline_parent_class)->finalize (pself);
}

static void j_readline_class_init (JReadlineClass* klass)
{
  G_OBJECT_CLASS (klass)->constructed = j_readline_class_constructed;
  G_OBJECT_CLASS (klass)->finalize = j_readline_class_finalize;
}

static void j_readline_init (JReadline* self)
{
}

JReadline* j_readline_new ()
{
  return g_object_new (J_TYPE_READLINE, NULL);
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

gchar* j_readline_get (JReadline* readline_)
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

      GError* tmperr = NULL;
      gchar* path = g_regex_replace (self->homerepl, pwd, -1, 0, "~", 0, &tmperr);
                  g_assert_no_error (tmperr);

      self->lastpwd = pwd;
      self->prompt = g_strdup_printf ("%s@%s:%s$ ", user, host, path);
        g_free (path);
    }

  rl_set_signals ();

  gchar* line1 = readline (self->prompt);
  gchar* line2 = g_utf8_make_valid (line1, -1);
return (rl_clear_signals (), rl_free (line1), line2);
}

void j_readline_history_add (JReadline* readline, const gchar* line)
{
  g_return_if_fail (J_IS_READLINE (readline));
  g_return_if_fail (line != NULL);
  JReadline* self = (readline);
  gchar* copy = g_strdup (line);

  if (((((self->history_flags & J_HIST_CONTROL_IGNORE_DUPLICATED) != 0)
        && checkduplicated (line) == TRUE)
      || (((self->history_flags & J_HIST_CONTROL_IGNORE_FIRST_SPACE) != 0)
        && g_utf8_get_char (line) == 0x20)) == FALSE)
    {
      add_history (g_strstrip (copy));
        g_free (copy);
    }
}

void j_readline_history_save (JReadline* readline, GError** error)
{
  g_return_if_fail (readline != NULL);
  JReadline* self = (readline);
  GFileError erro_;
  gint errn_;

  if ((errn_ = write_history (self->history_file)) != 0)
    {
      erro_ = g_file_error_from_errno (errn_);
      g_set_error_literal (error, G_FILE_ERROR, erro_, g_strerror (errn_));
      return;
    }

  if ((errn_ = history_truncate_file (self->history_file, self->history_file_size)) != 0)
    {
      erro_ = g_file_error_from_errno (errn_);
      g_set_error_literal (error, G_FILE_ERROR, erro_, g_strerror (errn_));
      return;
    }
}

void j_readline_history_load (JReadline* readline, GError** error)
{
  g_return_if_fail (readline != NULL);
  JReadline* self = (readline);
  GFileError erro_;
  gint errn_;

  if ((errn_ = read_history (self->history_file)) != 0)
    {
      if ((erro_ = g_file_error_from_errno (errn_)) == G_FILE_ERROR_NOENT)
        j_readline_history_save (readline, error);
      else
        g_set_error_literal (error, G_FILE_ERROR, erro_, g_strerror (errn_));
    }
}
