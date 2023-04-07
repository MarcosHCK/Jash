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
#include <parser.h>

struct _JParser
{
  guint ref_count;

  GPtrArray* codes;
};

struct _Walker
{
  JParser* self;
  JToken* last;
  GQueue* tokens;
};

enum
{
  COND_AND,
  COND_OR,
};

typedef struct _Walker Walker;
G_DEFINE_QUARK (j-parser-error-quark, j_parser_error);

static void walk_args (Walker* walker, GError** error);
static void walk_command (Walker* walker, GError** error);
static void walk_expansion (Walker* walker, GError** error);
static void walk_ifelse (Walker* walker, GError** error);
static void walk_scope (Walker* walker, GError** error);
static void profile_code (JParser* self, GError** error);
#define _g_array_unref0(var) ((var == NULL) ? NULL : (var = (g_array_unref (var), NULL)))
#define _g_error_free0(var) ((var == NULL) ? NULL : (var = (g_error_free (var), NULL)))
#define _j_parser_unref0(var) ((var == NULL) ? NULL : (var = (j_parser_unref (var), NULL)))

JParser* j_parser_new ()
{
  JParser* self;

  self = g_slice_new (JParser);
  self->ref_count = 1;
  self->codes = g_ptr_array_new_with_free_func ((GDestroyNotify) j_code_unref);
return (self);
}

JParser* j_parser_new_from_tokens (JToken* tokens, guint n_tokens, GError** error)
{
  JParser* self = j_parser_new ();
  GError* tmperr = NULL;

  Walker walker = {0};
  GQueue queue = G_QUEUE_INIT;
  gint i;

  for (i = 0; i < n_tokens; ++i)
    {
      if (tokens [i].type != J_TOKEN_TYPE_COMMENT)
        g_queue_push_tail (&queue, &tokens[i]);
    }

  walker.self = self;
  walker.last = g_queue_peek_tail (&queue);
  walker.tokens = &queue;

  walk_scope (&walker, &tmperr);
  g_queue_clear (&queue);

  if (G_UNLIKELY ((tmperr != NULL)))
    {
      g_propagate_error (error, tmperr);
      _j_parser_unref0 (self);
    }
  else
    {
      profile_code (self, &tmperr);

      if (G_UNLIKELY ((tmperr != NULL)))
        {
          g_propagate_error (error, tmperr);
          _j_parser_unref0 (self);
        }
    }
return self;
}

JParser* j_parser_ref (JParser* parser)
{
  g_return_val_if_fail (parser != NULL, NULL);
  JParser* self = (parser);
return (g_atomic_int_inc (&self->ref_count), self);
}

void j_parser_unref (JParser* parser)
{
  g_return_if_fail (parser != NULL);
  JParser* self = (parser);

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      g_ptr_array_unref (self->codes);
      g_slice_free (JParser, self);
    }
}

JCode** j_parser_get_codes (JParser* parser, guint* n_codes)
{
  g_return_val_if_fail (parser != NULL, NULL);
  JParser* self = (parser);
  guint nothing;

  *((n_codes) ? n_codes : &nothing) = self->codes->len;
return (JCode**) self->codes->pdata;
}

#define g_set_error(...) \
    G_STMT_START { \
      g_printerr ("(" G_STRLOC "): here (g_set_error)!\n"); \
      (g_set_error) (__VA_ARGS__); \
    } G_STMT_END
#define g_propagate_error(dst,src) \
    G_STMT_START { \
      GError** __dst = ((dst)); \
      GError* __src = ((src)); \
      g_printerr ("(" G_STRLOC "): here (g_propagate_error (0x%" G_GINT64_MODIFIER "x, 0x%" G_GINT64_MODIFIER "x))!\n", (guintptr) __dst, (guintptr) __src); \
      (g_propagate_error) (__dst, __src); \
    } G_STMT_END

