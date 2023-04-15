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
#include <ast.h>
#include <codegen.h>
#include <parser.h>
#include <walker.h>

#define J_PARSER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), J_TYPE_PARSER, JParserClass))
#define J_IS_PARSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), J_TYPE_PARSER))
#define J_PARSER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), J_TYPE_PARSER, JParserClass))
typedef struct _JParserClass JParserClass;
static GClosure* emit_closure (JCodegen* codegen, Ast ast, GError** error);
static Ast walk_arguments (Walker* walker, GError** error);
static Ast walk_command (Walker* walker, JToken* head, GError** error);
static Ast walk_expansion (Walker* walker, JToken* head, GError** error);
static Ast walk_expression (Walker* walker, JToken* head, GError** error);
static Ast walk_ifclosure (Walker* walker, JToken* head, GError** error);
static Ast walk_scope (Walker* walker, GError** error);
#define _g_array_unref0(var) ((var == NULL) ? NULL : (var = (g_array_unref (var), NULL)))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_node_destroy0(var) ((var == NULL) ? NULL : (var = (g_node_destroy (var), NULL)))
#define _g_ptr_array_unref0(var) ((var == NULL) ? NULL : (var = (g_ptr_array_unref (var), NULL)))
#define _j_parser_unref0(var) ((var == NULL) ? NULL : (var = (j_parser_unref (var), NULL)))

struct _JParser
{
  GObject parent;
};

struct _JParserClass
{
  GObjectClass parent;
};

G_DEFINE_FINAL_TYPE (JParser, j_parser, G_TYPE_OBJECT);
G_DEFINE_QUARK (j-parser-error-quark, j_parser_error);

static void j_parser_class_init (JParserClass* klass) { }
static void j_parser_init (JParser* self) { }

JParser* j_parser_new ()
{
  return g_object_new (J_TYPE_PARSER, NULL);
}

static void complain ()
{
  g_printerr ("closure called");
}

