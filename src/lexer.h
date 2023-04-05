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
#ifndef __JASH_LEXER__
#define __JASH_LEXER__ 1
#include <glib.h>

typedef struct _JLexer JLexer;
typedef struct _JToken JToken;
typedef enum _JTokenType JTokenType;
typedef enum _JTokenError JTokenError;

#define J_TOKEN_ERROR (j_lexer_error_quark ())
#define J_TOKEN_BUILTIN_AGAIN (g_intern_static_string ("again"))
#define J_TOKEN_BUILTIN_CD (g_intern_static_string ("cd"))
#define J_TOKEN_KEYWORD_ELSE (g_intern_static_string ("else"))
#define J_TOKEN_KEYWORD_END (g_intern_static_string ("end"))
#define J_TOKEN_BUILTIN_EXIT (g_intern_static_string ("exit"))
#define J_TOKEN_BUILTIN_FALSE (g_intern_static_string ("false"))
#define J_TOKEN_BUILTIN_FG (g_intern_static_string ("fg"))
#define J_TOKEN_BUILTIN_GET (g_intern_static_string ("get"))
#define J_TOKEN_BUILTIN_HELP (g_intern_static_string ("help"))
#define J_TOKEN_BUILTIN_HISTORY (g_intern_static_string ("history"))
#define J_TOKEN_KEYWORD_IF (g_intern_static_string ("if"))
#define J_TOKEN_BUILTIN_JOBS (g_intern_static_string ("jobs"))
#define J_TOKEN_BUILTIN_SET (g_intern_static_string ("set"))
#define J_TOKEN_KEYWORD_THEN (g_intern_static_string ("then"))
#define J_TOKEN_BUILTIN_TRUE (g_intern_static_string ("true"))
#define J_TOKEN_BUILTIN_UNSET (g_intern_static_string ("unset"))

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JToken
  {
    guint64 type : 4;
    guint64 line : (sizeof (guint64) * 8 - 4) / 2;
    guint64 column : (sizeof (guint64) * 8 - 4) / 2;
    const gchar* value;
  };

  enum _JTokenType
  {
    J_TOKEN_TYPE_BUILTIN,
    J_TOKEN_TYPE_COMMENT,
    J_TOKEN_TYPE_KEYWORD,
    J_TOKEN_TYPE_LITERAL,
    J_TOKEN_TYPE_OPERATOR,
    J_TOKEN_TYPE_SEPARATOR,
    J_TOKEN_TYPE_QUOTED,
  };

  enum _JTokenError
  {
    J_TOKEN_ERROR_FAILED,
    J_TOKEN_ERROR_UNKNOWN_TOKEN,
  };

  G_GNUC_INTERNAL GQuark j_lexer_error_quark (void) G_GNUC_CONST;
  G_GNUC_INTERNAL JLexer* j_lexer_new ();
  G_GNUC_INTERNAL JLexer* j_lexer_new_from_channel (GIOChannel* channel, GError** error);
  G_GNUC_INTERNAL JLexer* j_lexer_new_from_data (const gchar* data, gssize length, GError** error);
  G_GNUC_INTERNAL JLexer* j_lexer_new_from_file (const gchar* filename, GError** error);
  G_GNUC_INTERNAL JLexer* j_lexer_ref (JLexer* lexer);
  G_GNUC_INTERNAL void j_lexer_unref (JLexer* lexer);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_LEXER__