static JCode* pushcode_va (Walker* walker, JCodeType type, va_list l)
{
  JCode* code = NULL;

  switch (type)
    {
      case J_CODE_TYPE_DUMP:
      case J_CODE_TYPE_END:
      case J_CODE_TYPE_LF:
      case J_CODE_TYPE_LT:
      case J_CODE_TYPE_PAP:
      case J_CODE_TYPE_LSET:
      case J_CODE_TYPE_PIPE:
      case J_CODE_TYPE_PSI:
      case J_CODE_TYPE_PSO:
      case J_CODE_TYPE_SYNC:
      case J_CODE_TYPE_PPRC:
        code = j_code_new_simple (type);
        break;
      case J_CODE_TYPE_IF:
      case J_CODE_TYPE_IFN:
        code = j_code_new_int (type, va_arg (l, gint));
        break;
      case J_CODE_TYPE_EXEC:
      case J_CODE_TYPE_FSI:
      case J_CODE_TYPE_FSO:
      case J_CODE_TYPE_FSOA:
      case J_CODE_TYPE_GET:
      case J_CODE_TYPE_PAS:
      case J_CODE_TYPE_SET:
      case J_CODE_TYPE_USET:
        code = j_code_new_string (type, va_arg (l, gchar*));
        break;
      default: g_assert_not_reached ();
    }
return (g_ptr_array_add (walker->self->codes, code), code);
}

static JCode* pushcode (Walker* walker, JCodeType type, ...)
{
  JCode* code;
  va_list l;

  va_start (l, type);
  code = pushcode_va (walker, type, l);
return (va_end (l), code);
}

static JCode* pushmetacode_va (Walker* walker, gint meta, const gchar* fmt, va_list l)
{
  JCode* code = NULL;

  if (g_str_equal (fmt, "n"))
    code = j_code_new_simple (0);
  else
  if (g_str_equal (fmt, "i"))
    code = j_code_new_int (0, va_arg (l, gint));
  else
  if (g_str_equal (fmt, "u"))
    code = j_code_new_uint (0, va_arg (l, guint));
  else
  if (g_str_equal (fmt, "s"))
    code = j_code_new_string (0, va_arg (l, gchar*));

  code->type = J_CODE_TYPE_MAX_CODE + meta;
#if DEVELOPER == 1
  g_assert (J_CODE_TYPE_MAX_CODE + meta < (G_MAXUINT >> 5));
#endif // DEVELOPER
return (g_ptr_array_add (walker->self->codes, code), code);
}

static JCode* pushmetacode (Walker* walker, gint meta, const gchar* fmt, ...)
{
  JCode* code;
  va_list l;

  va_start (l, fmt);
  code = pushmetacode_va (walker, meta, fmt, l);
return (va_end (l), code);
}

#define locate(token) token->line,token->column
#define EXCPT(before,after) G_STMT_START { G_STMT_START { before; } G_STMT_END; return after; } G_STMT_END
#define THROW(code,...) (G_GNUC_EXTENSION ({ g_set_error (error, J_PARSER_ERROR, (code), __VA_ARGS__); }))
#define RETHROW(tmperr) (G_GNUC_EXTENSION ({ g_propagate_error (error, (tmperr)); }))

#define THROW_EOS() THROW (J_PARSER_ERROR_UNEXPECTED_EOF, "%i: %i: Unexpected end of scope", locate (walker->last))
#define THROW_UNEXPECTED(token) ({ JToken* __token = ((token)); THROW (J_PARSER_ERROR_UNEXPECTED_TOKEN, "%i: %i: Unexpected token '%s'", locate (__token), __token->value); })

static gint detectbase (const gchar* value)
{
  if (g_utf8_get_char (value) == (gunichar) '-')
    return detectbase (value);
  else
    {
      gchar* n = g_utf8_next_char (value);
      gunichar c1 = g_utf8_get_char (value);
      gunichar c2 = g_utf8_get_char (n);

      if (c1 != (gunichar) '0')
        return 10;
      else
        {
          switch (c2)
            {
              case (gunichar) 'b': return 2;
              case (gunichar) 'o': return 8;
              case (gunichar) 'd': return 10;
              case (gunichar) 'x': return 16;
              default: return 10;
            }
        }
    }
}

static gint parseint (const gchar* value, GError** error)
{
  gint base = detectbase (value);
  gint64 result = -1;

  g_ascii_string_to_signed (value, base, G_MININT, G_MAXINT, &result, error);
return (gint) result;
}

