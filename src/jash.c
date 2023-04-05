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
#include <jobqueue.h>
#include <lexer.h>
#include <parser.h>

#define _j_lexer_unref0(var) ((var == NULL) ? NULL : (var = (j_lexer_unref (var), NULL)))
#define _j_parser_unref0(var) ((var == NULL) ? NULL : (var = (j_parser_unref (var), NULL)))

static void prepare (JJobQueue* jobs, const gchar* filename, GError** error)
{
  GError* tmperr = NULL;
  JLexer* lexer = NULL;
  JParser* parser = NULL;
  JToken* tokens = NULL;
  guint n_tokens = 0;
  JCode** codes = NULL;
  guint n_codes = 0;
  guint good = 0;

  lexer = j_lexer_new_from_file (filename, &tmperr);

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

int main (int argc, char* argv[])
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
      g_printerr ("%s\n", tmperr->message);
      g_error_free (tmperr);
#ifdef G_OS_WIN32
      g_strfreev (argv);
#endif // G_OS_WIN32
      return 1;
    }

  JJobQueue* jobs = j_job_queue_new ();
  gboolean no_interactive = FALSE;

  do
  {
    if (argc < 2)
      prepare (jobs, NULL, &tmperr);
    else
      {
        for (i = 1; i < argc && !tmperr; i++)
          prepare (jobs, argv [i], &tmperr);
          no_interactive = TRUE;
      }

    if (argc >= 2)
      {
#ifdef G_OS_WIN32
        g_strfreev (argv);
#endif // G_OS_WIN32
        argc = 0;
      }

    if (G_UNLIKELY (tmperr != NULL))
      {
        g_printerr ("%s\n", tmperr->message);
        g_error_free (tmperr);
        j_job_queue_unref (jobs);
        return 1;
      }

    do
      g_thread_yield ();
    while (j_job_queue_execute (jobs));
  } while (!no_interactive);
return (j_job_queue_unref (jobs), 0);
}
