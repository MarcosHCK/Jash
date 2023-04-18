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
#include <parser/ast.h>
#include <parser/parser.h>
#include <parser/private.h>
#include <parser/walker.h>

#define J_PARSER_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), J_TYPE_PARSER, JParserClass))
#define J_IS_PARSER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), J_TYPE_PARSER))
#define J_PARSER_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS ((obj), J_TYPE_PARSER, JParserClass))
typedef struct _JParserClass JParserClass;
static JAst* walk_arguments (JWalker* walker, GError** error);
static JAst* walk_command (JWalker* walker, JToken* head, GError** error);
static JAst* walk_expansion (JWalker* walker, JToken* head, GError** error);
static JAst* walk_expression (JWalker* walker, JToken* head, GError** error);
static JAst* walk_ifclosure (JWalker* walker, JToken* head, GError** error);
static JAst* walk_scope (JWalker* walker, GError** error);
#define _g_array_unref0(var) ((var == NULL) ? NULL : (var = (g_array_unref (var), NULL)))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _g_ptr_array_unref0(var) ((var == NULL) ? NULL : (var = (g_ptr_array_unref (var), NULL)))
#define _j_ast_free0(var) ((var == NULL) ? NULL : (var = (j_ast_free (var), NULL)))
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

JAst* j_parser_parse (JParser* parser, JTokens* tokens, GError** error)
{
  g_return_val_if_fail (J_IS_PARSER (parser), NULL);
  g_return_val_if_fail (tokens != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  JParser* self = (parser);
  GError* tmperr = NULL;

  guint n_tokens = j_tokens_get_count (tokens);
  JToken* tokens_ = j_tokens_index (tokens, 0);

  JWalker walker = J_WALKER_INIT;
  GClosure* closure = NULL;
  JAst* ast = NULL;
  guint i;

  for (i = 0; i < n_tokens; ++i)
  if (tokens_ [i].type != J_TOKEN_TYPE_COMMENT)
    j_walker_emplace (&walker, &tokens_ [i]);

  j_walker_adjust (&walker, (({ g_assert_not_reached (); }), NULL));

  if ((ast = walk_scope (&walker, &tmperr), j_walker_clear (&walker)), G_LIKELY ((tmperr == NULL)))
    j_ast_dump (ast);
  else
    {
      g_propagate_error (error, tmperr);
      _j_ast_free0 (ast);
    }
return ast;
}

#if DEVELOPER
# define g_set_error(...) ({ g_printerr ("(" G_STRLOC "): g_set_error()!\n"); (g_set_error) (__VA_ARGS__); })
# define g_propagate_error(...) ({ g_printerr ("(" G_STRLOC "): g_propagate_error()!\n"); (g_propagate_error) (__VA_ARGS__); })
#endif // !DEVELOPER

#define locate(token) token->line,token->column
#define EXCPT(before,after) G_STMT_START { G_STMT_START { before; } G_STMT_END; return after; } G_STMT_END
#define THROW(code,...) ({ g_set_error (error, J_PARSER_ERROR, (code), __VA_ARGS__); })
#define THROWL(code,literal) ({ g_set_error_literal (error, J_PARSER_ERROR, (code), ((literal))); })
#define RETHROW(tmperr) ({ GError* __tmperr = ((tmperr)); g_propagate_error (error, __tmperr); })
#define RETHROWP(tmperr,token) ({ GError* __tmperr = ((tmperr)); JToken* __token = ((token)); g_propagate_prefixed_error (error, __tmperr, "%d: %d: ", locate (__token)); })

#define THROW_EOS() THROW (J_PARSER_ERROR_UNEXPECTED_EOF, "%i: %i: Unexpected end of scope", locate (j_walker_last (walker)))
#define THROW_UNEXPECTED(token) ({ JToken* __token = ((token)); THROW (J_PARSER_ERROR_UNEXPECTED_TOKEN, "%i: %i: Unexpected token '%s'", locate (__token), __token->value); })

static void collect (JWalker* src, JWalker* dst, GError** error, gint type, ...)
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

  while ((link = j_walker_take_link (src)) != NULL)
    {
      j_walker_emplace_link (dst, link);

      for (i = 0; i < n_untils; ++i)
      {
        type = untils [i].type;
        value = untils [i].value;

        if (((token = link->data)->type == type) && (value == NULL || (value == token->value || g_str_equal (value, token->value))))
          return;
      }
    }
  EXCPT (({ JWalker* walker = src; THROW_EOS (); }),);
}

