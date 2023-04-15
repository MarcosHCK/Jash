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
#include <lexer.h>
#include <parser.h>
#include <readline.h>

struct _Jash
{
  JLexer* lexer;
  JParser* parser;
};

typedef struct _Jash Jash;
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _j_lexer_unref0(var) ((var == NULL) ? NULL : (var = (j_lexer_unref (var), NULL)))
#define _j_module_unref0(var) ((var == NULL) ? NULL : (var = (j_module_unref (var), NULL)))
#define _j_parser_unref0(var) ((var == NULL) ? NULL : (var = (j_parser_unref (var), NULL)))
#define _j_readline_unref0(var) ((var == NULL) ? NULL : (var = (j_readline_unref (var), NULL)))

static void jash_init (Jash* self)
{
  self->lexer = j_lexer_new ();
  self->parser = j_parser_new ();
}

static void jash_clear (Jash* self)
{
  j_lexer_unref (self->lexer);
  j_parser_unref (self->parser);
}

static JModule* jash_load_data (Jash* self, const gchar* data, GError** error)
{
  JTokens* tokens = j_tokens_new ();
  JModule* module = NULL;
  GError* tmperr = NULL;

  if (j_lexer_scan_from_data (self->lexer, tokens, data, strlen (data), &tmperr), G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      j_tokens_unref (tokens);
    }
return (j_parser_parse (self->parser, module, tokens, error), j_tokens_unref (tokens), module);
}

static JModule* jash_load_file (Jash* self, const gchar* filename, GError** error)
{
  JTokens* tokens = j_tokens_new ();
  JModule* module = NULL;
  GError* tmperr = NULL;

  if (j_lexer_scan_from_file (self->lexer, tokens, filename, &tmperr), G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      j_tokens_unref (tokens);
    }
return (j_parser_parse (self->parser, module, tokens, error), j_tokens_unref (tokens), module);
}

static void jash_script (Jash* self, const gchar* filename, GError** error)
{
  GError* tmperr = NULL;
  JModule* module = NULL;

  module = jash_load_file (self, filename, &tmperr);

  if (G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else
    {
      (void) module;
    }
}

static void jash_interactive (Jash* self, JReadline* readline, GError** error)
{
  GError* tmperr = NULL;
  JModule* module = NULL;
  gchar* line;

  while (TRUE)
    {
      line = j_readline_getline (readline);
      module = jash_load_data (self, line, &tmperr);

      if (G_UNLIKELY (tmperr != NULL))
        {
          const gint code = tmperr->code;
          const gchar* domain = g_quark_to_string (tmperr->domain);
          const gchar* message = tmperr->message;

          g_printerr ("%s: %i: %s", domain, code, message);
          _g_error_free0 (tmperr);
        }
      else
        {
          (void) module;
        }
    }
}

int main (int argc, char* argv [])
{
  const gchar* lang_domain = "en_US";
  const gchar* description_string = "";
  const gchar* parameter_string = "[<script file>] ...";
  const gchar* summary_string = "";

  GOptionContext* context;
  GError* tmperr = NULL;
  Jash self = {0};

  const GOptionEntry entries [] =
    {
      G_OPTION_ENTRY_NULL,
    };

  context = g_option_context_new (parameter_string);
  g_option_context_add_main_entries (context, entries, lang_domain);
  g_option_context_set_description (context, description_string);
  g_option_context_set_help_enabled (context, TRUE);
  g_option_context_set_ignore_unknown_options (context, TRUE);
  g_option_context_set_strict_posix (context, FALSE);
  g_option_context_set_summary (context, summary_string);
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

  jash_init (&self);

  if (argc == 1)
    {
      JReadline* readline = NULL;

      if (j_readline_load (readline = j_readline_new (), &tmperr), G_LIKELY (tmperr == NULL))
        jash_interactive (&self, readline, &tmperr);

        j_readline_save (readline, NULL);
        j_readline_unref (readline);
    }
  else
    {
      gint i;

      for (i = 1; i < argc; ++i)
      {
        if (jash_script (&self, argv [i], &tmperr), G_UNLIKELY (tmperr != NULL))
          break;
      }
    }

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
    }
return (jash_clear (&self), ((tmperr) ? 1 : 0));
}