static gint collect_va (Walker* walker, GQueue* queue, GError** error, va_list l)
{
  struct _Until
  {
    JTokenType type;
    gchar* expected;
  };

  struct _Until  untils_stat [32];
  struct _Until* untils = untils_stat;
  GArray* untils_dyn = NULL;

  gint type, j, i = 0;
  gchar* expected;

  while ((type = va_arg (l, gint)) >= 0)
    {
      expected = va_arg (l, gchar*);

      if (G_N_ELEMENTS (untils_stat) >= i)
        {
          untils_stat [i].type = type;
          untils_stat [i].expected = expected;
        }
      else
        {
          if (untils_dyn == NULL)
            {
              untils_dyn = g_array_sized_new (0, 0, sizeof (struct _Until), G_N_ELEMENTS (untils_stat));
                           g_array_append_vals (untils_dyn, untils_stat, G_N_ELEMENTS (untils_stat));
            }

          untils_stat [0].type = type;
          untils_stat [0].expected = expected;

          g_array_append_val (untils_dyn, untils_stat [0]);
          untils = (gpointer) untils_dyn->data;
        }

      ++i;
    }

  JToken* token;

  while ((token = g_queue_pop_head (walker->tokens)) != NULL)
    {
      g_queue_push_tail (queue, token);

      for (j = 0; j < i; ++j)
      {
        type = untils [j].type;
        expected = untils [j].expected;

        if (type == token->type && (expected == NULL || (expected == token->value || g_str_equal (expected, token->value))))
          {
            _g_array_unref0 (untils_dyn);
            return (gint) queue->length;
          }
      }
    }
return (THROW_EOS (), _g_array_unref0 (untils_dyn), -1);
}

static gint collect (Walker* walker, GQueue* queue, GError** error, ...)
{
  gint count;
  va_list l;

          va_start (l, error);
  count = collect_va (walker, queue, error, l);
return (va_end (l), count);
}

static void walk_args (Walker* walker, GError** error)
{
  GError* tmperr = NULL;
  JToken* redirection = NULL;
  JToken* token = NULL;

  while ((token = g_queue_pop_head (walker->tokens)) != NULL)
    {
      const gint type = token->type;
      const gchar* value = token->value;

      switch ((JTokenType) type)
        {
          case J_TOKEN_TYPE_BUILTIN:
          case J_TOKEN_TYPE_KEYWORD:
          case J_TOKEN_TYPE_LITERAL:
          case J_TOKEN_TYPE_QUOTED:
            {
              if (redirection == NULL)
                pushcode (walker, J_CODE_TYPE_PAS, value);
              else
                {
                  const gchar* r1 = redirection->value;
                  const gchar* r2 = g_utf8_next_char (r1);

                  if (g_utf8_get_char (r1) == (gunichar) '<')
                    pushcode (walker, J_CODE_TYPE_FSI, value);
                  else
                  if (g_utf8_get_char (r1) == (gunichar) '>')
                    {
                      if (g_utf8_get_char (r2) == (gunichar) '\0')
                        pushcode (walker, J_CODE_TYPE_FSO, value);
                      else
                      if (g_utf8_get_char (r2) == (gunichar) '>')
                        pushcode (walker, J_CODE_TYPE_FSOA, value);
                    }
                  g_steal_pointer (&redirection);
                }
              break;
            }

          case J_TOKEN_TYPE_OPERATOR:
            {
              if (g_utf8_get_char (value) == (gunichar) '>'
                || g_utf8_get_char (value) == (gunichar) '<')
                redirection = token;
              else
              if (g_utf8_get_char (value) == (gunichar) '`')
                {
                  GQueue tokens2 = G_QUEUE_INIT;
                  Walker walker2 = { walker->self, NULL, &tokens2, };

                  if ((collect (walker, &tokens2, &tmperr, J_TOKEN_TYPE_OPERATOR, "`", -1)), G_UNLIKELY (tmperr != NULL))
                    EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                  else
                    {
                      if ((g_queue_pop_tail (&tokens2), g_queue_get_length (&tokens2)) == 0)
                        EXCPT (THROW (J_PARSER_ERROR_EXPECTED_TOKEN, "%i: %i: Expected token after '%s'", locate (token), value), g_queue_clear (&tokens2));
                      else
                        {
                          walker2.last = g_queue_peek_tail (&tokens2);

                          if ((walk_expansion (&walker2, &tmperr), g_queue_clear (&tokens2)), G_UNLIKELY (tmperr != NULL))
                            EXCPT (RETHROW (tmperr),);
                        }
                    }
                }
              else
                EXCPT (THROW_UNEXPECTED (token),);
              break;
            }

          default:
            {
              EXCPT (THROW_UNEXPECTED (token),);
              break;
            }
        }
    }

  if (redirection != NULL) EXCPT (THROW_EOS (),);
}