static guint claim_arguments (JAst* ast, gint min_arguments, gint max_arguments, GError** error)
{
  guint n_arguments = 0;
  JAst* arguments;
  JAst* child;

  if ((arguments = j_ast_find_child (ast, J_AST_TYPE_ARGUMENTS)) == NULL)
    g_error ("(" G_STRLOC "): Fix this!");

  child = j_ast_get_first_child (arguments);

  while (child != NULL)
    {
      switch (j_ast_get_type (child))
      {
        default:
          {
            ++n_arguments;
            child = j_ast_get_next_sibling (child);
            break;
          }

        case J_AST_TYPE_REDIRECT_INPUT:
        case J_AST_TYPE_REDIRECT_OUTPUT_APPEND:
        case J_AST_TYPE_REDIRECT_OUTPUT_REPLACE:
          {
            JAst* next = j_ast_get_next_sibling (child);

            j_ast_unlink (child);
            j_ast_append (ast, child);
            child = next;
            break;
          }
      }
    }

  if (min_arguments >= 0 && (n_arguments < min_arguments))
    THROWL (J_PARSER_ERROR_TOO_FEW_ARGUMENTS, "");
  else
  if (max_arguments >= 0 && (n_arguments > max_arguments))
    THROWL (J_PARSER_ERROR_TOO_MANY_ARGUMENTS, "");
return (n_arguments);
}

static JAst* walk_arguments (JWalker* walker, GError** error)
{ j_walker_dump (walker);
  GError* tmperr = NULL;
  JToken* redirect = NULL;
  JToken* token = NULL;

  JAst* ast = j_ast_new (J_AST_TYPE_ARGUMENTS);

  while ((token = j_walker_take (walker)) != NULL)
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
              j_ast_append (ast, j_ast_new_data (value));
            else
              {
                if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_APPEND)
                  j_ast_append (ast, j_ast_new_wrap (J_AST_TYPE_REDIRECT_OUTPUT_APPEND, j_ast_new_data (value)));
                else if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_READ)
                  j_ast_append (ast, j_ast_new_wrap (J_AST_TYPE_REDIRECT_INPUT, j_ast_new_data (value)));
                else if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_WRITE)
                  j_ast_append (ast, j_ast_new_wrap (J_AST_TYPE_REDIRECT_OUTPUT_REPLACE, j_ast_new_data (value)));
                  g_steal_pointer (&redirect);
              }
            break;
          }

        case J_TOKEN_TYPE_OPERATOR:
          {
            if (value != J_TOKEN_OPERATOR_EXPANSION)
              {
                if (redirect != NULL)
                  EXCPT (THROW_UNEXPECTED (token), (_j_ast_free0 (ast), NULL));
                else
                  {
                    gunichar c;
                    switch (c = g_utf8_get_char (value))
                    {
                      case (gunichar) '>':
                      case (gunichar) '<':
                        redirect = token;
                        break;
                      default: EXCPT (THROW_UNEXPECTED (token), (_j_ast_free0 (ast), NULL));
                    }
                  }
              }
            else
              {
                JWalker walker2 = J_WALKER_INIT;
                JAst* child = NULL;

                if (collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_EXPANSION, -1), G_UNLIKELY (tmperr != NULL))
                  EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
                else
                  {
                    j_walker_withdraw (&walker2);
                    j_walker_adjust (&walker2, token);

                    if ((child = walk_expansion (&walker2, token, &tmperr), j_walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
                      EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
                    else
                      {
                        if (redirect == NULL)
                          j_ast_append (ast, child);
                        else
                          {
                            if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_APPEND)
                              j_ast_append (ast, j_ast_new_wrap (J_AST_TYPE_REDIRECT_OUTPUT_APPEND, j_ast_new_data (value)));
                            else if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_READ)
                              j_ast_append (ast, j_ast_new_wrap (J_AST_TYPE_REDIRECT_INPUT, j_ast_new_data (value)));
                            else if (redirect->value == J_TOKEN_OPERATOR_REDIRECTION_WRITE)
                              j_ast_append (ast, j_ast_new_wrap (J_AST_TYPE_REDIRECT_OUTPUT_REPLACE, j_ast_new_data (value)));
                              g_steal_pointer (&redirect);
                          }
                      }
                  }
              }
            break;
          }

        default: EXCPT (THROW_UNEXPECTED (token), (_j_ast_free0 (ast), NULL));
      }
    }
