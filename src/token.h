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
#ifndef __JASH_TOKEN__
#define __JASH_TOKEN__ 1
#include <glib.h>

typedef struct _JToken JToken;
typedef enum _JTokenType JTokenType;

#define J_TOKEN_BUILTIN_AGAIN (j_builtin_intern_string_again ())
#define J_TOKEN_BUILTIN_CD (j_builtin_intern_string_cd ())
#define J_TOKEN_KEYWORD_ELSE (j_keyword_intern_string_else ())
#define J_TOKEN_KEYWORD_END (j_keyword_intern_string_end ())
#define J_TOKEN_BUILTIN_EXIT (j_builtin_intern_string_exit ())
#define J_TOKEN_BUILTIN_FALSE (j_builtin_intern_string_false ())
#define J_TOKEN_BUILTIN_FG (j_builtin_intern_string_fg ())
#define J_TOKEN_BUILTIN_GET (j_builtin_intern_string_get ())
#define J_TOKEN_BUILTIN_HELP (j_builtin_intern_string_help ())
#define J_TOKEN_BUILTIN_HISTORY (j_builtin_intern_string_history ())
#define J_TOKEN_KEYWORD_IF (j_keyword_intern_string_if ())
#define J_TOKEN_BUILTIN_JOBS (j_builtin_intern_string_jobs ())
#define J_TOKEN_BUILTIN_SET (j_builtin_intern_string_set ())
#define J_TOKEN_KEYWORD_THEN (j_keyword_intern_string_then ())
#define J_TOKEN_BUILTIN_TRUE (j_builtin_intern_string_true ())
#define J_TOKEN_BUILTIN_UNSET (j_builtin_intern_string_unset ())

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

  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_again (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_cd (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_keyword_intern_string_else (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_keyword_intern_string_end (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_exit (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_false (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_fg (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_get (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_help (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_history (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_keyword_intern_string_if (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_jobs (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_set (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_keyword_intern_string_then (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_true (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_builtin_intern_string_unset (void) G_GNUC_CONST;

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_TOKEN__