static void walk_command (Walker* walker, GError** error)
{
  GError* tmperr = NULL;
  JToken* token = NULL;

#define SEPARATORS \
  J_TOKEN_TYPE_OPERATOR, "&", \
  J_TOKEN_TYPE_OPERATOR, "|", \
  J_TOKEN_TYPE_OPERATOR, "&&", \
  J_TOKEN_TYPE_OPERATOR, "||" \

  struct
  {
    guint start_index;
    guint cond_type : (sizeof (guint) * 8 - 1);
    guint active : 1;
  } condition = {0};

  while ((token = g_queue_pop_head (walker->tokens)) != NULL)
    {
      GQueue tokens2 = G_QUEUE_INIT;
      Walker walker2 = { walker->self, NULL, &tokens2, };
      JToken* oper = NULL;

      if ((collect (walker, &tokens2, &tmperr, SEPARATORS, -1)), G_UNLIKELY (tmperr == NULL))
        oper = g_queue_pop_tail (&tokens2);
      else
        {
          if (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_UNEXPECTED_EOF))
            _g_error_free0 (tmperr);
          else
            EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
        }
      G_STMT_START
        {
          const gint type = token->type;
          const gchar* value = token->value;

          const gboolean in_pipe = (oper == NULL) ? FALSE : ! g_strcmp0 (oper->value, "|");
          const gboolean in_dtch = (oper == NULL) ? FALSE : ! (in_pipe || g_strcmp0 (oper->value, "&"));
          const gboolean in_cor = (oper == NULL) ? FALSE : ! (in_pipe || g_strcmp0 (oper->value, "||"));
          const gboolean in_cand = (oper == NULL) ? FALSE : ! (in_cor || g_strcmp0 (oper->value, "&&"));

          walker2.last = g_queue_peek_tail (&tokens2);

          if (in_pipe)
            {
              pushcode (walker, J_CODE_TYPE_PIPE);
              pushcode (walker, J_CODE_TYPE_PSO);
            }

          switch ((JTokenType) type)
            {
              case J_TOKEN_TYPE_BUILTIN:
                {
                  if (value == J_TOKEN_BUILTIN_AGAIN
                    || value == J_TOKEN_BUILTIN_HELP
                    || value == J_TOKEN_BUILTIN_HISTORY
                    || value == J_TOKEN_BUILTIN_JOBS)
                    {
                      pushcode (walker, J_CODE_TYPE_PAS, value);

                      if ((walk_args (&walker2, &tmperr)), G_UNLIKELY (tmperr != NULL))
                        EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                      else
                        pushcode (walker, J_CODE_TYPE_EXEC, g_get_prgname ());
                    }
                  else
                    {
                      if (value == J_TOKEN_BUILTIN_CD)
                        {
                          if (tokens2.length > 1)
                            EXCPT (THROW_UNEXPECTED (g_queue_peek_nth (&tokens2, 1)), g_queue_clear (&tokens2));
                          else
                          if (tokens2.length == 0)
                            EXCPT (THROW (J_PARSER_ERROR_EXPECTED_TOKEN, "%i: %i: cd expects an argument", locate (token)), g_queue_clear (&tokens2));
                          else
                            {
                              if ((walk_args (&walker2, &tmperr)), G_UNLIKELY (tmperr != NULL))
                                EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                              else
                                {
                                  JCode* code = g_ptr_array_steal_index (walker->self->codes, walker->self->codes->len - 1);
                                                pushmetacode (walker, J_CODE_META_CD, "s", code->string_argument);
                                                j_code_unref (code);
                                }
                            }
                        } else
                      if (value == J_TOKEN_BUILTIN_EXIT)
                        {
                          if (tokens2.length > 1)
                            EXCPT (THROW_UNEXPECTED (g_queue_peek_nth (&tokens2, 1)), g_queue_clear (&tokens2));
                          else
                          if (tokens2.length == 0)
                            pushmetacode (walker, J_CODE_META_EXIT, "i", 0);
                          else
                          if (tokens2.length == 1)
                            {
                              if ((walk_args (&walker2, &tmperr)), G_UNLIKELY (tmperr != NULL))
                                EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                              else
                                {
                                  JCode* code = g_ptr_array_steal_index (walker->self->codes, walker->self->codes->len - 1);
                                  gint result = parseint (code->string_argument, &tmperr);

                                  if (G_UNLIKELY (tmperr != NULL))
                                    EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                                  else
                                    pushmetacode (walker, J_CODE_META_EXIT, "i", result);
                                }
                            }
                        } else
                      if (value == J_TOKEN_BUILTIN_FALSE)
                        {
                          if (tokens2.length > 0)
                            EXCPT (THROW_UNEXPECTED (g_queue_pop_head (&tokens2)), g_queue_clear (&tokens2));
                          else
                            pushcode (walker, J_CODE_TYPE_LF);
                        } else
                      if (value == J_TOKEN_BUILTIN_FG)
                        {
                          if (tokens2.length > 1)
                            EXCPT (THROW_UNEXPECTED (g_queue_peek_nth (&tokens2, 1)), g_queue_clear (&tokens2));
                          else
                          if (tokens2.length == 0)
                            pushmetacode (walker, J_CODE_META_FG, "i", -1);
                          else
                          if (tokens2.length == 1)
                            {
                              if ((walk_args (&walker2, &tmperr)), G_UNLIKELY (tmperr != NULL))
                                EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                              else
                                {
                                  JCode* code = g_ptr_array_steal_index (walker->self->codes, walker->self->codes->len - 1);
                                  gint result = parseint (code->string_argument, &tmperr);

                                  if (G_UNLIKELY (tmperr != NULL))
                                    EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                                  else
                                    pushmetacode (walker, J_CODE_META_FG, "i", result);
                                }
                            }
                        } else
                      if (value == J_TOKEN_BUILTIN_GET)
                        {
                          if (tokens2.length > 1)
                            EXCPT (THROW_UNEXPECTED (g_queue_peek_nth (&tokens2, 1)), g_queue_clear (&tokens2));
                          else
                          if (tokens2.length == 0)
                            EXCPT (THROW (J_PARSER_ERROR_EXPECTED_TOKEN, "%i: %i: get expects an argument", locate (token)), g_queue_clear (&tokens2));
                          else
                            {
                              if ((walk_args (&walker2, &tmperr)), G_UNLIKELY (tmperr != NULL))
                                EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                              else
                                {
                                  JCode* code = g_ptr_array_steal_index (walker->self->codes, walker->self->codes->len - 1);
                                                pushcode (walker, J_CODE_TYPE_GET, code->string_argument);
                                                j_code_unref (code);
                                }
                            }
                        } else
                      if (value == J_TOKEN_BUILTIN_SET)
                        {
                          if (tokens2.length > 2)
                            EXCPT (THROW_UNEXPECTED (g_queue_peek_nth (&tokens2, 2)), g_queue_clear (&tokens2));
                          else
                          if (tokens2.length == 2)
                            {
                              if ((walk_args (&walker2, &tmperr)), G_UNLIKELY (tmperr != NULL))
                                EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                              else
                                {
                                  JCode* code2 = g_ptr_array_steal_index (walker->self->codes, walker->self->codes->len - 1);
                                  JCode* code1 = g_ptr_array_steal_index (walker->self->codes, walker->self->codes->len - 1);
                                                  pushcode (walker, J_CODE_TYPE_PAS, code2->string_argument);
                                                  pushcode (walker, J_CODE_TYPE_SET, code1->string_argument);
                                                  j_code_unref (code1), j_code_unref (code2);
                                }
                            }
                          else
                          if (tokens2.length == 0)
                            {
                              pushcode (walker, J_CODE_TYPE_LSET);
                              pushcode (walker, J_CODE_TYPE_DUMP);
                            }
                          else
                            {
                              g_queue_clear (&tokens2);
                              EXCPT (THROW (J_PARSER_ERROR_EXPECTED_TOKEN, "%i: %i: set expects two arguments (or none)", locate (token)), g_queue_clear (&tokens2));
                            }
                        } else
                      if (value == J_TOKEN_BUILTIN_TRUE)
                        {
                          if (tokens2.length > 0)
                            EXCPT (THROW_UNEXPECTED (g_queue_pop_head (&tokens2)), g_queue_clear (&tokens2));
                          else
                            pushcode (walker, J_CODE_TYPE_LT);
                        } else
                      if (value == J_TOKEN_BUILTIN_UNSET)
                        {
                          if (tokens2.length > 1)
                            EXCPT (THROW_UNEXPECTED (g_queue_peek_nth (&tokens2, 1)), g_queue_clear (&tokens2));
                          else
                          if (tokens2.length == 0)
                            EXCPT (THROW (J_PARSER_ERROR_EXPECTED_TOKEN, "%i: %i: unset expects an argument", locate (token)), g_queue_clear (&tokens2));
                          else
                            {
                              if ((walk_args (&walker2, &tmperr)), G_UNLIKELY (tmperr != NULL))
                                EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                              else
                                {
                                  JCode* code = g_ptr_array_steal_index (walker->self->codes, walker->self->codes->len - 1);
                                                pushcode (walker, J_CODE_TYPE_USET, code->string_argument);
                                                j_code_unref (code);
                                }
                            }
                        }
                    }
                  break;
                }

              case J_TOKEN_TYPE_LITERAL:
                {
                  pushcode (walker, J_CODE_TYPE_PAS, value);

                  if ((walk_args (&walker2, &tmperr)), G_UNLIKELY (tmperr != NULL))
                    EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                  else
                    pushcode (walker, J_CODE_TYPE_EXEC, token->value);
                  break;
                }

              default:
                EXCPT (THROW_UNEXPECTED (token), g_queue_clear (&tokens2));
            }

          if (in_pipe)
            {
              pushcode (walker, J_CODE_TYPE_PSI);
            }

#define COND_FINISH() \
      G_STMT_START \
        { \
          if (condition.active == TRUE) \
            { \
 ; \
              switch (condition.cond_type) \
              { \
                case COND_AND: \
                  { \
                    GPtrArray* codes = walker->self->codes; \
                    guint offset = codes->len - condition.start_index; \
                    JCode* code = j_code_new_uint (J_CODE_TYPE_IFN, offset); \
                    g_ptr_array_insert (codes, condition.start_index, code); \
                    break; \
                  } \
 ; \
                case COND_OR: \
                  { \
                    GPtrArray* codes = walker->self->codes; \
                    guint offset = codes->len - condition.start_index; \
                    JCode* code = j_code_new_uint (J_CODE_TYPE_IF, offset); \
                    g_ptr_array_insert (codes, condition.start_index, code); \
                    break; \
                  } \
 ; \
                default: g_assert_not_reached (); \
              } \
 ; \
              pushcode (walker, J_CODE_TYPE_END); \
            } \
        } \
      G_STMT_END

          if (in_cand || in_cor)
            {
              if (walker->tokens->length == 0)
                EXCPT (THROW_EOS (), g_queue_clear (&tokens2));
              else
                {
                  COND_FINISH ();
                  pushcode (walker, J_CODE_TYPE_SYNC);
                  pushcode (walker, J_CODE_TYPE_PPRC);

                  const guint cond1 = ((in_cand) ? COND_AND : 0);
                  const guint cond2 = ((in_cor) ? COND_OR : 0);

                  condition.start_index = walker->self->codes->len;
                  condition.cond_type = cond1 | cond2;
                  condition.active = TRUE;
                }
            }

          if (in_dtch)
            {
              if (walker->tokens->length != 0)
                EXCPT (THROW_UNEXPECTED (oper), g_queue_clear (&tokens2));
              else
                pushmetacode (walker, J_CODE_META_DETACH, "n");
            }

          g_queue_clear (&tokens2);
        }
#undef SEPARATORS
      G_STMT_END;
    }

  COND_FINISH ();
  pushcode (walker, J_CODE_TYPE_SYNC);
}

