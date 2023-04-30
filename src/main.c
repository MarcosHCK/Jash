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
#include <glib.h>
#include <lexer/lexer.h>
#include <parser/parser.h>
#include <runtime/runner.h>
#include <term/readline.h>

#define _g_closure_unref0(var) ((var == NULL) ? NULL : (var = (g_closure_unref (var), NULL)))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
static gint run (guint argc, gchar* argv[], GError** error);

int main (int argc, char* argv [])
{
  const gchar* lang_domain = "en_US";
  const gchar* description_string = "";
  const gchar* parameter_string = "[<script file>] ...";
  const gchar* summary_string = "";
  GOptionContext* context = NULL;
  GError* tmperr = NULL;
  gint exit_code;

  static GOptionEntry entries [] =
    {
      G_OPTION_ENTRY_NULL,
    };

#ifdef G_OS_WIN32
  argv = g_win32_get_command_line ();
#endif // G_OS_WIN32
  context = g_option_context_new (parameter_string);

  g_option_context_add_main_entries (context, entries, lang_domain);
  g_option_context_set_description (context, description_string);
  g_option_context_set_help_enabled (context, TRUE);
  g_option_context_set_ignore_unknown_options (context, FALSE);
  g_option_context_set_strict_posix (context, FALSE);
  g_option_context_set_summary (context, summary_string);
  g_option_context_set_translation_domain (context, lang_domain);
#ifdef G_OS_WIN32
  g_option_context_parse_strv (context, &argv, &tmperr);
  argc = g_strv_length (argv);
#else // G_OS_WIN32
  g_option_context_parse (context, &argc, &argv, &tmperr);
#endif // G_OS_WIN32

  if ((g_option_context_free (context)), G_UNLIKELY (tmperr != NULL))
    {
      const gint code = tmperr->code;
      const gchar* domain = g_quark_to_string (tmperr->domain);
      const gchar* message = tmperr->message;
#ifdef G_OS_WIN32
      g_strfreev (argv);
#endif // G_OS_WIN32
      g_printerr ("%s: %i: %s\n", domain, code, message);
      return (g_error_free (tmperr), 1);
    }

  if ((exit_code = run (argc, argv, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      const gint code = tmperr->code;
      const gchar* domain = g_quark_to_string (tmperr->domain);
      const gchar* message = tmperr->message;
#ifdef G_OS_WIN32
      g_strfreev (argv);
#endif // G_OS_WIN32
      g_printerr ("%s: %i: %s\n", domain, code, message);
      return (g_error_free (tmperr), 1);
    }
return (exit_code);
}

static void on_variable_modifying (JRunner* runner, const gchar* key, const gchar* value, JReadline* readline)
{
  if (!g_strcmp0 (key, "HISTCONTROL")) g_object_set (readline, "history-control", value, NULL);
  else if (!g_strcmp0 (key, "HISTFILE")) g_object_set (readline, "history-file", value, NULL);
  else if (!g_strcmp0 (key, "HISTFILESIZE")) g_object_set (readline, "history-file-size", value, NULL);
  else if (!g_strcmp0 (key, "HISTSIZE")) g_object_set (readline, "history-size", value, NULL);
}

static inline void defaultpropval (GObject* self, const gchar* property_name)
{
  GObjectClass* klass;
  GParamSpec* pspec;
  const GValue* value;

  g_return_if_fail ((klass = G_OBJECT_GET_CLASS (self)) != NULL);
  g_return_if_fail ((pspec = g_object_class_find_property (klass, property_name)) != NULL);
  g_return_if_fail ((value = g_param_spec_get_default_value (pspec)) != NULL);
  g_object_set_property (self, property_name, value);
}

static void on_variable_removing (JRunner* runner, const gchar* key, JReadline* readline)
{
  if (!g_strcmp0 (key, "HISTCONTROL")) defaultpropval ((gpointer) readline, "history-control");
  else if (!g_strcmp0 (key, "HISTFILE")) defaultpropval ((gpointer) readline, "history-file");
  else if (!g_strcmp0 (key, "HISTFILESIZE")) defaultpropval ((gpointer) readline, "history-file-size");
  else if (!g_strcmp0 (key, "HISTSIZE")) defaultpropval ((gpointer) readline, "history-size");
}

static gint run (guint argc, gchar* argv[], GError** error)
{
  GError* tmperr = NULL;
  JReadline* readline = NULL;
  JRunner* runner = NULL;
  gboolean finish = FALSE;
  gchar* line = NULL;
  gint i, exit_code = 0;

  runner = j_runner_new (argc == 1);

#define cleanup() \
    (({ \
        _g_object_unref0 (readline); \
        _g_object_unref0 (runner); \
      }))

  if (argc > 1)
    {
      for (i = 1; i < argc && finish == FALSE; ++i)
        {
          if ((finish = j_runner_run_file (runner, argv [i], &exit_code, &tmperr)), G_UNLIKELY (tmperr != NULL))
            {
              g_propagate_error (error, tmperr);
              return (cleanup (), 1);
            }
        }
    }
  else
    {
      if ((j_readline_history_load (readline = j_readline_new (), &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          return (cleanup (), 1);
        }

      g_signal_connect_object (runner, "variable-modifying", G_CALLBACK (on_variable_modifying), readline, 0);
      g_signal_connect_object (runner, "variable-removing", G_CALLBACK (on_variable_removing), readline, 0);

      do
        {
          if ((line = j_readline_get (readline)) == NULL)
            {
              /* Ctrl-C */
              g_assert_not_reached ();
            }

          if ((finish = j_runner_run_line (runner, line, &exit_code, &tmperr)), G_UNLIKELY (tmperr != NULL))
            {
              g_propagate_error (error, tmperr);
              return (cleanup (), 1);
            }
        } while ((j_readline_history_add (readline, line), finish) == FALSE);

      if ((j_readline_history_save (readline, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          return (cleanup (), 1);
        }
    }
return (cleanup (), exit_code);
#undef cleanup
}
