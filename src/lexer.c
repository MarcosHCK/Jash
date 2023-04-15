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
#include <datachannel.h>
#include <fdchannel.h>
#include <lexer.h>

#include <glib-unix.h>
#include <fcntl.h>

typedef struct _TokenClass TokenClass;
#define BLOCK_SIZ (512)
#define N_CLASSES (23)

struct _JLexer
{
  guint ref_count;
  TokenClass* classes;
};

struct _TokenClass
{
  JTokenType type;
  GRegex* pattern;
};

G_DEFINE_QUARK (j-lexer-error-quark, j_lexer_error);

static gint search (JLexer* lexer, JTokens* tokens, const gchar* input, gsize length, gsize line, gsize column, GError** error);
static gint breakdown (JLexer* lexer, JTokens* tokens, const gchar* input, gsize length, gsize line, gsize column, TokenClass* klass, GMatchInfo* info, GError** error);
#define _g_match_info_free0(var) ((var == NULL) ? NULL : (var = (g_match_info_free (var), NULL)))
#define _j_tokens_unref0(var) ((var == NULL) ? NULL : (var = (j_tokens_unref (var), NULL)))

static TokenClass token_klass (JTokenType type, const gchar* pattern)
{
  GError* tmperr = NULL;
  GRegex* regex = g_regex_new (pattern, G_REGEX_OPTIMIZE, 0, &tmperr);
                g_assert_no_error (tmperr);
  TokenClass klass = { type, regex, };
return klass;
}

static TokenClass token_klass_null ()
{
  TokenClass klass = {0};
return klass;
}

JLexer* j_lexer_new ()
{
  JLexer* self;

  self = g_slice_new (JLexer);
  self->ref_count = 1;
  self->classes = g_new (TokenClass, N_CLASSES);

  typedef gchar linecount [__LINE__ + 1];
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_COMMENT, "#(.*)");
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_QUOTED, "\"(.*?)\"");
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_QUOTED, "\'(.*?)\'");
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_AGAIN);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_CD);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_ELSE);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_END);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_EXIT);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_FALSE);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_FG);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_GET);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_HELP);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_HISTORY);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_IF);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_JOBS);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_SET);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_THEN);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_TRUE);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_UNSET);
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_OPERATOR, "(>>|&&|\\|\\|)");
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_OPERATOR, "[<>|`&]");
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_SEPARATOR, "[\n;]");
  self->classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_LITERAL, "[^\\s]+");
  G_STATIC_ASSERT (N_CLASSES == __LINE__ - sizeof (linecount));
return (self);
}

JLexer* j_lexer_ref (JLexer* lexer)
{
  g_return_val_if_fail (lexer != NULL, NULL);
  JLexer* self = (lexer);

  g_atomic_int_inc (&lexer->ref_count);
    return self;
}

void j_lexer_unref (JLexer* lexer)
{
  g_return_if_fail (lexer != NULL);
  JLexer* self = (lexer);
  guint i;

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      for (i = 0; i < N_CLASSES; ++i)
        g_regex_unref (self->classes [i].pattern);
        g_free (self->classes);
      g_slice_free (JLexer, self);
    }
}

void j_lexer_scan_from_channel (JLexer* lexer, JTokens* tokens, GIOChannel* channel, GError** error)
{
  g_return_if_fail (lexer != NULL);
  g_return_if_fail (tokens != NULL);
  g_return_if_fail (channel != NULL);
  g_return_if_fail (error == NULL || *error == NULL);
  JLexer* self = (lexer);

  GString* line = g_string_sized_new (BLOCK_SIZ);
  gsize terminator_pos, n_line = 1;
  GError* tmperr = NULL;
  GIOStatus status;

  while ((status = g_io_channel_read_line_string (channel, line, &terminator_pos, &tmperr)) != G_IO_STATUS_EOF)
    {
      switch (status)
      {
        case G_IO_STATUS_AGAIN:
          g_thread_yield ();
          continue;
        case G_IO_STATUS_ERROR:
          g_propagate_error (error, tmperr);
          g_string_free (line, TRUE);
          return;
        case G_IO_STATUS_NORMAL:
          {
            g_string_truncate (line, terminator_pos);
            g_string_append_c (line, '\n');
            search (lexer, tokens, line->str, line->len, n_line, 1, &tmperr);
            ++n_line;

            if (G_UNLIKELY (tmperr == NULL))
              break;
            else
              {
                g_propagate_error (error, tmperr);
                g_string_free (line, TRUE);
                return;
              }
          }
        default: g_assert_not_reached ();
      }
    }
}

void j_lexer_scan_from_data (JLexer* lexer, JTokens* tokens, const gchar* data, gssize length, GError** error)
{
  g_return_if_fail (lexer != NULL);
  g_return_if_fail (tokens != NULL);
  g_return_if_fail (data != NULL);
  g_return_if_fail (error == NULL || *error == NULL);
  JLexer* self = (lexer);

  length = (length >= 0) ? length : strlen (data);

  GBytes* bytes = g_bytes_new_static (data, length);
  GIOChannel* channel = j_data_channel_new_bytes (bytes);

  g_bytes_unref (bytes);

  j_lexer_scan_from_channel (self, tokens, channel, error);
  g_io_channel_shutdown (channel, 1, NULL);
  g_io_channel_unref (channel);
}

