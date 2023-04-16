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
#include <glib-object.h>
#include <token.h>

#define J_TYPE_PARSER (j_parser_get_type ())
#define J_PARSER(obj) (G_TYPE_CHECK_INSTANCE_CAST ((obj), J_TYPE_PARSER, JParser))
#define J_IS_PARSER(obj) (G_TYPE_CHECK_INSTANCE_TYPE ((obj), J_TYPE_PARSER))
typedef struct _JParser JParser;

#define J_PARSER_ERROR (j_parser_error_quark ())

#if __cplusplus
extern "C" {
#endif // __cplusplus

  typedef enum
  {
    J_PARSER_ERROR_FAILED,
    J_PARSER_ERROR_UNEXPECTED_EOF,
    J_PARSER_ERROR_EXPECTED_TOKEN,
    J_PARSER_ERROR_UNEXPECTED_TOKEN,
    J_PARSER_ERROR_TOO_FEW_ARGUMENTS,
    J_PARSER_ERROR_TOO_MANY_ARGUMENTS,
  } JParserError;

  G_GNUC_INTERNAL GQuark j_parser_error_quark (void) G_GNUC_CONST;
  G_GNUC_INTERNAL GType j_parser_get_type (void) G_GNUC_CONST;
  G_GNUC_INTERNAL JParser* j_parser_new ();
  G_GNUC_INTERNAL GClosure* j_parser_parse (JParser* parser, JTokens* tokens, GError** error);

#if __cplusplus
}
#endif // __cplusplus

#endif // __JASH_PARSER__
