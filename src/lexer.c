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
static const int CHUNK_MLT = 64;
static const int BLOCK_SIZ = 512;

struct _JLexer
{
  guint ref_count;

  GArray* tokens;
  GStringChunk* chunk;
};

struct _TokenClass
{
  JTokenType type;
  GRegex* pattern;
};

G_DEFINE_QUARK (j-lexer-error-quark, j_lexer_error);

static gint search (JLexer* lexer, const gchar* input, gsize length, gsize line, gsize column, GError** error);
static gint breakdown (JLexer* lexer, const gchar* input, gsize length, gsize line, gsize column, TokenClass* klass, GMatchInfo* info, GError** error);
#define _g_match_info_free0(var) ((var == NULL) ? NULL : (var = (g_match_info_free (var), NULL)))
#define _j_lexer_unref0(var) ((var == NULL) ? NULL : (var = (j_lexer_unref (var), NULL)))

JLexer* j_lexer_new ()
{
  JLexer* self;

  self = g_slice_new (JLexer);
  self->ref_count = 1;
  self->tokens = g_array_new (0, 0, sizeof (JToken));
  self->chunk = g_string_chunk_new (CHUNK_MLT);
return self;
}

JLexer* j_lexer_new_from_channel (GIOChannel* channel, GError** error)
{
  g_return_val_if_fail (channel != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  GString* line = g_string_sized_new (BLOCK_SIZ);
  JLexer* lexer = j_lexer_new ();
  gsize terminator_pos, line_n = 1;
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
          j_lexer_unref (lexer);
          return NULL;
        case G_IO_STATUS_NORMAL:
          {
            g_string_truncate (line, terminator_pos);
            search (lexer, line->str, line->len, line_n, 1, &tmperr);
            ++line_n;

            if (G_UNLIKELY (tmperr == NULL))
              break;
            else
              {
                g_propagate_error (error, tmperr);
                g_string_free (line, TRUE);
                j_lexer_unref (lexer);
                return NULL;
              }
          }
      }
    }
return (lexer);
}

JLexer* j_lexer_new_from_data (const gchar* data, gssize length, GError** error)
{
  g_return_val_if_fail (data != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  JLexer* lexer = NULL;
  GBytes* bytes = g_bytes_new_static (data, length);
  GIOChannel* channel = j_data_channel_new_bytes (bytes);
  GError* tmperr = NULL;

  lexer = j_lexer_new_from_channel (channel, &tmperr);
          g_io_channel_shutdown (channel, 1, NULL);
          g_io_channel_unref (channel);
          g_bytes_unref (bytes);

  if (G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      _j_lexer_unref0 (lexer);
    }
return (lexer);
}

JLexer* j_lexer_new_from_file (const gchar* filename, GError** error)
{
  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);
  gint fd, e;

  if ((fd = open (filename, O_RDONLY)) < 0)
    {
      g_set_error
      (error,
       G_IO_CHANNEL_ERROR,
       G_IO_CHANNEL_ERROR_FAILED,
       "Can not open file %s",
        filename);
      return NULL;
    }

  JLexer* lexer = NULL;
  GIOChannel* channel = j_fd_channel_new (fd);
  //GIOChannel* channel = g_io_channel_unix_new (fd);
  GError* tmperr = NULL;

  lexer = j_lexer_new_from_channel (channel, &tmperr);
          g_io_channel_shutdown (channel, 1, NULL);
          g_io_channel_unref (channel);

  if (G_UNLIKELY (tmperr != NULL))
    {
      g_propagate_error (error, tmperr);
      _j_lexer_unref0 (lexer);
    }
return (lexer);
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

  if (g_atomic_int_dec_and_test (&self->ref_count))
    {
      g_array_unref (self->tokens);
      g_string_chunk_free (self->chunk);
      g_slice_free (JLexer, self);
    }
}

JToken* j_lexer_get_tokens (JLexer* lexer, guint* n_tokens)
{
  g_return_val_if_fail (lexer != NULL, NULL);
  JLexer* self = (lexer);
  guint nothing;

  *((n_tokens) ? n_tokens : &nothing) = self->tokens->len;
return (JToken*) self->tokens->data;
}

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

static TokenClass* token_classes_peek (void)
{
  static TokenClass* __classes__ = NULL;

  if (g_once_init_enter (&__classes__))
    {
      const gint n_classes = 23;
      TokenClass* classes = g_new0 (TokenClass, n_classes + 1);

      typedef gchar linecount [__LINE__ + 1];
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_COMMENT, "#(.*)");
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_QUOTED, "\"(.*?)\"");
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_QUOTED, "\'(.*?)\'");
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_AGAIN);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_CD);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_ELSE);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_END);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_EXIT);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_FALSE);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_FG);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_GET);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_HELP);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_HISTORY);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_IF);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_JOBS);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_SET);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_KEYWORD, J_TOKEN_KEYWORD_THEN);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_TRUE);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_BUILTIN, J_TOKEN_BUILTIN_UNSET);
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_OPERATOR, "[<>|`&]");
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_OPERATOR, "(>>|&&|\\|\\|)");
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_SEPARATOR, ",;:");
      classes [__LINE__ - sizeof (linecount)] = token_klass (J_TOKEN_TYPE_LITERAL, "[^\\s]+");
      G_STATIC_ASSERT (n_classes == __LINE__ - sizeof (linecount));
      g_once_init_leave (&__classes__, classes);
    }
return (TokenClass*) __classes__;
}

static gint is_empty (JLexer* lexer, const gchar* input, gsize length)
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

static const gchar* prepare (JLexer* lexer, const gchar* input, gsize length)
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
return g_string_chunk_insert_len (lexer->chunk, input, length - tailing);
}

static gint breakdown (JLexer* lexer, const gchar* input, gsize length, gsize line, gsize column, TokenClass* klass, GMatchInfo* info, GError** error)
{
  GError* tmperr = NULL;
  gint start, stop, last = 0;
  gint added = 0;

  while (g_match_info_matches (info))
    {
      g_match_info_fetch_pos (info, 0, &start, &stop);

      if (start > last)
      {
        added += search (lexer, input + last, start - last, line, column + last, &tmperr);

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
        const gchar* value = prepare (lexer, begin, stop - start);

        switch ((JTokenType) type)
          {
            case J_TOKEN_TYPE_BUILTIN:
            case J_TOKEN_TYPE_KEYWORD:
              value = g_intern_string (value);
            default: break;
          }

        const gint coloff = column + start;
        const JToken token = { type, line, coloff, value, };

        g_array_append_val (lexer->tokens, token);
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
      added += search (lexer, input + last, length - last, line, column + last, &tmperr);

      if (G_UNLIKELY (tmperr != NULL))
        {
          g_propagate_error (error, tmperr);
          return -1;
        }
    }
return (added);
}

static gint search (JLexer* lexer, const gchar* input, gsize length, gsize line, gsize column, GError** error)
{
  TokenClass* classes = NULL;
  TokenClass* klass = NULL;
  GMatchInfo* info = NULL;
  GError* tmperr = NULL;
  gint added = 0;
  gint got, i;

  classes = token_classes_peek ();

  for (i = 0; classes [i].pattern; ++i)
  {
    klass = & classes [i];
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
        added += breakdown (lexer, input, length, line, column, klass, info, &tmperr);
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

  if (added == 0 && !is_empty (lexer, input, length))
  {
    g_set_error
    (error, J_TOKEN_ERROR, J_LEXER_ERROR_UNKNOWN_TOKEN,
     "%d: %d: unknown token %s", line, column, prepare (lexer, input, length));
    return -1;
  }
return (added);
}
