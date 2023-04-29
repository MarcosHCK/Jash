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

static GClosure* parse (JLexer* lexer, JParser* parser, JCodegen* codegen, const gchar* source, gboolean from_file, GError** error)
{
  GClosure* closure = NULL;
  GError* tmperr = NULL;
  JAst* ast = NULL;
  JTokens* tokens = NULL;

  if (from_file)
    {
      if ((tokens = j_lexer_scan_from_file (lexer, source, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          return NULL;
        }
    }
  else
    {
      if ((tokens = j_lexer_scan_from_data (lexer, source, strlen (source), &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          return NULL;
        }
    }

  if ((ast = j_parser_parse (parser, tokens, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      j_tokens_unref (tokens);
      return NULL;
    }
return (closure = j_codegen_emit (codegen, ast, error), j_ast_free (ast), j_tokens_unref (tokens), closure);
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
  GClosure* closure = NULL;
  GError* tmperr = NULL;
  JCodegen* codegen = NULL;
  JLexer* lexer = NULL;
  JParser* parser = NULL;
  JReadline* readline = NULL;
  JRunner* runner = NULL;
  gboolean finish = FALSE;
  gchar* line = NULL;
  gint i, exit_code = 0;

  codegen = j_codegen_new ();
  lexer = j_lexer_new ();
  parser = j_parser_new ();
  runner = j_runner_new (argc == 1);

#define cleanup() \
  (({ \
      g_object_unref (codegen); \
      g_object_unref (lexer); \
      g_object_unref (parser); \
      g_object_unref (runner); \
        exit_code; \
    }))

  if (argc > 1)
    {
      for (i = 1; i < argc && finish == FALSE; ++i)
      {
        if ((closure = parse (lexer, parser, codegen, argv [i], 1, &tmperr)), G_UNLIKELY (tmperr != NULL))
          {
            g_propagate_error (error, tmperr);
            return (cleanup (), 1);
          }

        if ((finish = j_runner_run (runner, closure, &exit_code, &tmperr)), G_LIKELY (tmperr == NULL))
          g_closure_unref (closure);
        else
          {
            g_propagate_error (error, tmperr);
            g_closure_unref (closure);
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

          if ((closure = parse (lexer, parser, codegen, line, 0, &tmperr)), G_UNLIKELY (tmperr != NULL))
            {
              const gint code = tmperr->code;
              const gchar* domain = g_quark_to_string (tmperr->domain);
              const gchar* message = tmperr->message;

              g_printerr ("%s: %i: %s\n", domain, code, message);
              _g_error_free0 (tmperr);
              continue;
            }

          if ((finish = j_runner_run (runner, closure, &exit_code, &tmperr)), G_UNLIKELY (tmperr != NULL))
            {
              g_propagate_error (error, tmperr);
              g_closure_unref (closure);
              return (cleanup (), 1);
            }

          j_readline_history_add (readline, line);
        }
      while ((_g_closure_unref0 (closure), finish) == FALSE);

      if ((j_readline_history_save (readline, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          return (cleanup (), 1);
        }
    }
return (cleanup (), exit_code);
}