static void walk_expansion (Walker* walker, GError** error)
{
  GError* tmperr = NULL;

  if ((walk_scope (walker, &tmperr)), G_UNLIKELY (tmperr != NULL))
    EXCPT (RETHROW (tmperr),);
  else
    {
      GPtrArray* codes = walker->self->codes;
      JCode* code = NULL;
      guint i, length = codes->len;

      for (i = 0; i < length; ++i)
        {
          if ((code = g_ptr_array_index (codes, length - i - 1))->type == J_CODE_TYPE_EXEC)
            {
              
              g_ptr_array_insert (codes, length - i - 2, j_code_new_simple (J_CODE_TYPE_PSO));
              g_ptr_array_insert (codes, length - i - 2, j_code_new_simple (J_CODE_TYPE_PIPE));
              pushcode (walker, J_CODE_TYPE_DUMP);
              break;
            }
        }
    }
}

static void walk_ifelse (Walker* walker, GError** error)
{
  GQueue tokens2 = G_QUEUE_INIT;
  Walker walker2 = { walker->self, NULL, &tokens2, };
  GError* tmperr = NULL;

  if ((collect (walker, &tokens2, &tmperr, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_THEN, -1)), G_UNLIKELY (tmperr != NULL))
    EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
  else
  if (tokens2.length == 1)
    {
      const JToken* token = g_queue_peek_tail (&tokens2);
      EXCPT (THROW (J_PARSER_ERROR_EXPECTED_TOKEN, "%i: %i: Expected token before '%s'", locate (token)), g_queue_clear (&tokens2));
    }
  else
    {
                      g_queue_pop_tail (&tokens2);
      walker2.last = g_queue_peek_tail (&tokens2);

      if ((walk_scope (&walker2, &tmperr), g_queue_clear (&tokens2)), G_UNLIKELY (tmperr != NULL))
        EXCPT (RETHROW (tmperr),);
      else
        {
          pushcode (walker, J_CODE_TYPE_PPRC);
          gboolean continue_ = TRUE;
          gboolean direct = TRUE;

          do
          {
            if ((collect (walker, &tokens2, &tmperr, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_ELSE, -1)), G_UNLIKELY (tmperr == NULL))
              {
                if (direct == FALSE)
                  EXCPT (THROW_UNEXPECTED (g_queue_peek_tail (&tokens2)), g_queue_clear (&tokens2));
                else
                  g_queue_pop_tail (&tokens2);
              }
            else
              {
                if (!g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_UNEXPECTED_EOF))
                  EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                else
                  {
                    continue_ = FALSE;
                    _g_error_free0 (tmperr);
                  }
              }

            GPtrArray* codes = walker->self->codes;
            JCodeType type = J_CODE_TYPE_IF;
            JCode* code = NULL;
            guint start_index = codes->len;
            guint offset = 0;

            walker2.last = g_queue_peek_tail (&tokens2);

            if ((walk_scope (&walker2, &tmperr), g_queue_clear (&tokens2)), G_UNLIKELY (tmperr != NULL))
              EXCPT (RETHROW (tmperr),);
            else
              {
                offset = codes->len - start_index;
                type = (direct) ? type : J_CODE_TYPE_IFN;
                code = j_code_new_uint (type, offset);
                direct = FALSE;

                g_ptr_array_insert (codes, start_index, code);
              }
          } while ((continue_ == TRUE) || (pushcode (walker, J_CODE_TYPE_END), FALSE));
        }
    }
}