return (ast);
}

static JAst* walk_command (JWalker* walker, JToken* head, GError** error)
{ j_walker_dump (walker);
  GError* tmperr = NULL;
  JAst* child = NULL;

  JAst* ast = j_ast_new (J_AST_TYPE_INVOKE);

  const guint type = head->type;
  const gchar* value = head->value;

  switch ((JTokenType) type)
    {
      case J_TOKEN_TYPE_BUILTIN:
        {
          if (value == J_TOKEN_BUILTIN_AGAIN
            || value == J_TOKEN_BUILTIN_CD
            || value == J_TOKEN_BUILTIN_EXIT
            || value == J_TOKEN_BUILTIN_FG
            || value == J_TOKEN_BUILTIN_GET
            || value == J_TOKEN_BUILTIN_HELP
            || value == J_TOKEN_BUILTIN_HISTORY
            || value == J_TOKEN_BUILTIN_UNSET)
            {
              if ((child = walk_arguments (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
                EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
              else
                {
                  j_ast_append (ast, j_ast_new_wrap (J_AST_TYPE_BUILTIN, j_ast_new_data (value)));
                  j_ast_append (ast, child);

                  if (claim_arguments (ast, -1, 1, &tmperr), G_UNLIKELY (tmperr != NULL))
                    {
                      if (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_TOO_FEW_ARGUMENTS))
                        g_propagate_prefixed_error (error, tmperr, "%d: %d: Too few arguments for '%s'", locate (head), value);
                      else
                      if (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_TOO_MANY_ARGUMENTS))
                        g_propagate_prefixed_error (error, tmperr, "%d: %d: Too many arguments for '%s'", locate (head), value);
                      else
                        g_propagate_error (error, tmperr);
                      return (_j_ast_free0 (ast), NULL);
                    }
                }
            }
          else
          if (value == J_TOKEN_BUILTIN_FALSE
            || value == J_TOKEN_BUILTIN_JOBS
            || value == J_TOKEN_BUILTIN_TRUE)
            {
              if ((child = walk_arguments (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
                EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
              else
                {
                  j_ast_append (ast, j_ast_new_wrap (J_AST_TYPE_BUILTIN, j_ast_new_data (value)));
                  j_ast_append (ast, child);

                  if (claim_arguments (ast, -1, 0, &tmperr), G_UNLIKELY (tmperr != NULL))
                    {
                      if (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_TOO_FEW_ARGUMENTS))
                        g_propagate_prefixed_error (error, tmperr, "%d: %d: Too few arguments for '%s'", locate (head), value);
                      else
                      if (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_TOO_MANY_ARGUMENTS))
                        g_propagate_prefixed_error (error, tmperr, "%d: %d: Too many arguments for '%s'", locate (head), value);
                      else
                        g_propagate_error (error, tmperr);
                      return (_j_ast_free0 (ast), NULL);
                    }
                }
            }
          else
          if (value == J_TOKEN_BUILTIN_SET)
            {
              if ((child = walk_arguments (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
                EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
              else
                {
                  guint n_arguments;
                  j_ast_append (ast, j_ast_new_wrap (J_AST_TYPE_BUILTIN, j_ast_new_data (value)));
                  j_ast_append (ast, child);

                  if ((n_arguments = claim_arguments (ast, -1, 2, &tmperr), G_LIKELY (tmperr == NULL)))
                    {
                      if (G_UNLIKELY (n_arguments == 1))
                        THROW (J_PARSER_ERROR_TOO_FEW_ARGUMENTS, "%d: %d: Too few arguments for '%s'", locate (head), value);
                    }
                  else
                    {
                      if (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_TOO_FEW_ARGUMENTS))
                        g_propagate_prefixed_error (error, tmperr, "%d: %d: Too few arguments for '%s'", locate (head), value);
                      else
                      if (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_TOO_MANY_ARGUMENTS))
                        g_propagate_prefixed_error (error, tmperr, "%d: %d: Too many arguments for '%s'", locate (head), value);
                      else
                        g_propagate_error (error, tmperr);
                      return (_j_ast_free0 (ast), NULL);
                    }
                }
            }
          break;
        }

      case J_TOKEN_TYPE_LITERAL:
        {
          if ((child = walk_arguments (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
            EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
          else
            {
              j_ast_append (ast, j_ast_new_wrap (J_AST_TYPE_TARGET, j_ast_new_data (value)));
              j_ast_append (ast, child);

              if (claim_arguments (ast, -1, -1, &tmperr), G_UNLIKELY (tmperr != NULL))
                g_error ("(" G_STRLOC "): Fix this!");
            }
          break;
        }

      case J_TOKEN_TYPE_OPERATOR:
        {
  #if DEVELOPER == 1
          g_assert (value == J_TOKEN_OPERATOR_EXPANSION);
  #endif // DEVELOPER
          JWalker walker2 = J_WALKER_INIT;
          JAst* target = NULL;

          j_walker_leave (walker, head);

          if ((child = walk_arguments (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
            EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
          else
            {
              target = j_ast_get_first_child (child);

  #if DEVELOPER == 1
              g_assert (j_ast_get_type (target) == J_AST_TYPE_EXPANSION);
  #endif // DEVELOPER

              j_ast_unlink (target);
              j_ast_append (ast, j_ast_new_wrap (J_AST_TYPE_TARGET, target));
              j_ast_append (ast, child);

              if (claim_arguments (ast, -1, -1, &tmperr), G_UNLIKELY (tmperr != NULL))
                g_error ("(" G_STRLOC "): Fix this!");
            }
          break;
        }

      default: EXCPT (THROW_UNEXPECTED (head), (_j_ast_free0 (ast), NULL));
    }
return ast;
}

static JAst* walk_expansion (JWalker* walker, JToken* head, GError** error)
{ j_walker_dump (walker);
  GError* tmperr = NULL;
  JAst* ast = NULL;

  if ((ast = walk_scope (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
    EXCPT (RETHROW (tmperr), NULL);
  else
    ast->data = GUINT_TO_POINTER (J_AST_TYPE_EXPANSION);
return ast;
}

static JAst* walk_expression (JWalker* walker, JToken* head, GError** error)
{ j_walker_dump (walker);
  GError* tmperr = NULL;
  JToken* token = NULL;

  JAst* ast = NULL;
  JAst* command = NULL;

#define SEPARATORS \
  J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_DETACH, \
  J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_EXPANSION, \
  J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_LOGICAL_AND, \
  J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_LOGICAL_OR, \
  J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_PIPE

  token = head;

  do
    {
      JWalker walker2 = J_WALKER_INIT;
      JToken* oper = NULL;

      while (TRUE)
        {
          if (collect (walker, &walker2, &tmperr, SEPARATORS, -1), G_UNLIKELY (tmperr != NULL))
            {
              if (G_LIKELY (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_UNEXPECTED_EOF)))
                _g_error_free0 (tmperr);
              else
                EXCPT (RETHROW (tmperr), (j_walker_clear (&walker2), _j_ast_free0 (ast), NULL));
            }
          else
            {
              if ((oper = j_walker_peek_back (&walker2))->value != J_TOKEN_OPERATOR_EXPANSION)
                j_walker_withdraw (&walker2);
              else
              {
                if (head->value == J_TOKEN_OPERATOR_EXPANSION)
                  continue;
                else
                {
                  if (collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_OPERATOR, J_TOKEN_OPERATOR_EXPANSION, -1), G_UNLIKELY (tmperr != NULL))
                    EXCPT (RETHROW (tmperr), (j_walker_clear (&walker2), _j_ast_free0 (ast), NULL));
                  else continue;
                }
              }
            }

          break;
        }

      G_STMT_START
        {
          j_walker_adjust (&walker2, token);

          if ((command = walk_command (&walker2, token, &tmperr), j_walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
            EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
          else
            {
              ast = (ast == NULL) ? command : (j_ast_append (ast, command), ast);

              if (oper != NULL)
              {
                const guint type = oper->type;
                const gchar* value = oper->value;

                if (value == J_TOKEN_OPERATOR_DETACH)
                  {
                    if (j_walker_length (walker) > 0)
                      EXCPT (THROW_UNEXPECTED (oper), (_j_ast_free0 (ast), NULL));
                    else
                      ast = j_ast_new_wrap (J_AST_TYPE_DETACH, ast);
                  }
                else if (value == J_TOKEN_OPERATOR_LOGICAL_AND)
                  ast = j_ast_new_wrap (J_AST_TYPE_LOGICAL_AND, ast);
                else if (value == J_TOKEN_OPERATOR_LOGICAL_OR)
                  ast = j_ast_new_wrap (J_AST_TYPE_LOGICAL_OR, ast);
                else if (value == J_TOKEN_OPERATOR_PIPE)
                  ast = j_ast_new_wrap (J_AST_TYPE_PIPE, ast);
              }
            }
        }
      G_STMT_END;
    }
#undef SEPARATORS
  while ((token = j_walker_take (walker)) != NULL);
return ast;
}

static JAst* walk_ifclosure (JWalker* walker, JToken* head, GError** error)
{ j_walker_dump (walker);
  JWalker walker2 = J_WALKER_INIT;
  JToken* then = NULL;
  GError* tmperr = NULL;

  JAst* ast = j_ast_new (J_AST_TYPE_IFCLOSURE);
  JAst* child = NULL;

  gboolean reverse = FALSE;
  guint ifcount = 1;

  if (collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_THEN, -1), G_UNLIKELY (tmperr != NULL))
    EXCPT (RETHROW (tmperr), (j_walker_clear (&walker2), _j_ast_free0 (ast), NULL));
  else
    {
      then = j_walker_peek_back (&walker2);

      j_walker_withdraw (&walker2);
      j_walker_adjust (&walker2, head);

      if (j_walker_peek_front (&walker2)->value == J_TOKEN_KEYWORD_IF)
        EXCPT (THROW_UNEXPECTED (j_walker_peek_front (&walker2)), (j_walker_clear (&walker2), _j_ast_free0 (ast), NULL));
      else
      if ((child = walk_scope (&walker2, &tmperr), j_walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
        EXCPT (RETHROW (tmperr), (j_walker_clear (&walker2), _j_ast_free0 (ast), NULL));
      else
        {
          j_ast_append (ast, (child->data = GUINT_TO_POINTER (J_AST_TYPE_IFCLOSURE_CONDITION), child));

          while (TRUE)
            {
              if (collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_IF, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_ELSE, -1), G_UNLIKELY (tmperr != NULL))
                {
                  if (G_UNLIKELY (!g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_UNEXPECTED_EOF)))
                    EXCPT (RETHROW (tmperr), (j_walker_clear (&walker2), _j_ast_free0 (ast), NULL));
                  else
                    {
                      if (ifcount > 1)
                        EXCPT (RETHROW (tmperr), (j_walker_clear (&walker2), _j_ast_free0 (ast), NULL));
                      else
                        _g_error_free0 (tmperr);
                    }
                }
              else
                {
                  const gchar* value2 = j_walker_peek_back (&walker2)->value;

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
              if (j_walker_peek_back (&walker2)->value == J_TOKEN_KEYWORD_ELSE)
                {
                  j_walker_withdraw (&walker2);
                  reverse = TRUE;
                }

              j_walker_adjust (&walker2, then);

              if ((child = walk_scope (&walker2, &tmperr), j_walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
                EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
              else
                {
                  j_ast_append (ast, (child->data = GUINT_TO_POINTER (J_AST_TYPE_IFCLOSURE_DIRECT), child));

                  if (reverse == FALSE && j_walker_length (walker) > 0)
                    EXCPT (THROW_UNEXPECTED (j_walker_take (walker)), (_j_ast_free0 (ast), NULL));
                  else if (reverse == TRUE)
                    {
                      if ((child = walk_scope (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
                        EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
                      else
                        j_ast_append (ast, (child->data = GUINT_TO_POINTER (J_AST_TYPE_IFCLOSURE_REVERSE), child));
                    }
                }
            }
          G_STMT_END;
        }
    }
return ast;
}

static JAst* walk_scope (JWalker* walker, GError** error)
{ j_walker_dump (walker);
  GError* tmperr = NULL;
  JToken* token = NULL;

  JAst* ast = j_ast_new (J_AST_TYPE_SCOPE);

  while ((token = j_walker_take (walker)) != NULL)
    {
      const guint type = token->type;
      const gchar* value = token->value;

      switch (type)
      {
        case J_TOKEN_TYPE_OPERATOR:
          {
            if (value != J_TOKEN_OPERATOR_EXPANSION)
              EXCPT (THROW_UNEXPECTED (token), (_j_ast_free0 (ast), NULL));
            G_GNUC_FALLTHROUGH;
          }
        case J_TOKEN_TYPE_BUILTIN:
        case J_TOKEN_TYPE_LITERAL:
          {
            JWalker walker2 = J_WALKER_INIT;
            JAst* expression = NULL;

            if ((collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_SEPARATOR, NULL, -1)), G_LIKELY (tmperr == NULL))
              j_walker_withdraw (&walker2);
            else
              {
                if (G_LIKELY (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_UNEXPECTED_EOF)))
                  _g_error_free0 (tmperr);
                else
                  EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
              }
            G_STMT_START
              {
                j_walker_adjust (&walker2, token);

                if ((expression = walk_expression (&walker2, token, &tmperr), j_walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
                  EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
                else
                  j_ast_append (ast, expression);
              }
            G_STMT_END;
            break;
          }

        case J_TOKEN_TYPE_KEYWORD:
          {
            if (value != J_TOKEN_KEYWORD_IF)
              EXCPT (THROW_UNEXPECTED (token), (_j_ast_free0 (ast), NULL));
            else
              {
                JWalker walker2 = J_WALKER_INIT;
                JAst* child = NULL;
                guint ifcount = 1;

                while (TRUE)
                  {
                    if (collect (walker, &walker2, &tmperr, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_IF, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_END, -1), G_UNLIKELY (tmperr != NULL))
                      EXCPT (RETHROW (tmperr), (j_walker_clear (&walker2), _j_ast_free0 (ast), NULL));
                    else
                      {
                        const gchar* value2 = j_walker_peek_back (&walker2)->value;

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
                    j_walker_withdraw (&walker2);
                    j_walker_adjust (&walker2, token);

                    if ((child = walk_ifclosure (&walker2, token, &tmperr), j_walker_clear (&walker2)), G_UNLIKELY (tmperr != NULL))
                      EXCPT (RETHROW (tmperr), (_j_ast_free0 (ast), NULL));
                    else
                      j_ast_append (ast, child);
                  }
                G_STMT_END;
              }
            break;
          }

        case J_TOKEN_TYPE_SEPARATOR:
          break;

        default: EXCPT (THROW_UNEXPECTED (token), (_j_ast_free0 (ast), NULL));
      }
    }
return ast;
}
