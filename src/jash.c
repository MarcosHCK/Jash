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
#include <jash.h>
#include <codegen/codegen.h>
#include <lexer/lexer.h>
#include <parser/parser.h>
#include <term/readline.h>

#define J_ASH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), J_TYPE_ASH, JAshClass))
#define J_IS_ASH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), J_TYPE_ASH))
#define J_ASH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), J_TYPE_ASH, JAshClass))
typedef struct _JAshClass JAshClass;
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_closure_unref0(var) ((var == NULL) ? NULL : (var = (g_closure_unref (var), NULL)))
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _j_ast_free0(var) ((var == NULL) ? NULL : (var = (j_ast_free (var), NULL)))
#define _j_tokens_unref0(var) ((var == NULL) ? NULL : (var = (j_tokens_unref (var), NULL)))

struct _JAsh
{
  GObject parent;

  /*<private>*/
  JCodegen* codegen;
  JLexer* lexer;
  JParser* parser;
  JReadline* readline; /* lazy */
};

struct _JAshClass
{
  GObjectClass parent;
};

G_DEFINE_FINAL_TYPE (JAsh, j_ash, G_TYPE_OBJECT);

static void j_ash_class_finalize (GObject* pself)
{
  JAsh* self = (gpointer) pself;
G_OBJECT_CLASS (j_ash_parent_class)->finalize (pself);
}

static void j_ash_class_dispose (GObject* pself)
{
  JAsh* self = (gpointer) pself;
  _g_object_unref0 (self->codegen);
  _g_object_unref0 (self->lexer);
  _g_object_unref0 (self->parser);
  _g_object_unref0 (self->readline);
G_OBJECT_CLASS (j_ash_parent_class)->dispose (pself);
}

static void j_ash_class_init (JAshClass* klass)
{
  G_OBJECT_CLASS (klass)->finalize = j_ash_class_finalize;
  G_OBJECT_CLASS (klass)->dispose = j_ash_class_dispose;
}

static void j_ash_init (JAsh* self)
{
  self->codegen = j_codegen_new ();
  self->lexer = j_lexer_new ();
  self->parser = j_parser_new ();
}

static GClosure* load (JAsh* self, const gchar* source, gboolean from_file, GError** error)
{
  JTokens* tokens = NULL;
  JAst* ast = NULL;
  GClosure* closure = NULL;
  GError* tmperr = NULL;

  if (from_file)
    {
      if ((tokens = j_lexer_scan_from_file (self->lexer, source, &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_prefixed_error (error, tmperr, "%s: ", (from_file) ? source : "(stdin)");
          _j_tokens_unref0 (tokens);
        }
    }
  else
    {
      if ((tokens = j_lexer_scan_from_data (self->lexer, source, strlen (source), &tmperr)), G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_prefixed_error (error, tmperr, "%s: ", (from_file) ? source : "(stdin)");
          _j_tokens_unref0 (tokens);
        }
    }

  if ((ast = j_parser_parse (self->parser, tokens, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_prefixed_error (error, tmperr, "%s: ", (from_file) ? source : "(stdin)");
      _j_tokens_unref0 (tokens);
      _j_ast_free0 (ast);
      return NULL;
    }

  if ((closure = j_codegen_emit (self->codegen, ast, &tmperr)), G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      _g_closure_unref0 (closure);
      _j_tokens_unref0 (tokens);
      _j_ast_free0 (ast);
      return NULL;
    }
return (j_ast_free (ast), j_tokens_unref (tokens), closure);
}

static gboolean run (JAsh* self, GClosure* closure, gboolean interactive, GError** error)
{
  GValue param_values [1] = { G_VALUE_INIT };
  GValue return_value [1] = { G_VALUE_INIT };
  GError* tmperr = NULL;
  gint status;

  g_value_init (return_value + 0, G_TYPE_INT);
  g_value_init (param_values + 0, G_TYPE_POINTER);
  g_value_set_pointer (param_values + 0, &tmperr);

  do
  {
    if ((g_closure_invoke (closure, return_value, 1, param_values, NULL)), G_UNLIKELY (tmperr != NULL))
      {
        g_propagate_error (error, tmperr);
        break;
      }
  } while (g_value_get_int (return_value) == J_CLOSURE_STATUS_CONTINUE);
    g_value_unset (return_value);
    g_value_unset (param_values + 0);
return (tmperr != NULL);
}

void j_ash_run_interactive (JAsh* self, GError** error)
{
  GClosure* closure = NULL;
  gboolean continue_ = 0;
  GError* tmperr = NULL;
  gchar* line;

  if (j_readline_load (self->readline = j_readline_new (), &tmperr), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else
    {
      while (TRUE)
      {
        if ((closure = load (self, line = j_readline_getline (self->readline), 0, &tmperr), g_free (line)),
              G_UNLIKELY (tmperr != NULL))
          {
            const gint code = tmperr->code;
            const gchar* domain = g_quark_to_string (tmperr->domain);
            const gchar* message = tmperr->message;

            g_printerr ("%s: %i: %s", domain, code, message);
            _g_error_free0 (tmperr);
          }
        else
          {
            if ((run (self, closure, 1, &tmperr), g_closure_unref (closure)), G_UNLIKELY (tmperr != NULL))
              {
                g_propagate_error (error, tmperr);
                break;
              }
          }
      }

      if (j_readline_save (self->readline, &tmperr), G_UNLIKELY (tmperr != NULL))
        g_propagate_error (error, tmperr);
    }
}

void j_ash_run_script (JAsh* self, const gchar* filename, GError** error)
{
  GClosure* closure = NULL;
  GError* tmperr = NULL;

  if ((closure = load (self, filename, 1, &tmperr)), G_UNLIKELY (tmperr != NULL))
    g_propagate_error (error, tmperr);
  else
    {
      run (self, closure, 0, error);
      g_closure_unref (closure);
    }
}

int main (int argc, char* argv [])
{
  const gchar* lang_domain = "en_US";
  const gchar* description_string = "";
  const gchar* parameter_string = "[<script file>] ...";
  const gchar* summary_string = "";

  GOptionContext* context = NULL;
  JAsh* jash = NULL;
  GError* tmperr = NULL;

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

  jash = g_object_new (J_TYPE_ASH, NULL);

  if (argc == 1)
    j_ash_run_interactive (jash, &tmperr);
  else
    {
      gint i;

      for (i = 1; i < argc; ++i)
      {
        if (j_ash_run_script (jash, argv [i], &tmperr), G_UNLIKELY (tmperr != NULL))
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
return (g_object_unref (jash), ((tmperr) ? 1 : 0));
}