void j_lexer_scan_from_file (JLexer* lexer, JTokens* tokens, const gchar* filename, GError** error)
{
  g_return_if_fail (lexer != NULL);
  g_return_if_fail (tokens != NULL);
  g_return_if_fail (filename != NULL);
  g_return_if_fail (error == NULL || *error == NULL);
  JLexer* self = (lexer);
  gint fd, e;

  if ((fd = open (filename, O_RDONLY)) < 0)
    {
      g_set_error
      (error,
       G_IO_CHANNEL_ERROR, G_IO_CHANNEL_ERROR_FAILED,
       "Can not open file %s", filename);
      return;
    }

  GIOChannel* channel = j_fd_channel_new (fd);

  j_lexer_scan_from_channel (self, tokens, channel, error);
  g_io_channel_shutdown (channel, 1, NULL);
  g_io_channel_unref (channel);
}

static gint is_empty (const gchar* input, gsize length)
{
  gchar* ptr = (gchar*) input;
  gsize left = (gsize) strnlen (input, length);

  while (left-- > 0)
    {
      if (g_utf8_get_char (ptr) != (gunichar) ' ')
        return FALSE;

      ptr = g_utf8_next_char (ptr);
    }
return TRUE;
}

static const gchar* prepare (const gchar* input, gsize length, gsize* full_length)
{
  gchar* ptr = (gchar*) input;
  gsize left = (gsize) strnlen (input, length);
  gsize tailing = 0;
  gint tail = 0;

  while (left-- > 0)
  {
    switch (g_utf8_get_char (ptr))
    {
      case 0:
        g_assert_not_reached ();

      case ' ':
        {
          if (tail == TRUE)
            ++tailing;
          else
            {
              input = g_utf8_next_char (input);
              --length;
            }
          break;
        }

      default:
        {
          if (tail == TRUE)
            tailing = FALSE;
          else
            tail = TRUE;
          break;
        }
    }

    ptr = g_utf8_next_char (ptr);
  }
return (*full_length = length - tailing, input);
}

static gint breakdown (JLexer* self, JTokens* tokens, const gchar* input, gsize length, gsize line, gsize column, TokenClass* klass, GMatchInfo* info, GError** error)
{
  GError* tmperr = NULL;
  gint start, stop, last = 0;
  gint added = 0;

  while (g_match_info_matches (info))
    {
      g_match_info_fetch_pos (info, 0, &start, &stop);

      if (start > last)
      {
        added += search (self, tokens, input + last, start - last, line, column + last, &tmperr);

        if (G_UNLIKELY (tmperr != NULL))
          {
            g_propagate_error (error, tmperr);
            return -1;
          }
      }

      G_STMT_START
      {
        const gint type = klass->type;
        const gchar* begin = input + start;
        const gchar* value = begin;
        gsize ssize;

        switch ((JTokenType) type)
          {
            case J_TOKEN_TYPE_COMMENT:
            case J_TOKEN_TYPE_LITERAL:
              value = prepare (value, stop - start, &ssize);
              value = g_string_chunk_insert_len (tokens->chunk, value, ssize);
              break;

            case J_TOKEN_TYPE_QUOTED:
              value = prepare (value + 1, stop - start - 2, &ssize);
              value = g_string_chunk_insert_len (tokens->chunk, value, ssize);
              break;

            default:
              {
                gchar* static_value = (gchar*) prepare (begin, stop - start, &ssize);
                gchar* dynamic_value = (gchar*) g_strndup (static_value, ssize);
                gchar* intern_value = (gchar*) g_intern_string (dynamic_value);
                g_free ((value = intern_value, dynamic_value));
                break;
              }
          }

        const guint coloff = column + start;
        const JToken token = { type, line, coloff, value, };

        g_array_append_val (&tokens->array->g_array, token);
        ++added;
      }
      G_STMT_END;

      g_match_info_next (info, &tmperr);
      last = stop;

      if (G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          return -1;
        }
    }

  if (length > last)
    {
      added += search (self, tokens, input + last, length - last, line, column + last, &tmperr);

      if (G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          return -1;
        }
    }
return (added);
}

static gint search (JLexer* self, JTokens* tokens, const gchar* input, gsize length, gsize line, gsize column, GError** error)
{
  TokenClass* klass = NULL;
  GMatchInfo* info = NULL;
  GError* tmperr = NULL;
  gint added = 0;
  gint got, i;

  for (i = 0; i < N_CLASSES; ++i)
  {
    klass = & self->classes [i];
    got = g_regex_match_full (klass->pattern, input, length, 0, 0, &info, &tmperr);

    if (G_UNLIKELY (tmperr != NULL))
      {
        g_propagate_error (error, tmperr);
        _g_match_info_free0 (info);
        return -1;
      }

    if (got == FALSE)
      {
        _g_match_info_free0 (info);
        continue;
      }
    else
      {
        added += breakdown (self, tokens, input, length, line, column, klass, info, &tmperr);
                g_match_info_free (info);

        if (G_UNLIKELY (tmperr == NULL))
          break;
        else
          {
            g_propagate_error (error, tmperr);
            return -1;
          }
      }
  }

  if (added == 0 && !is_empty (input, length))
  {
    const gsize size = length;
    const gchar* value = prepare (input, size, &length);

    g_set_error
    (error, J_LEXER_ERROR, J_LEXER_ERROR_UNKNOWN_TOKEN,
     "%i: %i: unknown token %.*s", (int) line, (int) column, (int) length, value);
    return -1;
  }
return (added);
}
