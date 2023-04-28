/* Copyright 2023 MarcosHCK
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
#ifndef __JASH_LEXER_TOKEN__
#define __JASH_LEXER_TOKEN__ 1
#include <glib.h>

typedef struct _JToken JToken;
typedef struct _JTokens JTokens;
typedef enum _JTokenType JTokenType;

#define J_TOKEN_BUILTIN_AGAIN (j_token_builtin_again_intern_string ())
#define J_TOKEN_BUILTIN_CD (j_token_builtin_cd_intern_string ())
#define J_TOKEN_BUILTIN_EXIT (j_token_builtin_exit_intern_string ())
#define J_TOKEN_BUILTIN_FALSE (j_token_builtin_false_intern_string ())
#define J_TOKEN_BUILTIN_FG (j_token_builtin_fg_intern_string ())
#define J_TOKEN_BUILTIN_GET (j_token_builtin_get_intern_string ())
#define J_TOKEN_BUILTIN_HELP (j_token_builtin_help_intern_string ())
#define J_TOKEN_BUILTIN_HISTORY (j_token_builtin_history_intern_string ())
#define J_TOKEN_BUILTIN_JOBS (j_token_builtin_jobs_intern_string ())
#define J_TOKEN_BUILTIN_SET (j_token_builtin_set_intern_string ())
#define J_TOKEN_BUILTIN_TRUE (j_token_builtin_true_intern_string ())
#define J_TOKEN_BUILTIN_UNSET (j_token_builtin_unset_intern_string ())
#define J_TOKEN_KEYWORD_ELSE (j_token_keyword_else_intern_string ())
#define J_TOKEN_KEYWORD_END (j_token_keyword_end_intern_string ())
#define J_TOKEN_KEYWORD_IF (j_token_keyword_if_intern_string ())
#define J_TOKEN_KEYWORD_THEN (j_token_keyword_then_intern_string ())
#define J_TOKEN_OPERATOR_DETACH (j_token_operator_detach_intern_string ())
#define J_TOKEN_OPERATOR_EXPANSION (j_token_operator_expansion_intern_string ())
#define J_TOKEN_OPERATOR_LOGICAL_AND (j_token_operator_logical_and_intern_string ())
#define J_TOKEN_OPERATOR_LOGICAL_OR (j_token_operator_logical_or_intern_string ())
#define J_TOKEN_OPERATOR_PIPE (j_token_operator_pipe_intern_string ())
#define J_TOKEN_OPERATOR_REDIRECTION_APPEND (j_token_operator_redirection_append_intern_string ())
#define J_TOKEN_OPERATOR_REDIRECTION_READ (j_token_operator_redirection_read_intern_string ())
#define J_TOKEN_OPERATOR_REDIRECTION_WRITE (j_token_operator_redirection_write_intern_string ())
#define J_TOKEN_SEPARATOR_CHAIN (j_token_separator_chain_intern_string ())
#define J_TOKEN_SEPARATOR_NEWLINE (j_token_separator_newline_intern_string ())

#define J_TOKEN_INIT { 0, 0, 0, NULL, }
#define J_TOKEN_TYPE_BITS (4)
#define J_TOKEN_LOCATION_BITS (((sizeof (guint64) * 8) - J_TOKEN_TYPE_BITS) / 2)

#if __cplusplus
extern "C" {
#endif // __cplusplus

  struct _JToken
  {
    guint64 type : J_TOKEN_TYPE_BITS;
    guint64 line : J_TOKEN_LOCATION_BITS;
    guint64 column : J_TOKEN_LOCATION_BITS;
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

  G_GNUC_INTERNAL const gchar* j_token_builtin_again_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_builtin_cd_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_builtin_exit_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_builtin_false_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_builtin_fg_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_builtin_get_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_builtin_help_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_builtin_history_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_builtin_jobs_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_builtin_set_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_builtin_true_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_builtin_unset_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_keyword_else_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_keyword_end_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_keyword_if_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_keyword_then_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_operator_detach_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_operator_expansion_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_operator_logical_and_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_operator_logical_or_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_operator_pipe_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_operator_redirection_append_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_operator_redirection_read_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_operator_redirection_write_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_separator_chain_intern_string (void) G_GNUC_CONST;
  G_GNUC_INTERNAL const gchar* j_token_separator_newline_intern_string (void) G_GNUC_CONST;

  G_GNUC_INTERNAL JTokens* j_tokens_ref (JTokens* tokens);
  G_GNUC_INTERNAL void j_tokens_unref (JTokens* tokens);
  G_GNUC_INTERNAL guint j_tokens_get_count (JTokens* tokens);
  G_GNUC_INTERNAL JToken* j_tokens_index (JTokens* tokens, guint index);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_LEXER_TOKEN__
