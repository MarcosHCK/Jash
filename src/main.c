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

static gint run (guint argc, gchar* argv[], GError** error)
{
  GClosure* closure = NULL;
  GError* tmperr = NULL;
  JAst* ast = NULL;
  JCodegen* codegen = NULL;
  JLexer* lexer = NULL;
  JParser* parser = NULL;
  JReadline* readline = NULL;
  JRunner* runner = NULL;
  JTokens* tokens = NULL;
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

  if (argc == 1)
    {
      if ((j_readline_history_load (readline = j_readline_new (), &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          return (cleanup (), 1);
        }

      do
      {
        if ((line = j_readline_get (readline)) == NULL)
          {
            /* Ctrl-C */
            g_assert_not_reached ();
          }

        if ((tokens = j_lexer_scan_from_data (lexer, line, -1, &tmperr)), G_UNLIKELY (tmperr != NULL))
          {
            g_propagate_error (error, tmperr);
            return (cleanup (), 1);
          }

        if ((ast = j_parser_parse (parser, tokens, &tmperr)), G_UNLIKELY (tmperr != NULL))
          {
            g_propagate_error (error, tmperr);
            j_tokens_unref (tokens);
            return (cleanup (), 1);
          }

        if ((closure = j_codegen_emit (codegen, ast, &tmperr)), G_UNLIKELY (tmperr != NULL))
          {
            g_propagate_error (error, tmperr);
            j_ast_free (ast);
            j_tokens_unref (tokens);
            return (cleanup (), 1);
          }

        j_ast_free (ast);
        j_tokens_unref (tokens);

        if ((finish = j_runner_run (runner, closure, &exit_code, &tmperr)), G_UNLIKELY (tmperr != NULL))
          {
            g_propagate_error (error, tmperr);
            g_closure_unref (closure);
            return (cleanup (), 1);
          }
      } while ((g_closure_unref (closure), finish) == FALSE);
    }
  else
    {
      for (i = 1; i < argc && finish == FALSE; ++i)
      {
        if ((tokens = j_lexer_scan_from_file (lexer, argv [i], &tmperr)), G_UNLIKELY (tmperr != NULL))
          {
            g_propagate_error (error, tmperr);
            return (cleanup (), 1);
          }

        if ((ast = j_parser_parse (parser, tokens, &tmperr)), G_UNLIKELY (tmperr != NULL))
          {
            g_propagate_error (error, tmperr);
            j_tokens_unref (tokens);
            return (cleanup (), 1);
          }

        if ((closure = j_codegen_emit (codegen, ast, &tmperr)), G_UNLIKELY (tmperr != NULL))
          {
            g_propagate_error (error, tmperr);
            j_ast_free (ast);
            j_tokens_unref (tokens);
            return (cleanup (), 1);
          }

        j_ast_free (ast);
        j_tokens_unref (tokens);

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
return (cleanup (), exit_code);
}

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
