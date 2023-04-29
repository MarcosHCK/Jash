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

enum
{
  prop_0,
  prop_history_control,
  prop_history_file,
  prop_history_file_size,
  prop_history_size,
  prop_number,
};

struct _JReadlineClass
{
  GObjectClass parent;
};

G_DEFINE_FINAL_TYPE (JReadline, j_readline, G_TYPE_OBJECT);
static GParamSpec* properties [prop_number] = {0};

static void j_readline_class_constructed (GObject* pself)
{
  JReadline* self = (gpointer) pself;
G_OBJECT_CLASS (j_readline_parent_class)->constructed (pself);
  const gchar* home = g_get_home_dir ();
  static gsize rl_initialized = 0;
  gchar *path, *escaped, *pattern, *tempor;
  GError* tmperr = NULL;
  GRegex* regex = NULL;

  regex = (regex = g_regex_new ("/?$", 0, 0, &tmperr), ({ g_assert_no_error (tmperr); }), regex);
  path = (tempor = g_regex_replace_literal (regex, home, -1, 0, "", 0, &tmperr), g_regex_unref (regex), ({ g_assert_no_error (tmperr); }), tempor);
  escaped = (tempor = g_regex_escape_string (path, strlen (path)), g_free (path), tempor);
  pattern = (tempor = g_strdup_printf ("^%s", escaped), g_free (escaped), tempor);
  self->homerepl = (regex = g_regex_new (pattern, G_REGEX_OPTIMIZE, 0, &tmperr), g_free (pattern), ({ g_assert_no_error (tmperr); }), regex);

  if (g_once_init_enter (&rl_initialized))
  {
    rl_initialize ();
    using_history ();

    rl_catch_signals = TRUE;
    rl_catch_sigwinch = TRUE;

    stifle_history (self->history_size);
    rl_bind_key ('\t', rl_complete);

    g_once_init_leave (&rl_initialized, 1);
  }
}

static void j_readline_class_finalize (GObject* pself)
{
  JReadline* self = (gpointer) pself; 
  _g_free0 (self->history_file);
  _g_free0 (self->lastpwd);
  _g_free0 (self->prompt);
G_OBJECT_CLASS (j_readline_parent_class)->finalize (pself);
}

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

static void j_readline_class_set_property (GObject* pself, guint property_id, const GValue* value, GParamSpec* pspec)
{
  JReadline* self = (gpointer) pself;

  switch (property_id)
  {
    case prop_history_control:
      self->history_flags = parsecontrol (g_value_get_string (value));
      break;
    case prop_history_file:
      g_free (self->history_file);
      self->history_file = g_value_dup_string (value);
      break;
    case prop_history_file_size:
    case prop_history_size:
      {
        GError* tmperr = NULL;
        gchar* string = (gchar*) g_value_get_string (value);
        guint64 number = 0;
  
        if ((g_ascii_string_to_unsigned (string, 10, 0, G_MAXUINT, &number, &tmperr)), G_UNLIKELY (tmperr != NULL))
          g_error ("(" G_STRLOC "): %s: %i: %s", g_quark_to_string (tmperr->domain), tmperr->code, tmperr->message);
        else
          {
            switch (property_id)
            {
              case prop_history_file_size: self->history_file_size = (guint) number; break;
              case prop_history_size: self->history_size = (guint) number; break;
            }
          }
        break;
      }
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (pself, property_id, pspec);
      break;
  }
}

static void j_readline_class_init (JReadlineClass* klass)
{
  G_OBJECT_CLASS (klass)->constructed = j_readline_class_constructed;
  G_OBJECT_CLASS (klass)->finalize = j_readline_class_finalize;
  G_OBJECT_CLASS (klass)->set_property = j_readline_class_set_property;

  const guint flags1 = G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT | G_PARAM_WRITABLE;
  const guint flags2 = G_PARAM_STATIC_STRINGS | G_PARAM_CONSTRUCT_ONLY | G_PARAM_WRITABLE;
  const gchar* file = g_build_filename (g_get_home_dir (), ".jash_history", NULL);

  properties [prop_history_control] = g_param_spec_string ("history-control", "history-control", "history-control", "ignoreboth", flags1);
  properties [prop_history_file] = g_param_spec_string ("history-file", "history-file", "history-file", file, flags2);
  properties [prop_history_file_size] = g_param_spec_string ("history-file-size", "history-file-size", "history-file-size", "2000", flags1);
  properties [prop_history_size] = g_param_spec_string ("history-size", "history-size", "history-size", "1000", flags1);
  g_object_class_install_properties (G_OBJECT_CLASS (klass), prop_number, properties);
  g_free ((gchar*) file);
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