static void walk_scope (Walker* walker, GError** error)
{
  GError* tmperr = NULL;
  JToken* token = NULL;

  while ((token = g_queue_pop_head (walker->tokens)) != NULL)
    {
      const gint type = token->type;
      const gchar* value = token->value;

      switch ((JTokenType) type)
        {
          case J_TOKEN_TYPE_BUILTIN:
          case J_TOKEN_TYPE_LITERAL:
            {
              GQueue tokens2 = G_QUEUE_INIT;
              Walker walker2 = { walker->self, NULL, &tokens2, };

              if ((collect (walker, &tokens2, &tmperr, J_TOKEN_TYPE_SEPARATOR, NULL, -1)), G_UNLIKELY (tmperr == NULL))
                g_queue_pop_tail (&tokens2);
              else
                {
                  if (g_error_matches (tmperr, J_PARSER_ERROR, J_PARSER_ERROR_UNEXPECTED_EOF))
                    _g_error_free0 (tmperr);
                  else
                    EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                }
              G_STMT_START
                {
                                 g_queue_push_head (&tokens2, token);
                  walker2.last = g_queue_peek_tail (&tokens2);

                  if ((walk_command (&walker2, &tmperr), g_queue_clear (&tokens2)), G_UNLIKELY (tmperr != NULL))
                    EXCPT (RETHROW (tmperr),);
                }
              G_STMT_END;
              break;
            }

          case J_TOKEN_TYPE_KEYWORD:
            {
              if (value != J_TOKEN_KEYWORD_IF)
                EXCPT (THROW_UNEXPECTED (token),);
              else
                {
                  GQueue tokens2 = G_QUEUE_INIT;
                  Walker walker2 = { walker->self, NULL, &tokens2, };

                  if ((collect (walker, &tokens2, &tmperr, J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_END, -1)), G_UNLIKELY (tmperr != NULL))
                    EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                  else
                    {
                      g_queue_pop_tail (&tokens2);
                      walker2.last = g_queue_peek_tail (&tokens2);

                      if ((walk_ifelse (&walker2, &tmperr), g_queue_clear (&tokens2)), G_UNLIKELY (tmperr != NULL))
                        EXCPT (RETHROW (tmperr),);
                    }
                }
              break;
            }

          case J_TOKEN_TYPE_OPERATOR:
            {
              if (g_utf8_get_char (value) != (gunichar) '`')
                EXCPT (THROW_UNEXPECTED (token),);
              else
                {
                  GQueue tokens2 = G_QUEUE_INIT;
                  Walker walker2 = { walker->self, NULL, &tokens2, };

                  if ((collect (walker, &tokens2, &tmperr, J_TOKEN_TYPE_OPERATOR, "`", -1)), G_UNLIKELY (tmperr != NULL))
                    EXCPT (RETHROW (tmperr), g_queue_clear (&tokens2));
                  else
                    {
                      pushcode (walker, J_CODE_TYPE_PAS, g_intern_static_string ("jash"));

                      g_queue_pop_tail (&tokens2);
                      walker2.last = g_queue_peek_tail (&tokens2);

                      if ((walk_expansion (&walker2, &tmperr), g_queue_clear (&tokens2)), G_UNLIKELY (tmperr != NULL))
                        EXCPT (RETHROW (tmperr),);

                      g_assert_not_reached ();
                    }
                }
              break;
            }

          case J_TOKEN_TYPE_SEPARATOR:
            break;

          default: EXCPT (THROW_UNEXPECTED (token),); break;
        }
    }
}

static void profile_code (JParser* self, GError** error)
{
  guint n_codes, i;
  JCode** codes = j_parser_get_codes (self, &n_codes);
  GError* tmperr = NULL;

  for (i = 0; i < n_codes; ++i)
    {
#define code_remove(index) \
  G_STMT_START \
      { \
        guint __index = ((index)); \
 ; \
        if (i >= __index) \
          --i; \
          --n_codes; \
 ; \
        g_ptr_array_remove_index (self->codes, __index); \
      } \
  G_STMT_END

      switch (codes [i]->type)
        {
          case J_CODE_TYPE_IF:
          case J_CODE_TYPE_IFN:
            {
              if (codes [i]->int_argument == 0)
                code_remove (i);
              break;
            }
        }
#undef code_remove
    }
}
