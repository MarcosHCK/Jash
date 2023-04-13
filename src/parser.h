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
#ifndef __JASH_PARSER__
#define __JASH_PARSER__ 1
#include <token.h>

typedef struct _JParser JParser;
typedef enum _JParserError JParserError;

#define J_PARSER_ERROR (j_parser_error_quark ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  enum _JParserError
  {
    J_PARSER_ERROR_FAILED,
    J_PARSER_ERROR_UNEXPECTED_EOF,
    J_PARSER_ERROR_EXPECTED_TOKEN,
    J_PARSER_ERROR_UNEXPECTED_TOKEN,
  };

  G_GNUC_INTERNAL GQuark j_parser_error_quark (void) G_GNUC_CONST;
  G_GNUC_INTERNAL JParser* j_parser_new_from_tokens (JToken* tokens, guint n_tokens, GError** error);
  G_GNUC_INTERNAL JParser* j_parser_ref (JParser* parser);
  G_GNUC_INTERNAL void j_parser_unref (JParser* parser);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_PARSER__