GClosure* j_parser_parse (JParser* parser, JTokens* tokens, GError** error)
{
  g_return_val_if_fail (parser != NULL, NULL);
  g_return_val_if_fail (tokens != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  JParser* self = (parser);
  GError* tmperr = NULL;

  Walker walker = WALKER_INIT;
  GClosure* closure = NULL;
  Ast ast = NULL;
  guint i;

  for (i = 0; i < tokens->array->count; ++i)
    {
      JToken* token = &tokens->array->elements [i];

      if (token->type != J_TOKEN_TYPE_COMMENT)
        walker_emplace (&walker, token);
    }

  walker_adjust (&walker, (({ g_assert_not_reached (); }), NULL));

  if ((ast = walk_scope (&walker, &tmperr), walker_clear (&walker)), G_UNLIKELY ((tmperr != NULL)))
    g_propagate_error (error, tmperr);
  else
    {
      JCodegen codegen;

      if ((closure = emit_closure (&codegen, ast, &tmperr), ast_free (ast)) != NULL)
        g_propagate_error (error, tmperr);
      else
        return closure;
    }
return NULL;
}

#if DEVELOPER
# define g_set_error(...) ({ g_printerr ("(" G_STRLOC "): g_set_error()!\n"); (g_set_error) (__VA_ARGS__); })
# define g_propagate_error(...) ({ g_printerr ("(" G_STRLOC "): g_propagate_error()!\n"); (g_propagate_error) (__VA_ARGS__); })
#endif // !DEVELOPER

#define locate(token) token->line,token->column
#define EXCPT(before,after) G_STMT_START { G_STMT_START { before; } G_STMT_END; return after; } G_STMT_END
#define THROW(code,...) ({ g_set_error (error, J_PARSER_ERROR, (code), __VA_ARGS__); })
#define RETHROW(tmperr) ({ GError* __tmperr = ((tmperr)); g_propagate_error (error, __tmperr); })
#define RETHROWP(tmperr,token) ({ GError* __tmperr = ((tmperr)); JToken* __token = ((token)); g_propagate_prefixed_error (error, __tmperr, "%d: %d: ", locate (__token)); })

#define THROW_EOS() THROW (J_PARSER_ERROR_UNEXPECTED_EOF, "%i: %i: Unexpected end of scope", locate (walker_last (walker)))
#define THROW_UNEXPECTED(token) ({ JToken* __token = ((token)); THROW (J_PARSER_ERROR_UNEXPECTED_TOKEN, "%i: %i: Unexpected token '%s'", locate (__token), __token->value); })

static GClosure* emit_closure (JCodegen* codegen, Ast ast, GError** error)
{
  GClosure* closure = NULL;

  j_codegen_init (codegen);
  j_codegen_prologue (codegen);
  j_codegen_generate (codegen, ast);
  j_codegen_epilogue (codegen);
return (closure = j_codegen_emit (codegen), j_codegen_clear (codegen), closure);
}

static void collect (Walker* src, Walker* dst, GError** error, gint type, ...)
{
  struct _Until
    {
      JTokenType type;
      gchar* value;
    } untils [32];

  va_list list;
  guint i, n_untils = 0;
  gchar* value;

  va_start (list, type);

  do
    {
      value = va_arg (list, gchar*);

      if (n_untils >= G_N_ELEMENTS (untils))
        g_assert_not_reached ();
      else
        {
          untils [n_untils].type = type;
          untils [n_untils].value = value;
          ++n_untils;
        }
    }
  while ((type = va_arg (list, gint)) > 0);

  JToken* token = NULL;
  GList* link = NULL;
  va_end (list);

  while ((link = walker_take_link (src)) != NULL)
    {
      walker_emplace_link (dst, link);

      for (i = 0; i < n_untils; ++i)
      {
        type = untils [i].type;
        value = untils [i].value;

        if (((token = link->data)->type == type) && (value == NULL || (value == token->value || g_str_equal (value, token->value))))
          return;
      }
    }
  EXCPT (({ Walker* walker = src; THROW_EOS (); }),);
}

static Ast walk_arguments (Walker* walker, GError** error)
{ dumpwalker (walker);
  GError* tmperr = NULL;
  JToken* redirect = NULL;
  JToken* token = NULL;

  Ast ast = ast_node (AST_ARGUMENTS);

  while ((token = walker_take (walker)) != NULL)
    {
      const guint type = token->type;
      const gchar* value = token->value;

      switch ((JTokenType) type)
      {
        case J_TOKEN_TYPE_BUILTIN:
        case J_TOKEN_TYPE_KEYWORD:
        case J_TOKEN_TYPE_LITERAL:
        case J_TOKEN_TYPE_QUOTED:
          {
            if (redirect == NULL)
              ast_append (ast, ast_data (value));
            else
              {
                if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_APPEND)
                  ast_append (ast, ast_wrap (AST_REDIRECT_OUTPUT_APPEND, ast_data (value)));
                else if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_READ)
                  ast_append (ast, ast_wrap (AST_REDIRECT_INPUT, ast_data (value)));
                else if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_WRITE)
                  ast_append (ast, ast_wrap (AST_REDIRECT_OUTPUT_REPLACE, ast_data (value)));
                  g_steal_pointer (&redirect);
              }
            break;
          }

        case J_TOKEN_TYPE_OPERATOR:
          {
            if (value != J_TOKEN_OPERATOR_EXPANSION)
              {
                if (redirect != NULL)
                  EXCPT (THROW_UNEXPECTED (token), (ast_free (ast), NULL));
                else
                  {
                    gunichar c;
                    switch (c = g_utf8_get_char (value))
                    {
                      case (gunichar) '>':
                      case (gunichar) '<':
                        redirect = token;
                        break;
                      default: EXCPT (THROW_UNEXPECTED (token), (ast_free (ast), NULL));
                    }
                  }
              }
            else
              {
                Walker walker2 = WALKER_INIT;
                Ast child = NULL;

                if (collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_EXPANSION, -1), G_UNLIKELY (tmperr != NULL))
                  EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
                else
                  {
                    walker_withdraw (&walker2);
                    walker_adjust (&walker2, token);

                    if ((child = walk_expansion (&walker2, token, &tmperr), walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
                      EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
                    else
                      {
                        if (redirect == NULL)
                          ast_append (ast, child);
                        else
                        {
                          if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_APPEND)
                            ast_append (ast, ast_wrap (AST_REDIRECT_OUTPUT_APPEND, ast_data (value)));
                          else if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_READ)
                            ast_append (ast, ast_wrap (AST_REDIRECT_INPUT, ast_data (value)));
                          else if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_WRITE)
                            ast_append (ast, ast_wrap (AST_REDIRECT_OUTPUT_REPLACE, ast_data (value)));
                            g_steal_pointer (&redirect);
                        }
                      }
                  }
              }
            break;
          }

        default: EXCPT (THROW_UNEXPECTED (token), (ast_free (ast), NULL));
      }
    }
