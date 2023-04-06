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
#include <instance.h>
#include <jobqueue.h>
#include <lexer.h>
#include <parser.h>
#include <readline.h>

#define _j_lexer_unref0(var) ((var == NULL) ? NULL : (var = (j_lexer_unref (var), NULL)))
#define _j_parser_unref0(var) ((var == NULL) ? NULL : (var = (j_parser_unref (var), NULL)))
#define _j_readline_unref0(var) ((var == NULL) ? NULL : (var = (j_readline_unref (var), NULL)))

int j_instance_again (int argc, char* argv [])
{
  g_assert_not_reached ();
}

int j_instance_help (int argc, char* argv [])
{
  g_assert_not_reached ();
}

int j_instance_history (int argc, char* argv [])
{
  g_assert_not_reached ();
}

int j_instance_jobs (int argc, char* argv [])
{
  g_assert_not_reached ();
}

static void prepare (JJobQueue* jobs, const gchar* data, gboolean data_is_file, GError** error)
{
  GError* tmperr = NULL;
  JLexer* lexer = NULL;
  JParser* parser = NULL;
  JToken* tokens = NULL;
  guint n_tokens = 0;
  JCode** codes = NULL;
  guint n_codes = 0;
  guint good = 0;

  if (data_is_file)
    lexer = j_lexer_new_from_file (data, &tmperr);
  else
    lexer = j_lexer_new_from_data (data, strlen (data), &tmperr);

  if (G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      _j_lexer_unref0 (lexer);
      return;
    }

  tokens = j_lexer_get_tokens (lexer, &n_tokens);
  parser = j_parser_new_from_tokens (tokens, n_tokens, &tmperr);
  _j_lexer_unref0 (lexer);

  if (G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      _j_parser_unref0 (parser);
      return;
    }

  codes = j_parser_get_codes (parser, &n_codes);
  good = (j_job_queue_add_intructions (jobs, codes, n_codes), 0);
  _j_parser_unref0 (parser);
}

int j_instance_shell (int argc, char* argv[])
{
  const gchar* lang_domain = "en_US";
  const gchar* description = "";
  const gchar* summary = "";

  GOptionContext* context = g_option_context_new ("[<script file>] ...");
  GError* tmperr = NULL;
  gint i;

  const GOptionEntry entries [] =
    {
      G_OPTION_ENTRY_NULL,
    };

  g_option_context_add_main_entries (context, entries, lang_domain);
  g_option_context_set_description (context, description);
  g_option_context_set_help_enabled (context, TRUE);
  g_option_context_set_ignore_unknown_options (context, TRUE);
  g_option_context_set_strict_posix (context, FALSE);
  g_option_context_set_summary (context, summary);
  g_option_context_set_translation_domain (context, lang_domain);
#ifdef G_OS_WIN32
  argv = g_win32_get_command_line ();

  g_option_context_parse_strv (context, &argv, &tmperr);
  argc = g_strv_length (argv);
#else // !G_OS_WIN32
  g_option_context_parse (context, &argc, &argv, &tmperr);
#endif // G_OS_WIN32
  g_option_context_free (context);

  if (G_UNLIKELY (tmperr != NULL))
    {
      const gint code = tmperr->code;
      const gchar* domain = g_quark_to_string (tmperr->domain);
      const gchar* message = tmperr->message;

      g_critical ("(" G_STRLOC "): %s: %i: %s", domain, code, message);
      g_error_free (tmperr);
#ifdef G_OS_WIN32
      g_strfreev (argv);
#endif // G_OS_WIN32
      return 1;
    }

  JJobQueue* jobs = j_job_queue_new ();
  JReadline* readline = NULL;
  gboolean no_interactive = FALSE;
  gint result = 0;

  do
  {
    if (argc > 1)
      {
        for (i = 1; i < argc && !tmperr; i++)
          prepare (jobs, argv [i], TRUE, &tmperr);
          no_interactive = TRUE;
#ifdef G_OS_WIN32
        g_strfreev (argv);
#endif // G_OS_WIN32      
      }
    else
      {
        gchar* line;

        if (G_UNLIKELY (readline == NULL))
          {
            j_readline_load (readline = j_readline_new (), &tmperr);

            if (G_UNLIKELY (tmperr != NULL))
              {
                result = 1;
                const gint code = tmperr->code;
                const gchar* domain = g_quark_to_string (tmperr->domain);
                const gchar* message = tmperr->message;

                g_critical ("(" G_STRLOC "): %s: %i: %s", domain, code, message);
                g_error_free (tmperr);
                break;
              }
          }

        prepare (jobs, line = j_readline_getline (readline), FALSE, &tmperr);
        g_free (line);
      }

    if (G_UNLIKELY (tmperr != NULL))
      {
        result = 1;
        const gint code = tmperr->code;
        const gchar* domain = g_quark_to_string (tmperr->domain);
        const gchar* message = tmperr->message;

        g_critical ("(" G_STRLOC "): %s: %i: %s", domain, code, message);
        g_error_free (tmperr);
        break;
      }

    do
      g_thread_yield ();
    while (j_job_queue_execute (jobs));
  } while (!no_interactive);

  _j_readline_unref0 (readline);
return (j_job_queue_unref (jobs), result);
}

int main (int argc, char* argv[])
{
  g_return_val_if_fail (argc >= 1, -1);

  const JInstanceIndex* index = j_instance_index_lookup (argv [0], strlen (argv [0]));
  const JInstance callback = (index == NULL) ? j_instance_shell : index->callback;
return callback (argc, argv);
}
