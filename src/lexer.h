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
#include <token.h>

typedef struct _JLexer JLexer;
typedef enum _JLexerError JLexerError;

#define J_LEXER_ERROR (j_lexer_error_quark ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  enum _JLexerError
  {
    J_LEXER_ERROR_FAILED,
    J_LEXER_ERROR_UNKNOWN_TOKEN,
  };

  G_GNUC_INTERNAL GQuark j_lexer_error_quark (void) G_GNUC_CONST;
  G_GNUC_INTERNAL JLexer* j_lexer_new ();
  G_GNUC_INTERNAL JLexer* j_lexer_ref (JLexer* lexer);
  G_GNUC_INTERNAL void j_lexer_unref (JLexer* lexer);
  G_GNUC_INTERNAL void j_lexer_scan_from_channel (JLexer* lexer, JTokens* tokens, GIOChannel* channel, GError** error);
  G_GNUC_INTERNAL void j_lexer_scan_from_data (JLexer* lexer, JTokens* tokens, const gchar* data, gssize length, GError** error);
  G_GNUC_INTERNAL void j_lexer_scan_from_file (JLexer* lexer, JTokens* tokens, const gchar* filename, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_LEXER__