return (ast);
}

static Ast walk_command (Walker* walker, JToken* head, GError** error)
{ dumpwalker (walker);
  GError* tmperr = NULL;
  Ast ast = NULL;

  const guint type = head->type;
  const gchar* value = head->value;

  switch ((JTokenType) type)
    {
      case J_TOKEN_TYPE_BUILTIN:
        {
          ast = ast_node (AST_BUILTIN);

          if (value == J_TOKEN_BUILTIN_AGAIN
            || value == J_TOKEN_BUILTIN_CD
            || value == J_TOKEN_BUILTIN_EXIT
            || value == J_TOKEN_BUILTIN_FG
            || value == J_TOKEN_BUILTIN_GET
            || value == J_TOKEN_BUILTIN_HELP
            || value == J_TOKEN_BUILTIN_HISTORY
            || value == J_TOKEN_BUILTIN_UNSET)
            {
              if (walker_length (walker) > 1)
                EXCPT (THROW (J_PARSER_ERROR_UNEXPECTED_TOKEN, "%d: %d: Too many arguments for '%s'", locate (head), value), (ast_free (ast), NULL));
              else
                {
                  Ast child;
                  ast_append (ast, ast_wrap (AST_TARGET, ast_data (value)));

                  if ((child = walk_arguments (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
                    EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
                  else
                    ast_append (ast, child);
                }
            }
          else
          if (value == J_TOKEN_BUILTIN_FALSE
            || value == J_TOKEN_BUILTIN_JOBS
            || value == J_TOKEN_BUILTIN_TRUE)
            {
              if (walker_length (walker) > 0)
                EXCPT (THROW (J_PARSER_ERROR_UNEXPECTED_TOKEN, "%d: %d: Too many arguments for '%s'", locate (head), value), (ast_free (ast), NULL));
              G_STMT_START
                {
                  Ast child;
                  ast_append (ast, ast_wrap (AST_TARGET, ast_data (value)));
                  ast_append (ast, ast_node (AST_ARGUMENTS));
                }
              G_STMT_END;
            }
          else
          if (value == J_TOKEN_BUILTIN_SET)
            {
              if (walker_length (walker) > 2)
                EXCPT (THROW (J_PARSER_ERROR_UNEXPECTED_TOKEN, "%d: %d: Too many arguments for '%s'", locate (head), value), (ast_free (ast), NULL));
              else if (walker_length (walker) == 1)
                EXCPT (THROW (J_PARSER_ERROR_UNEXPECTED_TOKEN, "%d: %d: Too few arguments for '%s'", locate (head), value), (ast_free (ast), NULL));
              else
                {
                  Ast child;
                  ast_append (ast, ast_wrap (AST_TARGET, ast_data (value)));

                  if ((child = walk_arguments (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
                    EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
                  else
                    ast_append (ast, child);
                }
            }

          break;
        }

      case J_TOKEN_TYPE_LITERAL:
        {
          Ast child;
          ast_append ((ast = ast_node (AST_INVOKE)), ast_wrap (AST_TARGET, ast_data (value)));

          if ((child = walk_arguments (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
            EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
          else
            ast_append (ast, child);
          break;
        }

      case J_TOKEN_TYPE_OPERATOR:
        {
  #if DEVELOPER == 1
          g_assert (value == J_TOKEN_OPERATOR_EXPANSION);
  #endif // DEVELOPER
          Walker walker2 = WALKER_INIT;
          Ast target = NULL;
          Ast child = NULL;

          walker_leave (walker, head);

          if ((child = walk_arguments (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
            EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
          else
            {
              ast = ast_node (AST_INVOKE);
              target = ast_first (child);

  #if DEVELOPER == 1
              g_assert (ast_first (child)->data == GINT_TO_POINTER (AST_EXPANSION));
  #endif // DEVELOPER

              ast_unlink (target);
              ast_append (ast, child);
              ast_append (ast, ast_wrap (AST_TARGET, target));
            }
          break;
        }

      default: EXCPT (THROW_UNEXPECTED (head), (ast_free (ast), NULL));
    }
return ast;
}

static Ast walk_expansion (Walker* walker, JToken* head, GError** error)
{ dumpwalker (walker);
  GError* tmperr = NULL;
  Ast ast = NULL;

  if ((ast = walk_scope (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
    EXCPT (RETHROW (tmperr), NULL);
  else
    {
      ast->data = GINT_TO_POINTER (AST_EXPANSION);
    }
return ast;
}

static Ast walk_expression (Walker* walker, JToken* head, GError** error)
{ dumpwalker (walker);
  GError* tmperr = NULL;
  JToken* token = NULL;

  Ast ast = NULL;
  Ast command = NULL;

#define SEPARATORS \
  J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_DETACH, \
  J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_EXPANSION, \
  J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_LOGICAL_AND, \
  J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_LOGICAL_OR, \
  J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_PIPE

  token = head;

  do
    {
      Walker walker2 = WALKER_INIT;
      JToken* oper = NULL;

      while (TRUE)
        {
          if (collect (walker, &walker2, &tmperr, SEPARATORS, -1), G_UNLIKELY (tmperr != NULL))
            {
              if (G_LIKELY (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_UNEXPECTED_EOF)))
                _g_error_free0 (tmperr);
              else
                EXCPT (RETHROW (tmperr), (walker_clear (&walker2), ast_free (ast), NULL));
            }
          else
            {
              if ((oper = walker_peek_back (&walker2))->value != J_TOKEN_OPERATOR_EXPANSION)
                walker_withdraw (&walker2);
              else
              {
                if (head->value == J_TOKEN_OPERATOR_EXPANSION)
                  continue;
                else
                {
                  if (collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_EXPANSION, -1), G_UNLIKELY (tmperr != NULL))
                    EXCPT (RETHROW (tmperr), (walker_clear (&walker2), ast_free (ast), NULL));
                  else continue;
                }
              }
            }

          break;
        }

      G_STMT_START
        {
          walker_adjust (&walker2, token);

          if ((command = walk_command (&walker2, token, &tmperr), walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
            EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
          else
            {
              ast = (ast == NULL) ? command : (ast_append (ast, command), ast);

              if (oper != NULL)
              {
                const guint type = oper->type;
                const gchar* value = oper->value;

                if (value == J_TOKEN_OPERATOR_DETACH)
                  {
                    if (walker_length (walker) > 0)
                      EXCPT (THROW_UNEXPECTED (oper), (ast_free (ast), NULL));
                    else
                      ast = ast_wrap (AST_DETACH, ast);
                  }
                else if (value == J_TOKEN_OPERATOR_LOGICAL_AND)
                  ast = ast_wrap (AST_LOGICAL_AND, ast);
                else if (value == J_TOKEN_OPERATOR_LOGICAL_OR)
                  ast = ast_wrap (AST_LOGICAL_OR, ast);
                else if (value == J_TOKEN_OPERATOR_PIPE)
                  ast = ast_wrap (AST_PIPE, ast);
              }
            }
        }
      G_STMT_END;
    }
#undef SEPARATORS
  while ((token = walker_take (walker)) != NULL);
return ast;
}

static Ast walk_ifclosure (Walker* walker, JToken* head, GError** error)
{ dumpwalker (walker);
  Walker walker2 = WALKER_INIT;
  JToken* then = NULL;
  GError* tmperr = NULL;

  Ast ast = ast_node (AST_IFCLOSURE);
  Ast child = NULL;

  gboolean reverse = FALSE;
  guint ifcount = 1;

  if (collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_THEN, -1), G_UNLIKELY (tmperr != NULL))
    EXCPT (RETHROW (tmperr), (walker_clear (&walker2), ast_free (ast), NULL));
  else
    {
      then = walker_peek_back (&walker2);

      walker_withdraw (&walker2);
      walker_adjust (&walker2, head);

      if (walker_peek_front (&walker2)->value == J_TOKEN_KEYWORD_IF)
        EXCPT (THROW_UNEXPECTED (walker_peek_front (&walker2)), (walker_clear (&walker2), ast_free (ast), NULL));
      else
      if ((child = walk_scope (&walker2, &tmperr), walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
        EXCPT (RETHROW (tmperr), (walker_clear (&walker2), ast_free (ast), NULL));
      else
        {
          ast_append (ast, ast_retype (AST_IFCLOSURE_CONDITION, child));

          while (TRUE)
            {
              if (collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_IF, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_ELSE, -1), G_UNLIKELY (tmperr != NULL))
                {
                  if (G_UNLIKELY (!g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_UNEXPECTED_EOF)))
                    EXCPT (RETHROW (tmperr), (walker_clear (&walker2), ast_free (ast), NULL));
                  else
                    {
                      if (ifcount > 1)
                        EXCPT (RETHROW (tmperr), (walker_clear (&walker2), ast_free (ast), NULL));
                      else
                        _g_error_free0 (tmperr);
                    }
                }
              else
                {
                  const gchar* value2 = walker_peek_back (&walker2)->value;

                  if (value2 == J_TOKEN_KEYWORD_IF)
                    {
                      ++ifcount;
                      continue;
                    }
                  else if (value2 == J_TOKEN_KEYWORD_ELSE)
                    {
                      if (--ifcount > 0)
                        continue;
                    }
                }

              break;
            }

          G_STMT_START
            {
              if (walker_peek_back (&walker2)->value == J_TOKEN_KEYWORD_ELSE)
                {
                  walker_withdraw (&walker2);
                  reverse = TRUE;
                }

              walker_adjust (&walker2, then);

              if ((child = walk_scope (&walker2, &tmperr), walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
                EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
              else
                {
                  ast_append (ast, ast_retype (AST_IFCLOSURE_DIRECT, child));

                  if (reverse == FALSE && walker_length (walker) > 0)
                    EXCPT (THROW_UNEXPECTED (walker_take (walker)), (ast_free (ast), NULL));
                  else if (reverse == TRUE)
                    {
                      if ((child = walk_scope (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
                        EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
                      else
                        ast_append (ast, ast_retype (AST_IFCLOSURE_REVERSE, child));
                    }
                }
            }
          G_STMT_END;
        }
    }
return ast;
}

static Ast walk_scope (Walker* walker, GError** error)
{ dumpwalker (walker);
  GError* tmperr = NULL;
  JToken* token = NULL;

  Ast ast = ast_node (AST_SCOPE);

  while ((token = walker_take (walker)) != NULL)
    {
      const guint type = token->type;
      const gchar* value = token->value;

      switch (type)
      {
        case J_TOKEN_TYPE_OPERATOR:
          {
            if (value != J_TOKEN_OPERATOR_EXPANSION)
              EXCPT (THROW_UNEXPECTED (token), (ast_free (ast), NULL));
            G_GNUC_FALLTHROUGH;
          }
        case J_TOKEN_TYPE_BUILTIN:
        case J_TOKEN_TYPE_LITERAL:
          {
            Walker walker2 = WALKER_INIT;
            Ast expression = NULL;

            if ((collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_SEPARATOR, NULL, -1)), G_LIKELY (tmperr == NULL))
              walker_withdraw (&walker2);
            else
              {
                if (G_LIKELY (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_UNEXPECTED_EOF)))
                  _g_error_free0 (tmperr);
                else
                  EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
              }
            G_STMT_START
              {
                walker_adjust (&walker2, token);

                if ((expression = walk_expression (&walker2, token, &tmperr), walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
                  EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
                else
                  ast_append (ast, expression);
              }
            G_STMT_END;
            break;
          }

        case J_TOKEN_TYPE_KEYWORD:
          {
            if (value != J_TOKEN_KEYWORD_IF)
              EXCPT (THROW_UNEXPECTED (token), (ast_free (ast), NULL));
            else
              {
                Walker walker2 = WALKER_INIT;
                Ast closure = NULL;
                guint ifcount = 1;

                while (TRUE)
                  {
                    if (collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_IF, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_END, -1), G_UNLIKELY (tmperr != NULL))
                      EXCPT (RETHROW (tmperr), (walker_clear (&walker2), ast_free (ast), NULL));
                    else
                      {
                        const gchar* value2 = walker_peek_back (&walker2)->value;

                        if (value2 == J_TOKEN_KEYWORD_IF)
                          {
                            ++ifcount;
                            continue;
                          }
                        else if (value2 == J_TOKEN_KEYWORD_END)
                          {
                            if (--ifcount > 0)
                              continue;
                          }
                      }

                    break;
                  }

                G_STMT_START
                  {
                    walker_withdraw (&walker2);
                    walker_adjust (&walker2, token);

                    if ((closure = walk_ifclosure (&walker2, token, &tmperr), walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
                      EXCPT (RETHROW (tmperr), (ast_free (ast), NULL));
                    else
                      ast_append (ast, closure);
                  }
                G_STMT_END;
              }
            break;
          }

        case J_TOKEN_TYPE_SEPARATOR:
          break;

        default: EXCPT (THROW_UNEXPECTED (token), (ast_free (ast), NULL));
      }
    }
return ast;
}
